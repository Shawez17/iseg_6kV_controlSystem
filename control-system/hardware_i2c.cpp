#include "hardware_i2c.h"

void setupI2cHardware(TwoWire& bus, int sdaPin, int sclPin) {
  bus.setSDA(sdaPin);
  bus.setSCL(sclPin);
  bus.begin();
}

void scanI2cDevices(TwoWire& bus) {
  byte error = 0;
  byte address = 0;
  int deviceCount = 0;

  Serial.println("Scanning I2C bus...");

  for (address = 1; address < 127; ++address) {
    bus.beginTransmission(address);
    error = bus.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      Serial.println(address, HEX);
      deviceCount++;
    } else if (error != 2) {
      Serial.print("Error at 0x");
      Serial.println(address, HEX);
    }
  }

  if (deviceCount == 0) {
    Serial.println("No I2C devices found.");
  }
}
