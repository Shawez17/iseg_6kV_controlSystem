#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "state.h"

void initDisplay(Adafruit_ST7789& tft);
void renderStatus(Adafruit_ST7789& tft, const SystemState& state);