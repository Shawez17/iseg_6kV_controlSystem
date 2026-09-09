#include "serial_commands.h"

void handleSerialCommands(Stream& serial, SystemState& state) {
  if (serial.available() <= 0) {
    return;
  }

  String command = serial.readStringUntil('\r');
  command.trim();
  serial.println(command);

  if (command == "DEBUG") {
    state.debug_mode = true;
    serial.println("Entering debug mode...");
  } else if (command == "STOP") {
    state.debug_mode = false;
    serial.println("Exiting debug mode...");
  } else {
    serial.println("ERROR");
  }
}
