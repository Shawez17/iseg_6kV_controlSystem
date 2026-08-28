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

Adafruit_ADS1115 ads1;
const uint8_t ADC_CHANNEL_0 = 0;
const uint8_t ADC_CHANNEL_1 = 1;
const uint8_t ADC_CHANNEL_2 = 2;
const float ADS_LSB_VOLTS = 0.0001875;

Adafruit_MCP4725 dac;
const uint8_t MCP_ADDR = 0x60;
const uint16_t MAX_DAC_BITS = 4095;

float V_set_dac;

const uint16_t NUM_SAMPLES = 100;
const uint16_t SAMPLE_DELAY_MS = 0;
const uint16_t SETTLE_DELAY_MS = 2;
const uint16_t PRINT_EVERY_N = 45;

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

  if (!ads1.begin(0x48)) {
    Serial.println("ADS1115 not found!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ADS1115 ERROR");
    display.display();
    while (1) {
      delay(10);
    }
  }
  ads1.setGain(GAIN_TWOTHIRDS);

  if (!dac.begin(MCP_ADDR)) {
    Serial.println("MCP4725 not found!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MCP4725 ERROR");
    display.display();
    while (1) {
      delay(10);
    }
  }

  display.clearDisplay();
  display.display();
  delay(1500);
}

void loop() {
  for (uint16_t dacSetValue = 45; dacSetValue < MAX_DAC_BITS; dacSetValue++) {

    dac.setVoltage(dacSetValue, false);
    delay(SETTLE_DELAY_MS);

    long sum0 = 0;
    long sum1 = 0;
    long sum2 = 0;
    long sum3 = 0;

    for (uint16_t i = 0; i < NUM_SAMPLES; i++) {
      sum0 += ads1.readADC_SingleEnded(ADC_CHANNEL_0);
      sum1 += ads1.readADC_SingleEnded(ADC_CHANNEL_1);
      sum2 += ads1.readADC_SingleEnded(ADC_CHANNEL_2);
      sum3 += ads1.readADC_SingleEnded(ADC_CHANNEL_3);

      if (SAMPLE_DELAY_MS > 0) {
        delay(SAMPLE_DELAY_MS);
      }
    }

    float avgRaw0 = (float)sum0 / NUM_SAMPLES;
    float avgRaw1 = (float)sum1 / NUM_SAMPLES;
    float avgRaw2 = (float)sum2 / NUM_SAMPLES;
    float avgRaw3 = (float)sum3 / NUM_SAMPLES;

    float V_ads1_a0 = avgRaw0 * ADS_LSB_VOLTS;
    float V_ads1_a1 = avgRaw1 * ADS_LSB_VOLTS;
    float V_ads1_a2 = avgRaw2 * ADS_LSB_VOLTS;
    float V_ads1_a3 = avgRaw3 * ADS_LSB_VOLTS;

    float diff3_V_ref = V_ads1_a2 - V_ads1_a1;
    float diff2 = V_ads1_a0 - V_ads1_a1;

    V_set_dac = diff3_V_ref * ((float)dacSetValue / MAX_DAC_BITS);
    float diff1 = V_set_dac - diff2; 
    float diff3 = V_ads1_a3 - V_ads1_a1; 

    if (dacSetValue % PRINT_EVERY_N == 0) {
      Serial.print("Vset expected  : ");
      Serial.print(V_set_dac, 5);
      Serial.println(" V");
      Serial.print("Vdac measured  : ");
      Serial.print(diff2, 5);
      Serial.println(" V");
      Serial.print("Vset - Vdac    : ");
      Serial.print(diff1, 5);
      Serial.println(" V");
      Serial.print("Dac bits       : ");
      Serial.println(dacSetValue);
      Serial.print("analogPot     :");
      Serial.println(diff3);
      Serial.println();

      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Vset: ");
      display.print(V_set_dac, 3);
      display.println(" V");
      display.setCursor(0, 12);
      display.print("Vadc: ");
      display.print(diff2, 3);
      display.println(" V");
      display.setCursor(0, 24);
      display.print("Error: ");
      display.print(diff1, 3);
      display.println(" V");
      display.setCursor(0, 36);
      display.print("Vref: ");
      display.print(diff3_V_ref, 3);
      display.println(" V");
      display.setCursor(0, 48);
      display.print("Bits: ");
      display.println(dacSetValue);
      display.setCursor(0,60);
      display.print("pot: ");
      display.println(diff3,3);
      display.println(" V");
      display.display();
    }
  }
}
