"""Compile and run actual ESPHome C++/YAML on the host, without an ESP32."""

import os
from pathlib import Path
import subprocess
import sys
import tempfile

import yaml

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "tests/.esphome/startup"


def run(climate):
    name = "startup-climate" if climate else "startup-no-climate"
    packages = ["binary_sensors", "sensors", "text_sensors", "select", "switch", "fan"]
    if climate:
        packages.append("climate")
    packages.append("modulated_fan_speed")
    packages.append("airflow_constraints")
    packages.append("filter_lifetime")
    packages.append("temperature_profile_mode")
    packages.append("active_errors")
    bus =yaml.safe_load((ROOT / "packages/canbus.yml").read_text())["canbus"][0]
    bus.update(platform="test_canbus", id="test_bus")
    del bus["tx_pin"], bus["rx_pin"]
    actions = [
        {"lambda": (ROOT / "tests/startup_checks.cpp").read_text()},
        {"delay": "1500ms"},
        {"lambda": '''
            esphome::test_canbus::check(id(test_bus).sent.empty(),
                "feedback must not leave delayed outbound commands");
        '''},
    ]
    for mode, offset in [(1, 2), (6, 1)]:
        actions.extend([
            {"lambda": f'''
                id(test_bus).receive(49, {mode});
                id(test_bus).sent.clear();
                id(auto_ventilation).turn_on();
            '''},
            {"delay": "2500ms"},
            {"lambda": f'''
                const auto &sent = id(test_bus).sent;
                esphome::test_canbus::check(sent.size() >= {offset + 2} &&
                    sent[{offset}].can_data_length_code == 4 && sent[{offset}].data[0] == 0x85 &&
                    sent[{offset}].data[2] == 0x08 && sent[{offset}].data[3] == 0x01 &&
                    sent[{offset + 1}].can_data_length_code == 4 && sent[{offset + 1}].data[0] == 0x85 &&
                    sent[{offset + 1}].data[2] == 0x01 && sent[{offset + 1}].data[3] == 0x01,
                    "Auto after delayed mode {mode} cancels both timers in order");
                id(test_bus).receive(49, 255);
                id(test_bus).sent.clear();
            '''},
        ])
    if climate:
        # Let the real thermostat's minimum run/off timers expire between actions.
        cases = [
            (0, "0xc8, 0x00", "HEAT", 2, "heating selects Warm"),  # 20.0 C
            (2, "0xe1, 0x00", None, 0, "idle selects Normal"),  # 22.5 C
            (0, "0x04, 0x01", "COOL", 1, "cooling selects Cool"),  # 26.0 C
        ]
        for profile, temperature, mode, expected, contract in cases:
            mode_call = "" if mode is None else f'''
                auto call = id(comfoair_climate).make_call();
                call.set_mode(climate::CLIMATE_MODE_{mode});
                call.perform();
            '''
            actions.extend([
                {"lambda": f'''
                    id(test_bus).receive(67, {profile});
                    id(test_bus).sent.clear();
                    id(test_bus).receive(274, std::vector<uint8_t>{{{temperature}}});
                    {mode_call}
                '''},
                {"delay": "1500ms"},
                {"lambda": f'''
                    const auto &sent = id(test_bus).sent;
                    esphome::test_canbus::check(sent.size() == 2 &&
                        sent[0].data[1] == 0x84 && sent[0].data[2] == 0x15 &&
                        sent[0].data[3] == 0x03 && sent[0].data[4] == 0x01 && sent[1].data[6] == {expected},
                        "thermostat after synchronization: {contract}");
                '''},
            ])
    actions.extend([
        {"lambda": '''
            auto &bus = *id(test_bus);
            bus.requests.clear();
            id(comfoair).set_request_ids({65, 67});
            id(comfoair).update();
            id(comfoair).update();
            esphome::test_canbus::check(bus.requests.size() == 1,
                "an update must not interleave a second request cycle");
        '''},
        {"delay": "300ms"},
        {"lambda": '''
            auto &requests = id(test_bus).requests;
            esphome::test_canbus::check(requests.size() == 2 &&
                requests[0].can_id == ((65u << 14) | 0x6a) &&
                requests[1].can_id == ((67u << 14) | 0x6a) &&
                requests[0].use_extended_id && requests[1].use_extended_id &&
                requests[0].remote_transmission_request && requests[1].remote_transmission_request,
                "delayed PDO cycle preserves order and sends extended RTR frames");
            id(comfoair).set_request_ids({});
            id(comfoair).update();
            esphome::test_canbus::check(requests.size() == 2, "empty request list sends nothing");
            id(comfoair).set_request_ids({65, 67});
            id(comfoair).update();
        '''},
        {"delay": "300ms"},
        {"lambda": '''
            const auto &requests = id(test_bus).requests;
            esphome::test_canbus::check(requests.size() == 4 &&
                requests[2].can_id == requests[0].can_id && requests[3].can_id == requests[1].can_id,
                "completed request cycle can start again");
        '''},
    ])
    actions.extend([
        {"lambda": '''
            using esphome::test_canbus::check;
            auto &bus = *id(test_bus);
            // RMI responses from the unit (node 1) to us (node 0x2a): 0x1f prefix, request bit clear.
            const uint32_t unit_to_us = 0x1f000000u | (0x2au << 6) | 0x01;
            const uint32_t unit_to_us_multi = unit_to_us | (1u << 14);
            const uint32_t unit_to_us_error = unit_to_us | (1u << 15);
            auto fragments = [](const std::vector<uint8_t> &payload) {
                std::vector<std::vector<uint8_t>> frames;
                for (size_t offset = 0, block = 0; offset < payload.size(); offset += 7, block++) {
                    size_t length = std::min(size_t(7), payload.size() - offset);
                    std::vector<uint8_t> frame{uint8_t(block | (offset + length == payload.size() ? 0x80 : 0))};
                    frame.insert(frame.end(), payload.begin() + offset, payload.begin() + offset + length);
                    frames.push_back(frame);
                }
                return frames;
            };
            auto errors = [](std::initializer_list<int> error_ids) {
                std::vector<uint8_t> bits(32, 0);
                for (int error_id : error_ids)
                    bits[error_id / 8] |= 1 << (error_id % 8);
                return bits;
            };

            bus.sent.clear();
            id(comfoair).request_active_errors();
            check(bus.sent.size() == 1 && bus.sent[0].can_data_length_code == 3 &&
                  bus.sent[0].data[0] == 0x80 && bus.sent[0].data[1] == 0x03 && bus.sent[0].data[2] == 0x01 &&
                  (bus.sent[0].can_id & 0x3f) == 0x2a && ((bus.sent[0].can_id >> 6) & 0x3f) == 1 &&
                  ((bus.sent[0].can_id >> 16) & 1) == 1 && !bus.sent[0].remote_transmission_request,
                  "GETACTIVEERRORS is a single 80 03 01 request frame to the unit");

            auto frames = fragments(errors({21, 77}));
            check(frames.size() == 5, "a 32 byte response is five fragments");
            for (size_t i = 0; i < frames.size(); i++) {
                bus.receive_frame(unit_to_us_multi, frames[i]);
                if (i + 1 < frames.size())
                    check(!id(active_errors).has_state(), "nothing is published before the last fragment");
            }
            check(id(active_errors).state ==
                      "21: DANGER! OVERHEATING! Two or more sensors are detecting an incorrect temperature. "
                      "Ventilation has stopped.; "
                      "77: The filters of the Ventilation Unit must be replaced now" &&
                  id(has_active_errors).state,
                  "a multi-frame response lists each active error and sets the problem sensor");

            id(comfoair).request_active_errors();
            bus.receive_frame(unit_to_us, std::vector<uint8_t>(8, 0));
            check(id(active_errors).state == "No active errors" && !id(has_active_errors).state,
                  "a single-frame response with no bits set means no active errors");

            id(comfoair).request_active_errors();
            bus.receive_frame(unit_to_us, std::vector<uint8_t>{0, 0, 0, 0, 0x10, 0, 0, 0});
            check(id(active_errors).state == "36: Unknown error" && id(has_active_errors).state,
                  "an ID without a description is reported as unknown");

            id(comfoair).request_active_errors();
            bus.receive_frame(unit_to_us_error, std::vector<uint8_t>{0x0b});
            check(id(active_errors).state == "36: Unknown error",
                  "a RMI error response publishes nothing");

            // Requests are queued; responses for other nodes are ignored.
            int done = 0;
            bool ok1 = false, ok2 = false;
            std::vector<uint8_t> first, second;
            bus.sent.clear();
            id(comfoair).send_rmi_request({0x01, 0x20, 0x01, 0x10, 0x03},
                [&](bool ok, const std::vector<uint8_t> &r) { ok1 = ok; first = r; done++; });
            id(comfoair).send_rmi_request({0x01, 0x20, 0x01, 0x10, 0x04},
                [&](bool ok, const std::vector<uint8_t> &r) { ok2 = ok; second = r; done++; });
            check(bus.sent.size() == 1, "a second request waits for the first to complete");
            bus.receive_frame(0x1f000000u | (0x2bu << 6) | 0x01, std::vector<uint8_t>{0xee});
            bus.receive_frame(0x1f000000u | (0x2au << 6) | 0x30, std::vector<uint8_t>{0xee});
            check(done == 0 && bus.sent.size() == 1, "responses for another node or from another source are ignored");
            bus.receive_frame(unit_to_us, std::vector<uint8_t>{0x11, 0x22});
            check(done == 1 && ok1 && first == std::vector<uint8_t>({0x11, 0x22}) &&
                  bus.sent.size() == 2 && bus.sent[1].data[4] == 0x04,
                  "the queued request is sent once the first completes");
            bus.receive_frame(unit_to_us, std::vector<uint8_t>{0x33});
            check(done == 2 && ok2 && second == std::vector<uint8_t>({0x33}),
                  "the queued request gets its own response");

            bool called = false, success = true;
            id(comfoair).send_rmi_request({0x80, 0x03, 0x01},
                [&](bool ok, const std::vector<uint8_t> &) { called = true; success = ok; });
            bus.receive_frame(unit_to_us_multi, std::vector<uint8_t>{0x01, 0xaa});
            check(called && !success, "a fragment out of order fails the request");
            bus.receive_frame(unit_to_us_multi, std::vector<uint8_t>{0x80, 0xaa});  // late fragment, nothing in flight

            // IDs 24, 35 and 102 make the text exceed 255 bytes right inside the three-byte en dash of ID 102.
            id(comfoair).request_active_errors();
            for (const auto &frame : fragments(errors({24, 35, 102})))
                bus.receive_frame(unit_to_us_multi, frame);
            const std::string text = id(active_errors).state;
            bool valid_utf8 = true;
            for (size_t i = 0; i < text.size();) {
                uint8_t c = text[i];
                size_t n = c < 0x80 ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xe ? 3 : (c >> 3) == 0x1e ? 4 : 0;
                if (n == 0 || i + n > text.size()) {
                    valid_utf8 = false;
                    break;
                }
                for (size_t k = 1; k < n; k++)
                    if ((uint8_t(text[i + k]) & 0xc0) != 0x80)
                        valid_utf8 = false;
                i += n;
            }
            check(text.size() <= 255 && text.size() > 3 && text.compare(text.size() - 3, 3, "...") == 0 && valid_utf8,
                  "a state over 255 characters is shortened at a character boundary");
        '''},
        {"lambda": '''
            using esphome::test_canbus::check;
            auto &bus = *id(test_bus);
            const uint32_t unit_to_us = 0x1f000000u | (0x2au << 6) | 0x01;
            const uint32_t unit_to_us_error = unit_to_us | (1u << 15);

            // The real Reset All Errors button: RESETALLERRORS, then an immediate read-back.
            bus.sent.clear();
            id(reset_all_errors_button).press();
            check(bus.sent.size() == 1 && bus.sent[0].can_data_length_code == 3 &&
                  bus.sent[0].data[0] == 0x82 && bus.sent[0].data[1] == 0x03 && bus.sent[0].data[2] == 0x01 &&
                  (bus.sent[0].can_id & 0x3f) == 0x2a && ((bus.sent[0].can_id >> 6) & 0x3f) == 1 &&
                  ((bus.sent[0].can_id >> 16) & 1) == 1,
                  "Reset All Errors sends RESETALLERRORS 82 03 01 to the unit");
            bus.receive_frame(unit_to_us, std::vector<uint8_t>{0x00});
            check(bus.sent.size() == 2 && bus.sent[1].can_data_length_code == 3 &&
                  bus.sent[1].data[0] == 0x80 && bus.sent[1].data[1] == 0x03 && bus.sent[1].data[2] == 0x01,
                  "a successful reset is followed by reading the active errors back");
            bus.receive_frame(unit_to_us, std::vector<uint8_t>(8, 0));
            check(id(active_errors).state == "No active errors" && !id(has_active_errors).state,
                  "the read-back after a reset updates the entities");
            bus.sent.clear();
            id(reset_all_errors_button).press();
            bus.receive_frame(unit_to_us_error, std::vector<uint8_t>{0x0b});
            check(bus.sent.size() == 1, "a failed reset is not followed by a read-back");
        '''},
        {"lambda": '''
            id(rmi_timeout_result) = 0;
            id(comfoair).send_rmi_request({0x80, 0x03, 0x01},
                [](bool success, const std::vector<uint8_t> &) { id(rmi_timeout_result) = success ? 1 : 2; }, 200);
        '''},
        {"delay": "300ms"},
        {"lambda": '''
            using esphome::test_canbus::check;
            auto &bus = *id(test_bus);
            check(id(rmi_timeout_result) == 2, "an unanswered request fails after its timeout");
            bus.sent.clear();
            id(comfoair).request_active_errors();
            check(bus.sent.size() == 1, "a timed out request does not block the next one");
            bus.receive_frame(0x1f000000u | (0x2au << 6) | 0x01, std::vector<uint8_t>(8, 0));
        '''},
    ])
    if not climate:
        # Polling is scheduled by the component, independent of climate; keep the climate runs short.
        # First read after the given delay, then once per interval, with no read in between.
        poll_response = "id(test_bus).receive_frame(0x1f000000u | (0x2au << 6) | 0x01, std::vector<uint8_t>(8, 0));"
        actions.extend([
            {"lambda": '''
                id(test_bus).sent.clear();
                id(comfoair).set_error_poll_interval(400);
                id(comfoair).start_active_errors_polling(150);
            '''},
            {"delay": "80ms"},
            {"lambda": '''
                esphome::test_canbus::check(id(test_bus).sent.empty(), "no active errors read before the first delay");
            '''},
            {"delay": "170ms"},
            {"lambda": f'''
                const auto &sent = id(test_bus).sent;
                esphome::test_canbus::check(sent.size() == 1 && sent[0].data[0] == 0x80 && sent[0].data[1] == 0x03,
                    "the first active errors read follows the first delay");
                {poll_response}
            '''},
            {"delay": "200ms"},
            {"lambda": '''
                esphome::test_canbus::check(id(test_bus).sent.size() == 1, "no extra read between polling intervals");
            '''},
            {"delay": "250ms"},
            {"lambda": f'''
                esphome::test_canbus::check(id(test_bus).sent.size() == 2, "active errors are read once per interval");
                {poll_response}
                id(comfoair).stop_active_errors_polling();
            '''},
            {"delay": "400ms"},
            {"lambda": '''
                esphome::test_canbus::check(id(test_bus).sent.size() == 2, "a stopped poll is not read again");
            '''},
        ])
    actions.append({
        "lambda": '''
            esphome::test_canbus::check(global_preferences->sync(),
                "host preferences can be saved for a restart");
            std::exit(esphome::test_canbus::failures ? EXIT_FAILURE : EXIT_SUCCESS);
        '''
    })
    config = {
        "substitutions": {"filter_time": "1s", "filter_send_every": "1", "filter_window_size": "1", "comfoair_canbus_id": "test_bus"},
        "esphome": {
            "name": name,
            "build_path": str(BUILD / name),
            "on_boot": [{"priority": -100, "then": actions}],
        },
        "host": {},
        "logger": {"level": "DEBUG"},
        "external_components": [
            {"source": {"type": "local", "path": str(ROOT / "components")}},
            {"source": {"type": "local", "path": str(ROOT / "tests/components")}},
        ],
        "canbus": [bus],
        # error_poll_interval: never keeps the active errors poll timers out of the explicit RMI cases.
        "zehnder_comfoair_q": {"id": "comfoair", "canbus_id": "test_bus", "request_ids": [65, 67], "request_interval": "never", "error_poll_interval": "never"},
        "globals": [{"id": "rmi_timeout_result", "type": "int", "initial_value": "0"}],
    }
    if os.environ.get("ESPHOME_TEST_SANITIZE") == "1":
        config["esphome"]["platformio_options"] = {
            "build_flags": ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        }
    if os.environ.get("ESPHOME_TEST_BOUNDS") == "1":
        flags = (["-D_LIBCPP_ABI_BOUNDED_ITERATORS_IN_VECTOR=1",
                  "-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG"]
                 if sys.platform == "darwin" else ["-D_GLIBCXX_DEBUG"])
        config["esphome"].setdefault("platformio_options", {}).setdefault("build_flags", []).extend(flags)
    BUILD.mkdir(parents=True, exist_ok=True)
    path = BUILD / f"{name}.yaml"
    includes = "packages:\n" + "".join(
        f"  {p}: !include {ROOT / 'packages' / (p + '.yml')}\n" for p in packages
    )
    # Verify actual ESPHome merge semantics, not a copied expected source list.
    from esphome.config import read_config
    from esphome.core import CORE
    CORE.reset()
    CORE.config_path = path
    path.write_text(yaml.safe_dump(config, sort_keys=False) + includes)
    merged = read_config({})
    assert merged is not None
    assert sorted(int(pdo) for pdo in merged["zehnder_comfoair_q"]["request_ids"]) == [65, 67, 226, 230]
    assert len(merged["canbus"]) == 1
    subprocess.run([sys.executable, "-m", "esphome", "compile", str(path)], check=True)
    # Fresh preferences per run; do not inherit the workstation's host state.
    with tempfile.TemporaryDirectory(dir=BUILD) as prefs:
        for boot in ("fresh boot", "restart with saved preferences"):
            print(f"\n{name}: {boot}", flush=True)
            subprocess.run(
                [str(BUILD / name / ".pioenvs" / name / "program")],
                env={**os.environ, "ESPHOME_PREFDIR": prefs},
                check=True,
                timeout=15,
            )


if __name__ == "__main__":
    run(False)
    if "--no-climate" not in sys.argv:
        run(True)
