#pragma once

#include <Arduino.h>

#include "state.h"

void initializeEthernetCommunication();
void pollEthernetCommunication(SystemState& state);
bool ethernetInterfaceReady();
IPAddress ethernetLocalIP();
