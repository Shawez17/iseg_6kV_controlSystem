#pragma once

#include <Arduino.h>

#include "state.h"

void setupModeSelectPins();
void updateTransportMode(SystemState& state);
void updateDisplayMode(SystemState& state);