#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MCP4725.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_ADS1115 ads1;   // ADS1115 at 0x48

const uint8_t ADC_CHANNEL_0 = 0;  // A0 = DAC output
const uint8_t ADC_CHANNEL_1 = 1;  // A1 = GND
const uint8_t ADC_CHANNEL_2 = 2;  // A2 = 5V reference

const float ADS_LSB_VOLTS = 0.0001875;

Adafruit_MCP4725 dac;

const uint8_t MCP_ADDR = 0x60;
const uint16_t MAX_DAC_BITS = 4095;

uint16_t dacSetValue;

float V_set_dac;

const uint16_t NUM_SAMPLES = 100;
const uint16_t SAMPLE_DELAY_MS = 0;



void setup() {

  Serial.begin(115200);
  delay(1000);

  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found!");
    while (1) {
      delay(10);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.println("Init...");
  display.display();

  ads1.setGain(GAIN_TWOTHIRDS);

  dacSetValue = 2000;

  dac.setVoltage(dacSetValue, false);

  Serial.println();
  Serial.print("DAC bits: ");
  Serial.println(dacSetValue);

  display.clearDisplay();

  display.display();

  delay(1500);
}

void loop() {

  long sum0 = 0;
  long sum1 = 0;
  long sum2 = 0;


  for (uint16_t i = 0; i < NUM_SAMPLES; i++) {

    sum0 += ads1.readADC_SingleEnded(ADC_CHANNEL_0);
    sum1 += ads1.readADC_SingleEnded(ADC_CHANNEL_1);
    sum2 += ads1.readADC_SingleEnded(ADC_CHANNEL_2);

    if (SAMPLE_DELAY_MS > 0) {
      delay(SAMPLE_DELAY_MS);
    }
  }


  float avgRaw0 = (float)sum0 / NUM_SAMPLES;
  float avgRaw1 = (float)sum1 / NUM_SAMPLES;
  float avgRaw2 = (float)sum2 / NUM_SAMPLES;


  float V_ads1_a0 = avgRaw0 * ADS_LSB_VOLTS;
  float V_ads1_a1 = avgRaw1 * ADS_LSB_VOLTS;  // A1 = GND
  float V_ads1_a2 = avgRaw2 * ADS_LSB_VOLTS;  // A2 = 5V reference

  float diff3_V_ref = V_ads1_a2 - V_ads1_a1;

  float diff2 = V_ads1_a0 - V_ads1_a1;

  V_set_dac =
      diff3_V_ref *
      ((float)dacSetValue / MAX_DAC_BITS);

  // Difference between expected DAC voltage
  // and actual measured DAC voltage

  float diff1 = V_set_dac - diff2;

  Serial.print("Vset expected  : ");
  Serial.print(V_set_dac, 5);
  Serial.println(" V");

  Serial.print("Vdac measured  : ");
  Serial.print(diff2, 5);
  Serial.println(" V");

  Serial.print("Vset - Vdac    : ");
  Serial.print(diff1, 5);
  Serial.println(" V");

  Serial.println();


  display.clearDisplay();

  display.setTextSize(1);

  // Vset
  display.setCursor(0, 0);
  display.print("Vset: ");
  display.print(V_set_dac, 3);
  display.println(" V");

  // Actual DAC voltage
  display.setCursor(0, 16);
  display.print("Vadc: ");
  display.print(diff2, 3);
  display.println(" V");

  // Error
  display.setCursor(0, 32);
  display.print("Error: ");
  display.print(diff1, 3);
  display.println(" V");

  // Reference
  display.setCursor(0, 48);
  display.print("Vref: ");
  display.print(diff3_V_ref, 3);
  display.println(" V");

  display.display();


  // Update every 30 seconds
  delay(500);
}
