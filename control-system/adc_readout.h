#pragma once

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

#include "config.h"
#include "state.h"

void setupAdcModule(Adafruit_ADS1115& positive, Adafruit_ADS1115& negative);
void updateAdcReadings(Adafruit_ADS1115& positive,
                       Adafruit_ADS1115& negative,
                       SystemState& state);