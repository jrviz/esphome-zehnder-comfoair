# Optional active error reporting

Include `packages/active_errors.yml` alongside the normal base/dashboard package.
The unit's active errors are read with RMI `GETACTIVEERRORS` (`80 03 01`) and exposed as:

- **Active Errors** (text sensor): `No active errors`, or each active error as
  `<id>: <description> (<code>)`, separated by `; `.
- **Has Active Errors** (binary sensor, `problem` device class): on while any error is active.
- **Reset All Errors** (button, diagnostic): sends RMI `RESETALLERRORS` (`82 03 01`) and, if
  the unit answers, reads the active errors straight back so the two entities above update
  at once. It does nothing on boot and only acts when pressed.

**Warnings count.** Has Active Errors is on for every active entry, including the status and
warning flags the unit raises as well as faults, for example the filter warnings (77–79), standby
(80), bypass in manual mode (89), hood active (96) and the status flags 97 and 100. This is
deliberate: a filter change that is due keeps the sensor on as a reminder. To react to only some of
them, match the code shown in Active Errors (for example `Filter_warning_flag`) in an automation or
template sensor.

ID 100 (`analysis_busy_warning`, "error analysis is in progress") is transient: the unit raises it
while it checks its errors, after **Reset All Errors** and at power-up until its initialisation is
complete, so Has Active Errors can be on briefly then. A reset does not clear warnings that describe
a condition, such as the filter flag; those stay on until the condition is gone, for example after the
filters are changed.

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

Descriptions are worded as in section 9.5 "ComfoAirQ alarm codes" (Table 2) of Zehnder's
ComfoConnect PRO Modbus installation manual (revision 137), as `<description> (<code>)`, so
the same alarm can be looked up by number or code. Error IDs follow the firmware 1.4.0+
numbering. The manual does not list IDs 63–65, 70–73 and 82–88; those keep the wording from
the aiocomfoconnect protocol notes and are unverified. An ID with no description shows as
`Unknown error`.

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

Home Assistant rejects entity states longer than 255 characters, so a very long list is
shortened with `...`. The full text is logged at debug level.

## Protocol

Requests go through a small queued request/response layer (`send_rmi_request`): requests
are sent one at a time, multi-frame responses (up to 7 data bytes per frame, last frame
flagged `0x80`) are reassembled in order, and a missing or out-of-order response, a
protocol error response, or a 2 s timeout ends the request without publishing. Responses
from the unit (`0x1F` prefix, request bit clear) are routed in by the CAN frame trigger in
the package; responses are not correlated by RMI sequence number. Bit *n* of the response
(least significant bit first) is error ID *n*.
