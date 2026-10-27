# Optional active error reporting

Include `packages/active_errors.yml` alongside the normal base/dashboard package.
The unit's active errors are read with RMI `GETACTIVEERRORS` (`80 03 01`) and exposed as:

- **Active Errors** (text sensor): `No active errors`, or each active error as
  `<id>: <description>`, separated by `; `.
- **Has Active Errors** (binary sensor, `problem` device class): on while any error is active.
- **Reset All Errors** (button, diagnostic): sends RMI `RESETALLERRORS` (`82 03 01`) and, if
  the unit answers, reads the active errors straight back so the two entities above update
  at once. It does nothing on boot and only acts when pressed.

## Including the package

ESPHome Device Builder (remote package): list it next to your board's dashboard file.

```yaml
packages:
  yoziru.esphome-zehnder-comfoair:
    url: https://github.com/yoziru/esphome-zehnder-comfoair.git
    ref: ${zehnder_comfoair_ref}
    files:
      - zehnder-comfoair-q-m5stack-atoms3.dashboard.yml
      - packages/active_errors.yml
```

Local checkout (CLI): add it next to the base and board packages in your device file.

```yaml
packages:
  base: !include packages/base.yml
  device_base: !include boards/m5stack-atoms3.yml
  active_errors: !include packages/active_errors.yml
```

**Warnings count.** Has Active Errors is on for every active entry, including the status and
warning flags the unit raises as well as faults, for example the filter warnings (77–79), service
mode (80), bypass in manual mode (89), hood active (96) and the status flags 97 and 100. This is
deliberate: a filter change that is due keeps the sensor on as a reminder. To react to only some of
them, match the ID at the start of an entry (for example `79:`) in an automation or template sensor.

ID 100 ("Error analysis is in progress…") is transient: the unit raises it
while it checks its errors, after **Reset All Errors** and at power-up until its initialisation is
complete, so Has Active Errors can be on briefly then. A reset does not clear warnings that describe
a condition, such as the filter flag; those stay on until the condition is gone, for example after the
filters are changed.

## Using the list in Home Assistant

Several errors can be active at once, so Active Errors is one string with the entries in ID order,
separated by `; ` (no description contains that separator), each starting with its ID and a colon. For
anything more specific, build template sensors in Home Assistant from it. Replace the entity ID with
yours, and add them to your Home Assistant configuration (not to the ESPHome device):

```yaml
template:
  - binary_sensor:
      # On while a filter change is due or about to be (IDs 79 and 77)
      - name: "MVHR filter change due"
        unique_id: mvhr_filter_change_due
        device_class: problem
        availability: "{{ states('sensor.zehnder_comfoair_q_active_errors') not in ['unknown', 'unavailable'] }}"
        state: "{{ states('sensor.zehnder_comfoair_q_active_errors') is search('(^|; )(77|79):') }}"

      # On only for real faults, ignoring the status and warning flags
      - name: "MVHR fault"
        unique_id: mvhr_fault
        device_class: problem
        availability: "{{ states('sensor.zehnder_comfoair_q_active_errors') not in ['unknown', 'unavailable'] }}"
        state: >
          {% set ignore = [77, 78, 79, 80, 89, 96, 97, 100] %}
          {% set ns = namespace(faults=0) %}
          {% for entry in states('sensor.zehnder_comfoair_q_active_errors').split('; ') if entry != 'No active errors' %}
            {% if (entry.split(':')[0] | int(0)) not in ignore %}
              {% set ns.faults = ns.faults + 1 %}
            {% endif %}
          {% endfor %}
          {{ ns.faults > 0 }}

  - sensor:
      # How many entries are active
      - name: "MVHR active error count"
        unique_id: mvhr_active_error_count
        availability: "{{ states('sensor.zehnder_comfoair_q_active_errors') not in ['unknown', 'unavailable'] }}"
        state: >
          {{ states('sensor.zehnder_comfoair_q_active_errors').split('; ') | reject('eq', 'No active errors') | list | count }}
```

The `availability` lines keep these sensors unavailable, and not wrongly off, while the device is
offline. The IDs are those at the start of each entry. The "fault" example treats an ID with no
description (`Unknown error`) as a fault, and the IDs it ignores are the status and warning flags listed
above (97 is a status flag according to Zehnder's manual). If a very long list is shortened with `...`
(see Wording), an entry at the end can be cut off and missed.

## Polling

`error_poll_interval` sets how often the errors are read. The default is `5min`, with the first
read 15 s after boot. Override it in your device YAML, on the same component the package extends:

```yaml
zehnder_comfoair_q:
  error_poll_interval: 15min
```

**`error_poll_interval: never` disables polling.** The entities are still created, but the errors
are never read periodically, so **Active Errors** and **Has Active Errors** only update when a
read is triggered, which happens after pressing **Reset All Errors**. To remove the feature
completely (no entities, no polling), leave `packages/active_errors.yml` out.

The read is a single GET command and changes no setting.

## Wording

Descriptions are the ERRORS table (firmware 1.4.0+ numbering) of
[aiocomfoconnect](https://github.com/michaelarnauts/aiocomfoconnect) (`aiocomfoconnect/const.py`), used
as written. They say which part failed, for example `The extract air fan has a malfunction (FAN_EHA ERROR)`,
and some end with the code aiocomfoconnect uses for it. An ID with no description shows as
`Unknown error`.

Zehnder's ComfoConnect PRO Modbus installation manual (revision 137), section 9.5 "ComfoAirQ alarm
codes" (Table 2), lists the same IDs and was used to cross-check them. It words many of them
differently: it names sensors by designation (T11, P12, F12, ...) and in places gives a different
meaning, for example ID 80 is "Service mode is active" here and "Standby is active" in the manual. The
manual has no entry for IDs 63–65, 70–73 and 82–88, which rely on aiocomfoconnect alone.

Home Assistant rejects entity states longer than 255 characters, so a very long list is
shortened with `...`. The full text is logged at debug level.

## Language

English is the default. The language is a **compile-time** choice for devices with little
memory: `error_language: <code>` on the `zehnder_comfoair_q:` block selects one
`error_descriptions_<code>.h` and only that file is compiled into the firmware. Other language
files in the component folder are ignored, so they cost no flash or RAM, and the text is
constant data held in flash. An unknown code is rejected when the config is validated.

To add a language, copy `components/zehnder_comfoair_q/error_descriptions_en.h` to
`error_descriptions_<code>.h` in the same folder (it must stay flat beside the other component
files, as ESPHome's local component copy does not include subfolders), translate the strings, keep
the `#ifdef` wrapper, and set `error_language: <code>`. No C++ changes are needed.

## Protocol

Requests go through a small queued request/response layer (`send_rmi_request`): requests
are sent one at a time, multi-frame responses (up to 7 data bytes per frame, last frame
flagged `0x80`) are reassembled in order, and a missing or out-of-order response, a
protocol error response, or a 2 s timeout ends the request without publishing. Responses
from the unit (`0x1F` prefix, request bit clear) are routed in by the CAN frame trigger in
the package; responses are not correlated by RMI sequence number. Bit *n* of the response
(least significant bit first) is error ID *n*.
