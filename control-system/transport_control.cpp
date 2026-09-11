#include "transport_control.h"

#include "config.h"

namespace {

bool isSelectablePin(int pin) {
  return pin >= 0;
}

}  // namespace

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