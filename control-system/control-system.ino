#include <Arduino.h>

#include "config.h"
#include "state.h"
#include "hardware_i2c.h"
#include "adc_readout.h"
#include "dac_control.h"
#include "display_ui.h"
#include "serial_commands.h"

Adafruit_ADS1115 adsPositive;
Adafruit_ADS1115 adsNegative;
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, TFT_CS, TFT_DC, TFT_RST);
DFRobot_GP8XXX_IIC gp8413(RESOLUTION_15_BIT, 0x58, &Wire1);

SystemState systemState;

void setup() {
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);

  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("Starting HV control system...");

  SPI1.setSCK(10);
  SPI1.setTX(11);
  SPI1.setRX(12);
  SPI1.setCS(TFT_CS);

  initDisplay(tft);
  setupI2cHardware(Wire1, I2C_SDA_PIN, I2C_SCL_PIN);
  scanI2cDevices(Wire1);

  setupAdcModule(adsPositive, adsNegative);
  setupDacModule(gp8413);

  systemState.debug_mode = false;
  Serial.println("System ready.");
}

void loop() {
  static unsigned long lastDisplayUpdateMs = 0;

  digitalWrite(LED_GPIO, HIGH);
  handleSerialCommands(Serial, systemState);
  updateAdcReadings(adsPositive, adsNegative, systemState);
  updateDacOutputs(gp8413, systemState);

  if (systemState.debug_mode && (millis() - lastDisplayUpdateMs > 250)) {
    renderStatus(tft, systemState);
    lastDisplayUpdateMs = millis();
  }

  digitalWrite(LED_GPIO, LOW);
  delay(50);
}
