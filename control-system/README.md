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

## Serial command reference

The firmware accepts ASCII commands over the USB serial port and also supports the same command parser over an active Ethernet client. Commands are line based and are terminated with a newline (`\n`). Input is converted to uppercase internally, so command names are case-insensitive.

### Output behavior

- `DEBUG` and `DEBUG ON` enable debug output.
- `STOP` and `DEBUG OFF` disable debug output.
- Serial output is intentionally gated: it only prints when debug mode is enabled.
- The firmware never blocks waiting for a serial monitor; it keeps executing even if no host is connected.
- Ethernet-mode traffic follows the same command parser, but USB control is rejected when Ethernet is active.

### Supported commands

| Command | Description | Notes |
| --- | --- | --- |
| `DEBUG` / `DEBUG ON` | Enables debug mode. | Prints command echo and status responses. |
| `STOP` / `DEBUG OFF` | Disables debug mode. | Suppresses extra serial output. |
| `ERROR:CLEAR` | Clears the current active error state. | Used to reset error screens and faults. |
| `CONF:TRAN?` | Reports the active transport mode. | Returns `TRANSPORT=USB` or `TRANSPORT=ETHERNET`. |
| `SOUR:VOLT<ch>,<volts>` | Sets the target set-point voltage for a channel. | Format: `SOUR:VOLT1,1200` or `SOUR:VOLT2,1500`. |
| `MEAS:VOLT?` | Prints the current measured and set-point voltages. | Includes +VSET, +HV, -VSET, and -HV values. |
| `MEAS:CURR?` | Prints the current monitor values. | Includes +IMON and -IMON values. |
| `SOUR:DAC<ch>,<code>` | Sets a DAC raw code value. | Requires debug mode. |
| `DAC?` | Prints the current DAC codes. | Returns `DAC_CODE_0=... , DAC_CODE_1=...`. |
| `DAC0=<code>` | Writes a raw DAC code into channel 0. | Requires debug mode. |
| `DAC1=<code>` | Writes a raw DAC code into channel 1. | Requires debug mode. |
| `DAC=<code0>,<code1>` | Writes both DAC codes at once. | Requires debug mode. |

### Command details

#### `SOUR:VOLT` set-point command

- Format: `SOUR:VOLT<channel>,<voltage>`
- Valid channels: `1` and `2`
- Example: `SOUR:VOLT1,2.500` sets the positive set point to 2.5 V
- Example: `SOUR:VOLT2,-1.250` sets the negative set point to -1.25 V
- Voltage values are clamped to the configured maximum set point range before conversion to DAC code.
- The command is rejected if USB control is attempted while Ethernet is the active transport.

#### DAC commands

The raw DAC commands are intentionally debug-only for safety. They are accepted only when `DEBUG` is enabled.

- `DAC0=<0-32767>` updates the positive-channel DAC code
- `DAC1=<0-32767>` updates the negative-channel DAC code
- `DAC=<code0>,<code1>` updates both channels in one command
- `SOUR:DAC1,15000` and `SOUR:DAC2,18000` are also accepted in the same debug-only path

#### Diagnostics

- `MEAS:VOLT?` prints the current readings as a compact snapshot
- `MEAS:CURR?` prints current monitor values for both channels
- `CONF:TRAN?` reports whether the board is in `USB` or `ETHERNET` mode
- `ERROR:CLEAR` resets the fault state so the system can resume after a command or transport error

### Example session

```text
DEBUG ON
SOUR:VOLT1,2.500
SOUR:VOLT2,-1.250
MEAS:VOLT?
DAC? 
STOP
```

This command set is intentionally simple and is designed to support both local debugging and remote control via the active communication transport.