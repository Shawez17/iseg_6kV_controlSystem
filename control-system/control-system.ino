#include <Arduino.h>

#include "config.h"
#include "state.h"
#include "hardware_i2c.h"
#include "display_ui.h"
#include "serial_commands.h"
#include "transport_control.h"
#include "network_transport.h"

Adafruit_ADS1115 adsPositive;
Adafruit_ADS1115 adsNegative;
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, TFT_CS, TFT_DC, TFT_RST);
DFRobot_GP8XXX_IIC gp8413(RESOLUTION_15_BIT, 0x58, &Wire1);

SystemState systemState;

void setup() {
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);

  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }

  Serial.println("Starting HV control system...");

  SPI1.setSCK(TFT_SCK);
  SPI1.setTX(TFT_TX);
  SPI1.setRX(TFT_RX);
  SPI1.setCS(TFT_CS);

  initDisplay(tft);
  setupModeSelectPins();
  setupI2cHardware(Wire1, I2C_SDA_PIN, I2C_SCL_PIN); 
  scanI2cDevices(Wire1);
  setupAdcModule(adsPositive, adsNegative);
  setupDacModule(gp8413, systemState);

  initNetworkTransport(systemState);

  systemState.transport_mode = TransportMode::Usb;
  systemState.display_mode = DisplayMode::Live;
  systemState.debug_mode = false;
  Serial.println("Setup is done\nSystem ready.");
}

void loop() {

  digitalWrite(LED_GPIO, HIGH);
  delay(1000);
  digitalWrite(LED_GPIO, LOW);
  delay(1000);
  Serial.println("in the loop");
  static unsigned long lastDisplayUpdateMs = 0;
  updateTransportMode(systemState);
  Serial.println("in the loop_1");
  updateDisplayMode(systemState);
  Serial.println("in the loop_2, yay!");
  handleSerialCommands(Serial, systemState, CommandSource::Usb); 
  Serial.println("in the loop_3, yay! again , serial works");
  //if (systemState.transport_mode == TransportMode::Ethernet) {
  //  pollNetworkTransport(systemState);
  //
  //}
  updateDacOutputs(gp8413, systemState);
  Serial.println("in the loop_3.1, dac updates");
  renderDisplay(tft, systemState);
  Serial.println("in the loop_3.1, tft updates");

  updateAdcReadings(adsPositive, adsNegative, systemState);
  Serial.println("in the loop_3.1, adc updates");
    //if (millis() - lastDisplayUpdateMs > 250) {
   //if (millis() - lastDisplayUpdateMs > 250) {
  //  lastDisplayUpdateMs = millis();
  //}
  
  
  flush_to_serial(systemState);
  Serial.println("in the loop_4, yay! again , ethernet works! works");
  digitalWrite(LED_GPIO, HIGH);
  Serial.println("in the loop_5, yay! again , all works");
  delay(50);
}
