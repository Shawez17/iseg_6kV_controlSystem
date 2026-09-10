# HV-6kV-2Ch

HV-6kV-2Ch is a project to build a dual channel high voltage power supply with 0 to +/- 6kV output that can be remotely monitored and controlled remotely. It is based on building a custom high precision control and monitoring system for iSeg's resonant Royer DC-DC converter modules.

## Project structure

```text
iseg_6kV_controlSystem/
├── README.md
├── control-system/
│   ├── control-system.ino     # main sketch entry point; thin orchestration layer
│   ├── config.h               # pin mapping, hardware constants, and compile-time settings
│   ├── state.h                # runtime state for ADC values, voltages, and debug mode
│   ├── hardware_i2c.h/.cpp    # I2C bus initialization and device discovery
│   ├── adc_readout.h/.cpp     # ADC setup and averaged voltage/current sampling
│   ├── dac_control.h/.cpp     # DAC writing and output control logic
│   ├── display_ui.h/.cpp      # TFT UI initialization and display rendering
│   ├── serial_commands.h/.cpp # debug command parsing over Serial
│   └── ...
├── mcu_tests/
│   └── ...
├── PCB-design/
│   └── ...
├── references/
│   └── ...
└── ...
```

