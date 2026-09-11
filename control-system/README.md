# control-system

The main control system for the HV-6kV-2Ch project is implemented using Wiznet W5100S-EVB-Pico microcontroller module containing a RP2040 MCU and the W5100 Ethernet controller.

## Code organization

The project is organized into a small Arduino-style modular layout:

- `control-system.ino` is intentionally minimal. It only initializes hardware and calls the subsystem functions.
- `config.h` holds board-specific constants such as GPIO numbers, sampling counts, timing delays, and scaling factors.
- `state.h` defines the current runtime state so the modules share a single source of truth instead of multiple unrelated globals.
- `hardware_i2c.*` handles the I2C bus setup, scanning, and bus-level configuration.
- `adc_readout.*` reads the ADS1115 channels, averages samples, and converts ADC counts into engineering units.
- `dac_control.*` is responsible for writing the DAC values that command the HV system output.
- `display_ui.*` draws the TFT status screen and shows live system values.
- `serial_commands.*` reads Serial commands such as `DEBUG` and `STOP` and toggles runtime behavior.


## Serial commands

- `DEBUG` enables debug mode.
- `STOP` disables debug mode.
- `DAC?` prints the current DAC codes.
- `DAC0=<0-32767>` updates DAC channel 0.
- `DAC1=<0-32767>` updates DAC channel 1.
- `DAC=<code0>,<code1>` updates both DAC channels together.