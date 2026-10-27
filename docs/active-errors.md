# Optional active error reporting

Include `packages/active_errors.yml` alongside the normal base/dashboard package.
The unit's active errors are read with RMI `GETACTIVEERRORS` (`80 03 01`) every
`error_poll_interval` (default 5 minutes, first read 15 s after boot) and exposed as:

- **Active Errors** (text sensor): `No active errors`, or each active error as
  `<id>: <description> (<code>)`, separated by `; `.
- **Has Active Errors** (binary sensor, `problem` device class): on while any error is active.

Nothing is polled unless the package is included. Set `error_poll_interval: never` to
include the entities but not poll. The read is a single GET command; it changes no setting.

## Wording

Descriptions are worded as in section 9.5 "ComfoAirQ alarm codes" (Table 2) of Zehnder's
ComfoConnect PRO Modbus installation manual (revision 137), as `<description> (<code>)`, so
the same alarm can be looked up by number or code. Error IDs follow the firmware 1.4.0+
numbering. The manual does not list IDs 63–65, 70–73 and 82–88; those keep the wording from
the aiocomfoconnect protocol notes and are unverified. An ID with no description shows as
`Unknown error`.

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
