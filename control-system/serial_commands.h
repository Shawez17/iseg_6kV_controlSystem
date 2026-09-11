#pragma once

#include <Arduino.h>

#include "state.h"

enum class CommandSource : uint8_t {
	Usb,
	Ethernet,
};

void handleSerialCommands(Stream& serial, SystemState& state, CommandSource source);
