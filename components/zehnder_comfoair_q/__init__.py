from pathlib import Path

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, text_sensor
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC
from esphome.components.canbus import CONF_CANBUS_ID, CanbusComponent, CanSpeed


DEPENDENCIES = ["canbus"]
AUTO_LOAD = ["binary_sensor", "text_sensor"]

zehnder_comfoair_q_ns = cg.esphome_ns.namespace("zehnder_comfoair_q")
ZehnderComfoAirQ = zehnder_comfoair_q_ns.class_("ZehnderComfoAirQ", cg.Component)

# we don't use UPDATE_INTERVAL as it's only the requests we send, but updates can come in any time (if the value changes)
CONF_REQUEST_INTERVAL = "request_interval"
CONF_REQUEST_IDS = "request_ids"
CONF_REQUEST_DELAY = "request_delay"
CONF_REQUEST_LOCAL_NODE_ID = "local_node_id"

# Active errors (GETACTIVEERRORS) are opt-in via packages/active_errors.yml: nothing is polled unless
# error_poll_interval is set, and the entities only exist if configured.
CONF_ERROR_POLL_INTERVAL = "error_poll_interval"
CONF_ACTIVE_ERRORS_TEXT_SENSOR = "active_errors_text_sensor"
CONF_HAS_ACTIVE_ERRORS_BINARY_SENSOR = "has_active_errors_binary_sensor"

# Selects which error_descriptions_<code>.h gets compiled in. The available codes are discovered from
# the files next to this one, so adding a language is just adding a file (see error_descriptions_en.h).
# Only the selected file is included in the firmware.
CONF_ERROR_LANGUAGE = "error_language"
_ERROR_DESCRIPTIONS_PREFIX = "error_descriptions_"
ERROR_LANGUAGES = sorted(
    p.stem[len(_ERROR_DESCRIPTIONS_PREFIX):]
    for p in Path(__file__).parent.glob(f"{_ERROR_DESCRIPTIONS_PREFIX}*.h")
)


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ZehnderComfoAirQ),
            cv.GenerateID(CONF_CANBUS_ID): cv.use_id(CanbusComponent),
            cv.Optional(CONF_REQUEST_INTERVAL, default="1h"): cv.update_interval,
            cv.Optional(CONF_REQUEST_IDS, default={}): cv.ensure_list(
                cv.int_range(min=0, max=0x3ff),
            ),
            cv.Optional(CONF_REQUEST_DELAY, default="100ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_REQUEST_LOCAL_NODE_ID, default=0x2a): cv.int_range(min=0x01, max=0x3f),
            cv.Optional(CONF_ERROR_POLL_INTERVAL): cv.update_interval,
            cv.Optional(CONF_ERROR_LANGUAGE, default="en"): cv.one_of(*ERROR_LANGUAGES, lower=True),
            cv.Optional(CONF_ACTIVE_ERRORS_TEXT_SENSOR): text_sensor.text_sensor_schema(
                icon="mdi:alert-circle",
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_HAS_ACTIVE_ERRORS_BINARY_SENSOR): binary_sensor.binary_sensor_schema(
                device_class="problem",
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
        }
    )
)


async def to_code(config):
    # also set CAN bus parameters here (since they're fixed anyway)
    parent = await cg.get_variable(config[CONF_CANBUS_ID])
    cg.add(parent.set_use_extended_id(True))
    cg.add(parent.set_bitrate(CanSpeed.CAN_50KBPS))

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_parent(parent))
    cg.add(var.set_update_interval(config[CONF_REQUEST_INTERVAL]))
    cg.add(var.set_request_ids(config[CONF_REQUEST_IDS]))
    cg.add(var.set_request_delay(config[CONF_REQUEST_DELAY]))
    cg.add(var.set_local_node_id(config[CONF_REQUEST_LOCAL_NODE_ID]))

    if CONF_ERROR_POLL_INTERVAL in config:
        cg.add(var.set_error_poll_interval(config[CONF_ERROR_POLL_INTERVAL]))

    cg.add_define(
        "ZEHNDER_ERROR_DESCRIPTIONS_HEADER",
        f"{_ERROR_DESCRIPTIONS_PREFIX}{config[CONF_ERROR_LANGUAGE]}.h",
    )

    if CONF_ACTIVE_ERRORS_TEXT_SENSOR in config:
        sens = await text_sensor.new_text_sensor(config[CONF_ACTIVE_ERRORS_TEXT_SENSOR])
        cg.add(var.set_active_errors_text_sensor(sens))

    if CONF_HAS_ACTIVE_ERRORS_BINARY_SENSOR in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_HAS_ACTIVE_ERRORS_BINARY_SENSOR])
        cg.add(var.set_has_active_errors_binary_sensor(sens))
