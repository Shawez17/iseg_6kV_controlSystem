#pragma once

#include <Arduino.h>
#include "state.h"

void initNetworkTransport(SystemState& state);
void pollNetworkTransport(SystemState& state);
