#include <Arduino.h>
#include "hardware/watchdog.h"

#include "config.h"
#include "state.h"
#include "hardware_i2c.h"
#include "display_ui.h"
#include "serial_communication.h"
#include "ethernet_communication.h"
#include "error_handling.h"

Adafruit_ADS1115 adsPositive;
Adafruit_ADS1115 adsNegative;
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, TFT_CS, TFT_DC, TFT_RST);
DFRobot_GP8XXX_IIC gp8413(RESOLUTION_15_BIT, 0x58, &Wire1);

SystemState systemState;
bool startupScreensCompleted = false;

void setup() {
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);

  systemState.transport_mode = TransportMode::Usb;
  systemState.display_mode = DisplayMode::Live;
  systemState.debug_mode = false;

  initializeSerialPort();
  delay(100);
  // Serial.println("Serial ready");

  SPI1.setSCK(10);
  SPI1.setTX(11);
  SPI1.setRX(12);
  SPI1.setCS(TFT_CS);
  initDisplay(tft);

  // Always run the startup sequence before any transport/peripheral errors
  // can preempt the normal UI path.
  renderDisplay(tft, systemState);  // Welcome screen
  renderDisplay(tft, systemState);  // Warning screen -> first live frame
  startupScreensCompleted = true;

  setupModeSelectPins();
  setupI2cHardware(Wire1, I2C_SDA_PIN, I2C_SCL_PIN);
  scanI2cDevices(Wire1);
  setupAdcModule(adsPositive, adsNegative);
  setupDacModule(gp8413, systemState);
  // initializeEthernetCommunication();

  // Arm the watchdog last, once all blocking setup work (including the
  // Ethernet DHCP attempt) has already completed. From here on, the ADC
  // sampling loop pets it between channel reads, loop() pets it once per
  // iteration, and the display/Ethernet code temporarily widens it around
  // any of their own known-long blocking calls.
  watchdog_enable(I2C_WATCHDOG_TIMEOUT_MS, true);
}

void loop() {
  watchdog_update();

  // Fallback guard: if setup flow changes in future and misses startup
  // screens, force them once from loop.
  if (!startupScreensCompleted) {
    renderDisplay(tft, systemState);
    renderDisplay(tft, systemState);
    startupScreensCompleted = true;
  }

  updateTransportMode(systemState);
  updateErrorState();

  // Handle exactly one round of incoming commands per iteration (this also
  // lets ERROR:CLEAR be processed promptly while an error is active).
  if (systemState.transport_mode == TransportMode::Ethernet) {
    pollEthernetCommunication(systemState);
  } else {
    handleSerialCommands(Serial, systemState, CommandSource::Usb);
  }

  if (hasActiveError()) {
    renderDisplay(tft, systemState);
    return;
  }

  updateDacOutputs(gp8413, systemState);
  renderDisplay(tft, systemState);
  updateAdcReadings(adsPositive, adsNegative, systemState);
  flushToSerial(systemState);

  digitalWrite(LED_GPIO, !digitalRead(LED_GPIO));
}