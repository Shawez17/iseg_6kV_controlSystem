#include "serial_communication.h"

#include "config.h"
#include "error_handling.h"

namespace {

constexpr uint32_t SERIAL_BAUD_RATE = 9600;

bool canPrintToStream(Stream& serial, const SystemState& state) {
  return state.debug_mode;
}

bool isSelectablePin(int pin) {
  return pin >= 0;
}

uint16_t voltageToDacCode(float volts) {
  const float limitedVolts = constrain(volts, 0.0f, VSET_MAX_VOLTS);
  const float normalized = limitedVolts / VSET_MAX_VOLTS;
  return static_cast<uint16_t>(normalized * static_cast<float>(MAX_DAC_BITS));
}

bool parseChannelAndValue(const String& payload, uint8_t& channel, float& value) {
  const int commaIndex = payload.indexOf(',');
  if (commaIndex < 0) {
    return false;
  }

  const String channelText = payload.substring(0, commaIndex);
  const String valueText = payload.substring(commaIndex + 1);
  channel = static_cast<uint8_t>(channelText.toInt());
  value = valueText.toFloat();
  return (channel == 1 || channel == 2);
}

uint16_t clampDacCode(long value) {
  if (value < 0) {
    return 0;
  }
  if (value > MAX_DAC_BITS) {
    return MAX_DAC_BITS;
  }
  return static_cast<uint16_t>(value);
}

bool parseDacAssignment(const String& payload, uint16_t& code) {
  char* endPtr = nullptr;
  const long parsedValue = strtol(payload.c_str(), &endPtr, 10);
  if (endPtr == payload.c_str() || *endPtr != '\0') {
    return false;
  }

  code = clampDacCode(parsedValue);
  return true;
}

void printDacCodes(Stream& serial, const SystemState& state) {
  if (!canPrintToStream(serial, state)) {
    return;
  }

  serial.print("DAC_CODE_0=");
  serial.print(state.dac_code_0);
  serial.print(", DAC_CODE_1=");
  serial.println(state.dac_code_1);
}

const char* transportText(TransportMode mode) {
  return (mode == TransportMode::Ethernet) ? "ETHERNET" : "USB";
}

void printLine(Stream& serial, const SystemState& state, const String& message) {
  if (canPrintToStream(serial, state)) {
    serial.println(message);
  }
}

void printLine(Stream& serial, const SystemState& state, const __FlashStringHelper* message) {
  if (canPrintToStream(serial, state)) {
    serial.println(message);
  }
}

bool isVoltageCommand(const String& command) {
  return command.startsWith("SOUR:VOLT") || command.startsWith("MEAS:VOLT") || command.startsWith("CONF:IP");
}

bool isDebugDacCommand(const String& command) {
  return command == "DAC?" || command.startsWith("DAC0=") || command.startsWith("DAC1=") || command.startsWith("DAC=") || command.startsWith("SOUR:DAC");
}

}  // namespace

void initializeSerialPort() {
  Serial.begin(SERIAL_BAUD_RATE);
  // Do not block on Serial connection; the firmware should keep running without a monitor attached.
}

bool isSerialHostConnected() {
  return static_cast<bool>(Serial);
}

bool isDebugOutputEnabled(const SystemState& state) {
  return state.debug_mode;
}

void setupModeSelectPins() {
  if (isSelectablePin(TRANSPORT_SEL_PIN)) {
    pinMode(TRANSPORT_SEL_PIN,
            TRANSPORT_SEL_USE_PULLUP ? INPUT_PULLUP : INPUT_PULLDOWN);
  }

  if (isSelectablePin(DISPLAY_SEL_PIN)) {
    pinMode(DISPLAY_SEL_PIN,
            DISPLAY_SEL_USE_PULLUP ? INPUT_PULLUP : INPUT_PULLDOWN);
  }
}

void updateTransportMode(SystemState& state) {
  if (isSelectablePin(TRANSPORT_SEL_PIN)) {
    state.transport_mode = (digitalRead(TRANSPORT_SEL_PIN) == HIGH)
                               ? TransportMode::Ethernet
                               : TransportMode::Usb;
  }
}

void updateDisplayMode(SystemState& state) {
  if (isSelectablePin(DISPLAY_SEL_PIN)) {
    state.display_mode = (digitalRead(DISPLAY_SEL_PIN) == HIGH)
                             ? DisplayMode::Trend
                             : DisplayMode::Live;
  }
}

void handleSerialCommands(Stream& serial, SystemState& state, CommandSource source) {
  if (serial.available() <= 0) {
    return;
  }

  String command = serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();

  if (canPrintToStream(serial, state)) {
    serial.println(command);
  }

  if (command == "DEBUG" || command == "DEBUG ON") {
    state.debug_mode = true;
    printLine(serial, state, "Entering debug mode...");
  } else if (command == "STOP" || command == "DEBUG OFF") {
    state.debug_mode = false;
    printLine(serial, state, "Exiting debug mode...");
  } else if (command == "ERROR:CLEAR") {
    clearErrorState();
    printLine(serial, state, "Error state cleared");
  } else if (command == "CONF:TRAN?") {
    if (canPrintToStream(serial, state)) {
      serial.print("TRANSPORT=");
      serial.println(transportText(state.transport_mode));
    }
  } else if (command.startsWith("SOUR:VOLT")) {
    if (source == CommandSource::Usb && state.transport_mode == TransportMode::Ethernet) {
      raiseError(serial, ErrorCode::Transport, "TRANSPORT", "USB control disabled while Ethernet is active");
      return;
    }

    const String payload = command.substring(9);
    uint8_t channel = 0;
    float volts = 0.0f;
    if (!parseChannelAndValue(payload, channel, volts)) {
      raiseError(serial, ErrorCode::Command, "COMMAND", "Expected SOUR:VOLT <ch>,<volts>");
      return;
    }

    if (channel == 1) {
      state.target_vset_pos = volts;
      state.dac_code_0 = voltageToDacCode(volts);
      if (canPrintToStream(serial, state)) {
        serial.print("VOLT1=");
        serial.print(state.target_vset_pos, 3);
        serial.print("V DAC=");
        serial.println(state.dac_code_0);
      }
    } else {
      state.target_vset_neg = volts;
      state.dac_code_1 = voltageToDacCode(volts);
      if (canPrintToStream(serial, state)) {
        serial.print("VOLT2=");
        serial.print(state.target_vset_neg, 3);
        serial.print("V DAC=");
        serial.println(state.dac_code_1);
      }
    }
  } else if (command == "MEAS:VOLT?") {
    if (canPrintToStream(serial, state)) {
      serial.print("+VSET=");
      serial.print(state.vset_pos, 3);
      serial.print(", +HV=");
      serial.print(state.hv_pos, 1);
      serial.print(", -VSET=");
      serial.print(state.vset_neg, 3);
      serial.print(", -HV=");
      serial.println(state.hv_neg, 1);
    }
  } else if (command == "MEAS:CURR?") {
    if (canPrintToStream(serial, state)) {
      serial.print("+IMON=");
      serial.print(state.imon_pos, 4);
      serial.print(", -IMON=");
      serial.println(state.imon_neg, 4);
    }
  } else if (command.startsWith("SOUR:DAC")) {
    if (!state.debug_mode) {
      raiseError(serial, ErrorCode::Command, "COMMAND", "Raw DAC commands require debug mode");
      return;
    }

    const String payload = command.substring(8);
    uint8_t channel = 0;
    float codeValue = 0.0f;
    if (!parseChannelAndValue(payload, channel, codeValue)) {
      raiseError(serial, ErrorCode::Command, "COMMAND", "Expected SOUR:DAC <ch>,<code>");
      return;
    }

    const uint16_t code = static_cast<uint16_t>(constrain(codeValue, 0.0f, static_cast<float>(MAX_DAC_BITS)));
    if (channel == 1) {
      state.dac_code_0 = code;
    } else {
      state.dac_code_1 = code;
    }
    if (canPrintToStream(serial, state)) {
      serial.print("DAC");
      serial.print(channel);
      serial.print("=");
      serial.println(code);
    }
  } else if (command == "DAC?") {
    printDacCodes(serial, state);
  } else if (command.startsWith("DAC0=")) {
    if (!state.debug_mode) {
      raiseError(serial, ErrorCode::Command, "COMMAND", "Raw DAC commands require debug mode");
      return;
    }
    if (parseDacAssignment(command.substring(5), state.dac_code_0)) {
      if (canPrintToStream(serial, state)) {
        serial.print("Updated DAC_CODE_0 to ");
        serial.println(state.dac_code_0);
      }
    } else {
      raiseError(serial, ErrorCode::Command, "COMMAND", "Expected DAC0=<0-32767>");
    }
  } else if (command.startsWith("DAC1=")) {
    if (!state.debug_mode) {
      raiseError(serial, ErrorCode::Command, "COMMAND", "Raw DAC commands require debug mode");
      return;
    }
    if (parseDacAssignment(command.substring(5), state.dac_code_1)) {
      if (canPrintToStream(serial, state)) {
        serial.print("Updated DAC_CODE_1 to ");
        serial.println(state.dac_code_1);
      }
    } else {
      raiseError(serial, ErrorCode::Command, "COMMAND", "Expected DAC1=<0-32767>");
    }
  } else if (command.startsWith("DAC=")) {
    if (!state.debug_mode) {
      raiseError(serial, ErrorCode::Command, "COMMAND", "Raw DAC commands require debug mode");
      return;
    }

    const String payload = command.substring(4);
    const int separatorIndex = payload.indexOf(',');
    if (separatorIndex < 0) {
      raiseError(serial, ErrorCode::Command, "COMMAND", "Expected DAC=<code0>,<code1>");
      return;
    }

    const String code0Text = payload.substring(0, separatorIndex);
    const String code1Text = payload.substring(separatorIndex + 1);
    uint16_t newCode0 = state.dac_code_0;
    uint16_t newCode1 = state.dac_code_1;

    if (!parseDacAssignment(code0Text, newCode0) || !parseDacAssignment(code1Text, newCode1)) {
      printLine(serial, state, "ERROR: expected DAC=<code0>,<code1>");
      return;
    }

    state.dac_code_0 = newCode0;
    state.dac_code_1 = newCode1;
    if (canPrintToStream(serial, state)) {
      serial.print("Updated DAC_CODE_0 to ");
      serial.print(state.dac_code_0);
      serial.print(", DAC_CODE_1 to ");
      serial.println(state.dac_code_1);
    }
  } else {
    raiseError(serial, ErrorCode::Command, "COMMAND", "Unsupported command");
  }
}

void flushToSerial(const SystemState& state) {
  if (!isDebugOutputEnabled(state)) {
    return;
  }

  Serial.printf("POS:  Vmon = %.4f V, Imon = %.4f V, Vset = %.4f V, HV = %.4f V\n",
                state.vmon_pos,
                state.imon_pos,
                state.vset_pos,
                state.hv_pos);

  Serial.printf("NEG:  Vmon = %.4f V, Imon = %.4f V, Vset = %.4f V, HV = %.4f V\n",
                state.vmon_neg,
                state.imon_neg,
                state.vset_neg,
                state.hv_neg);
}
