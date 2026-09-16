#include <Arduino.h>

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

void setup() {
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);

  initializeSerialPort();
  delay(100);
  // Serial.println("Serial ready");

  SPI1.setSCK(10);
  SPI1.setTX(11);
  SPI1.setRX(12);
  SPI1.setCS(TFT_CS);
  initDisplay(tft);
  setupModeSelectPins();
  setupI2cHardware(Wire1, I2C_SDA_PIN, I2C_SCL_PIN);
  scanI2cDevices(Wire1);
  setupAdcModule(adsPositive, adsNegative);
  setupDacModule(gp8413, systemState);
  
  systemState.transport_mode = TransportMode::Usb;
  systemState.display_mode = DisplayMode::Live;
  systemState.debug_mode = false;
}

void loop() {
  updateTransportMode(systemState);
  updateErrorState();

  if (hasActiveError()) {
    if (systemState.transport_mode == TransportMode::Ethernet) {
      pollEthernetCommunication(systemState);
    } 
    else {
      handleSerialCommands(Serial, systemState, CommandSource::Usb);
    }

    if (hasActiveError()) {
      renderDisplay(tft, systemState);
      return;
    }
  }

  if (systemState.transport_mode == TransportMode::Ethernet) {
    pollEthernetCommunication(systemState);
  } 
  else {
    handleSerialCommands(Serial, systemState, CommandSource::Usb);
  }

  updateDacOutputs(gp8413, systemState);
  renderDisplay(tft, systemState);
  updateAdcReadings(adsPositive, adsNegative, systemState);
  flushToSerial(systemState);

  digitalWrite(LED_GPIO, !digitalRead(LED_GPIO));
}