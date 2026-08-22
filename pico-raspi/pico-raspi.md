#include "Arduino.h"
#include <Wire.h>
#define LED_GPIO 25

void setup() {
  pinMode(LED_GPIO, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin();

  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();
}

void loop() {
  digitalWrite(LED_GPIO, HIGH);
  delay(1000);
  digitalWrite(LED_GPIO, LOW);
  delay(1000);

  byte error, address;
  int nDevices = 0;

  delay(3000);

  // --- Scan Wire (I2C0) ---
  Serial.println("Scanning Wire (I2C0) ...");
  nDevices = 0;
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C0: device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial.printf("I2C0: error %u at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("I2C0: No devices found");
  }

  delay(500);

  // --- Scan Wire1 (I2C1) ---
  Serial.println("Scanning Wire1 (I2C1) ...");
  nDevices = 0;
  for (address = 0x01; address < 0x7f; address++) {
    Wire1.beginTransmission(address);
    error = Wire1.endTransmission();
    if (error == 0) {
      Serial.printf("I2C1: device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial.printf("I2C1: error %u at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("I2C1: No devices found");
  }
}
