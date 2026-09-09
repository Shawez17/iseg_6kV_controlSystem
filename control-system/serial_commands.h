#pragma once

#include <Arduino.h>

#include "state.h"

void handleSerialCommands(Stream& serial, SystemState& state);
