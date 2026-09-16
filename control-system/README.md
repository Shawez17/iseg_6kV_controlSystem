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

## Serial behavior

- `DEBUG` and `DEBUG ON` enable debug output.
- `STOP` and `DEBUG OFF` disable debug output.
- Serial output is only emitted when debug mode is active or a serial host is connected.
- The firmware does not wait forever for a monitor; it continues running without blocking on `Serial`.

## Example commands

- `DAC?` prints the current DAC codes.
- `DAC0=<0-32767>` updates DAC channel 0.
- `DAC1=<0-32767>` updates DAC channel 1.
- `DAC=<code0>,<code1>` updates both DAC channels together.