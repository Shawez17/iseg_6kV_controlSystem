# HV-6kV-2Ch

HV-6kV-2Ch is a project to build a dual channel high voltage power supply with 0 to +/- 6kV output that can be remotely monitored and controlled remotely. It is based on building a custom high precision control and monitoring system for iSeg's resonant Royer DC-DC converter modules.

## Project structure

```text
iseg_6kV_controlSystem/
├── README.md
├── control-system/
│   ├── control-system.ino          # main sketch entry point; thin orchestration layer
│   ├── config.h                    # pin mapping, hardware constants, and compile-time settings
│   ├── state.h                     # runtime state for ADC values, voltages, and debug mode
│   ├── hardware_i2c.h/.cpp         # I2C bus setup, ADS1115 ADC readout, GP8413 DAC control
│   ├── serial_communication.h/.cpp # USB + Ethernet command parsing and transport selection
│   ├── ethernet_communication.h/.cpp # DHCP-only Ethernet transport and TCP command server
│   ├── display_ui.h/.cpp           # TFT UI initialization and display rendering
│   ├── error_handling.h/.cpp       # error state tracking and error screen rendering
│   ├── README.md                   # serial/Ethernet command reference and usage details
│   └── ...
├── mcu_tests/
│   └── ...
├── PCB-design/
│   └── ...
├── references/
│   └── ...
└── ...
```

