#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;

const uint8_t MCP_ADDR = 0x60;
const float DAC_VREF = 5.0;      // change this to match your DAC's actual Vcc/reference voltage
const uint16_t MAX_DAC_BITS = 4095;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();   // default ESP32 I2C pins: SDA=GPIO21, SCL=GPIO22

  if (!dac.begin(MCP_ADDR)) {
    Serial.println("MCP4725 not found - check wiring/address");
    while (1) delay(10);
  }
  dacSetValue = 2000;

  V_set_dac = DAC_VREF * ((float)dacSetValue / MAX_DAC_BITS);

  if (dacSetValue > MAX_DAC_BITS) dacSetValue = MAX_DAC_BITS;

  dac.setVoltage(dacValue, false);

  Serial.print("Target voltage: ");
  Serial.print(V_set_dac, 5);
  Serial.println(" V");
  Serial.print("DAC code sent: ");
  Serial.println(dacSetValue);
}

void loop() {
  // nothing needed here, DAC holds its output
}
