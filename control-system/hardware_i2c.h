#pragma once

#include "config.h"
#include "state.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <DFRobot_GP8XXX.h>

void setupI2cHardware(TwoWire& bus, int sdaPin, int sclPin);
void scanI2cDevices(TwoWire& bus);

void setupAdcModule(Adafruit_ADS1115& positive, Adafruit_ADS1115& negative);
void updateAdcReadings(Adafruit_ADS1115& positive,
                       Adafruit_ADS1115& negative,
                       SystemState& state);
void setupDacModule(DFRobot_GP8XXX_IIC& dac, SystemState& state);
void updateDacOutputs(DFRobot_GP8XXX_IIC& dac, const SystemState& state);
void flush_to_serial(const SystemState& state);

