#pragma once

#include <Arduino.h>
#include <Wire.h>

void setupI2cHardware(TwoWire& bus, int sdaPin, int sclPin);
void scanI2cDevices(TwoWire& bus);