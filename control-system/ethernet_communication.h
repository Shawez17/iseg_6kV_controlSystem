#pragma once

#include "state.h"

void initializeEthernetCommunication();
void pollEthernetCommunication(SystemState& state);
bool ethernetClientConnected();