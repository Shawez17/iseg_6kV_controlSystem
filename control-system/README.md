# control-system

The main control system for the HV-6kV-2Ch project runs on the WIZnet W5100S-EVB-Pico board, which uses an RP2040 MCU and the W5100 Ethernet controller.

## Code organization

The sketch is intentionally kept compact and focused on a few responsibilities:

- `control-system.ino` owns setup/loop orchestration only.
- `config.h` stores board-level constants, GPIO mapping, timing, and scaling values.
- `state.h` defines the single runtime state used by the control logic and display.
- `hardware_i2c.*` configures I2C, reads the ADS1115 channels, and updates the DAC outputs.
- `display_ui.*` draws the TFT UI and refreshes only the values that changed.
- `serial_communication.*` handles USB and debug command parsing while keeping serial output off when no host is connected.

## Communication interfaces

The board exposes exactly one active command interface at a time — USB serial or Ethernet — selected exclusively by the hardware `TRANSPORT_SEL_PIN` (GPIO 1). The firmware polls whichever transport is currently selected once per `loop()` iteration; the other transport is inactive. Use `CONF:SEL?` or `CONF:TRAN?` (see below) to confirm which mode is active.

### USB serial

- Baud rate: `9600` (8N1, no flow control), set via `SERIAL_BAUD_RATE` in [`config.h`](config.h).
- Connect the board's USB port to a host and open a serial terminal (Arduino IDE Serial Monitor, `screen`, `minicom`, PuTTY, etc.) at 9600 baud with newline (`\n`) line endings.
- The firmware never blocks waiting for a serial monitor; it keeps executing even if no host is connected, and only prints responses when a host is detected (`Serial` reports connected).
- USB mode is active whenever `TRANSPORT_SEL_PIN` reads `LOW` (or is left unconnected/floating to its default).

### Ethernet (TCP)

- The on-board WIZnet W5100S is configured for DHCP only — there is no static-IP fallback. On boot (and whenever the link is re-established), the firmware requests a lease from whatever DHCP server is on the network.
- MAC address: `02:60:51:10:50:01` (`ETHERNET_MAC_ADDRESS` in [`config.h`](config.h)). If multiple boards are deployed on the same network, this address must be made unique per board.
- TCP port: `5025` (`TCP_PORT` in [`config.h`](config.h)) — the standard SCPI/LXI port. Only one client connection is served at a time.
- Ethernet mode is active whenever `TRANSPORT_SEL_PIN` reads `HIGH`.
- Finding the board's IP address:
  - The TFT display briefly shows an `IP:x.x.x.x` banner across the top of the live view for a few seconds the first time a DHCP lease is acquired (or re-acquired after a link drop).
  - Alternatively, check your DHCP server's / router's lease table for the MAC address above.
- Once you have the IP, connect with any raw TCP client, for example:
  ```bash
  nc <board-ip> 5025
  ```
  or a serial-over-TCP terminal (PuTTY "Raw" connection type, etc.). Commands and responses use the same line-based, newline-terminated protocol as USB.
- If no lease can be obtained, the firmware raises a transport error and retries automatically every `ETHERNET_DHCP_RETRY_MS` (10 s) without blocking the rest of the control loop.

## Serial command reference

The same ASCII command parser is used on both transports (USB and the active Ethernet client). Commands are line based and are terminated with a newline (`\n`). Input is converted to uppercase internally, so command names are case-insensitive.

### Output behavior

- `DEBUG` and `DEBUG ON` enable debug output.
- `STOP` and `DEBUG OFF` disable debug output.
- Command responses are printed back on whichever transport sent the command (USB replies only when a serial host is connected; Ethernet replies are written to the connected client socket).
- Continuous telemetry printing (`flushToSerial`) runs only over USB, and only while debug mode is enabled.
- During an active error state, incoming commands are still parsed so `ERROR:CLEAR` can recover immediately.

### Supported commands

| Command | Description | Notes |
| --- | --- | --- |
| `DEBUG` / `DEBUG ON` | Enables debug mode. | Prints command echo and status responses. |
| `STOP` / `DEBUG OFF` | Disables debug mode. | Suppresses extra serial output. |
| `ERROR:CLEAR` | Clears the current active error state. | Used to reset error screens and faults. |
| `CONF:TRAN?` | Reports the active transport mode. | Returns `TRANSPORT=USB` or `TRANSPORT=ETHERNET`. |
| `CONF:SEL?` | Reports the transport selector pin state. | Returns `TRANSPORT_SEL_PIN=HIGH` or `TRANSPORT_SEL_PIN=LOW`. |
| `SOUR:VOLT<ch>,<volts>` | Sets the target set-point voltage for a channel. | Format: `SOUR:VOLT1,1200` or `SOUR:VOLT2,1500`. |
| `MEAS:VOLT?` / `MEAS:VOLT` | Prints the current measured and set-point voltages. | Includes +VSET, +HV, -VSET, and -HV values. |
| `MEAS:VOLT1` / `MEAS:VOLT1?` | Prints positive-channel voltage snapshot. | Returns +VSET and +HV. |
| `MEAS:VOLT2` / `MEAS:VOLT2?` | Prints negative-channel voltage snapshot. | Returns -VSET and -HV. |
| `MEAS:CURR?` / `MEAS:CURR` | Prints the current monitor values. | Includes +IMON and -IMON values. |
| `MEAS:CURR1` / `MEAS:CURR1?` | Prints positive-channel current monitor. | Returns +IMON. |
| `MEAS:CURR2` / `MEAS:CURR2?` | Prints negative-channel current monitor. | Returns -IMON. |
| `SOUR:DAC<ch>,<code>` | Sets a DAC raw code value. | Requires debug mode. |
| `DAC?` | Prints the current DAC codes. | Returns `DAC_CODE_0=... , DAC_CODE_1=...`. |
| `DAC0=<code>` | Writes a raw DAC code into channel 0. | Requires debug mode. |
| `DAC1=<code>` | Writes a raw DAC code into channel 1. | Requires debug mode. |
| `DAC=<code0>,<code1>` | Writes both DAC codes at once. | Requires debug mode. |

### Command details

#### `SOUR:VOLT` set-point command

- Format: `SOUR:VOLT<channel>,<voltage>`
- Valid channels: `1` (positive) and `2` (negative)
- The `<voltage>` value is a **non-negative magnitude** in the `0` to `VSET_MAX_VOLTS` range (6000 V by default) for both channels; the negative channel's actual HV output is negative by design (see `HV_FACTOR_NEG` in [`config.h`](config.h)), not because a negative number was sent.
- Example: `SOUR:VOLT1,2.500` sets the positive channel's target to 2.5 V
- Example: `SOUR:VOLT2,1.250` sets the negative channel's target to 1.25 V (which the hardware outputs as -1.25 V \* `HV_FACTOR_NEG` on the HV rail)
- Values are clamped to `[0, VSET_MAX_VOLTS]` before conversion to a DAC code; sending a negative number is clamped to `0`, not the magnitude of the negative value — do not prefix the value with `-`.

#### DAC commands

The raw DAC commands are intentionally debug-only for safety. They are accepted only when `DEBUG` is enabled.

- `DAC0=<0-32767>` updates the positive-channel DAC code
- `DAC1=<0-32767>` updates the negative-channel DAC code
- `DAC=<code0>,<code1>` updates both channels in one command
- `SOUR:DAC1,15000` and `SOUR:DAC2,18000` are also accepted in the same debug-only path

#### Diagnostics

- `MEAS:VOLT?` (or `MEAS:VOLT`) prints both channels as a compact snapshot
- `MEAS:VOLT1` and `MEAS:VOLT2` print per-channel voltage snapshots
- `MEAS:CURR?` (or `MEAS:CURR`) prints both current monitor values
- `MEAS:CURR1` and `MEAS:CURR2` print per-channel current monitor values
- `CONF:TRAN?` reports whether the board is in `USB` or `ETHERNET` mode
- `CONF:SEL?` reports the raw `TRANSPORT_SEL_PIN` state as `HIGH` or `LOW`
- `ERROR:CLEAR` resets the fault state so the system can resume after a command or transport error

### Example session

```text
DEBUG ON
SOUR:VOLT1,2.500
SOUR:VOLT2,1.250
MEAS:VOLT?
MEAS:VOLT1
DAC? 
STOP
```

This command set is intentionally simple and is designed to support both local debugging and remote control via the active communication transport.