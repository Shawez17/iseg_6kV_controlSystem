#pragma once

#include "config.h"
#include "state.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <DFRobot_GP8XXX.h>

void setupI2cHardware(TwoWire& bus, int sdaPin, int sclPin);
void scanI2cDevices(TwoWire& bus);

// Raw averaged ADC counts for channels 1-3 of a single ADS1115 device
// (channel 0 is unused on this board and is intentionally not read).
struct AdcChannelReadout {
  float ch1 = 0.0f;
  float ch2 = 0.0f;
  float ch3 = 0.0f;
};

AdcChannelReadout readAdcChannels(Adafruit_ADS1115& adc);

void setupAdcModule(Adafruit_ADS1115& positive, Adafruit_ADS1115& negative);
void updateAdcReadings(Adafruit_ADS1115& positive,
                       Adafruit_ADS1115& negative,
                       SystemState& state);
void setupDacModule(DFRobot_GP8XXX_IIC& dac, SystemState& state);
void updateDacOutputs(DFRobot_GP8XXX_IIC& dac, const SystemState& state);


