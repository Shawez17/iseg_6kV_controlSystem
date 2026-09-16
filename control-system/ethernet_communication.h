#pragma once

#include <Arduino.h>

#include "state.h"

void initializeEthernetCommunication();
void pollEthernetCommunication(SystemState& state);
bool ethernetClientConnected();
bool ethernetInterfaceReady();
IPAddress ethernetLocalIP();
