#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MCP4725.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= ADS1115 (x2) =================
Adafruit_ADS1115 ads1;   // first ADS1115
Adafruit_ADS1115 ads2;   // second ADS1115

const uint8_t ADS1_ADDR = 0x48;
const uint8_t ADS2_ADDR = 0x49;
const uint8_t ADC_CHANNEL = 0;      // A0 on each ADS1115
const float ADS_LSB_VOLTS = 0.0001875; // GAIN_TWOTHIRDS -> ±6.144V range

// ================= MCP4725 DAC =================
Adafruit_MCP4725 dac;
const uint8_t MCP_ADDR = 0x60;
const uint16_t MAX_DAC_BITS = 4095;
const float DAC_VREF = 5.0;         // adjust if your DAC's Vcc/reference isn't 5V

uint16_t dacSetValue;
float V_set_dac;

const uint16_t NUM_SAMPLES = 1000;
const uint16_t SAMPLE_DELAY_MS = 0; // set >0 if you want a delay between each of the 1000 samples

// ======================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ---------------- I2C0 ----------------
  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin();

  // ---------------- OLED ----------------
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found!");
    while (1) delay(10);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Init...");
  display.display();

  // ---------------- ADS1115 #1 (0x48) ----------------
  if (!ads1.begin(ADS1_ADDR)) {
    Serial.println("ADS1115 #1 (0x48) not found - check wiring/address");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ADS1 (0x48) ERROR!");
    display.display();
    while (1) delay(10);
  }
  ads1.setGain(GAIN_TWOTHIRDS);

  // ---------------- ADS1115 #2 (0x49) ----------------
  if (!ads2.begin(ADS2_ADDR)) {
    Serial.println("ADS1115 #2 (0x49) not found - check wiring/address");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ADS2 (0x49) ERROR!");
    display.display();
    while (1) delay(10);
  }
  ads2.setGain(GAIN_TWOTHIRDS);

  // ---------------- MCP4725 DAC ----------------
  if (!dac.begin(MCP_ADDR)) {
    Serial.println("MCP4725 not found - check wiring/address");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MCP4725 ERROR!");
    display.display();
    while (1) delay(10);
  }

  // ---------------- Pick a random DAC value ONCE, hold it constant ----------------
  //randomSeed(micros());
  //dacSetValue = random(0, MAX_DAC_BITS + 1);
  dacSetValue = 2000;
  dac.setVoltage(dacSetValue, false);
  V_set_dac = DAC_VREF * ((float)dacSetValue / MAX_DAC_BITS);

  Serial.println("=== DAC value fixed for this run ===");
  Serial.print("DAC code: ");
  Serial.print(dacSetValue);
  Serial.print("   V_set_dac: ");
  Serial.print(V_set_dac, 5);
  Serial.println(" V");
  Serial.println();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("DAC fixed at:");
  display.print(V_set_dac, 5);
  display.println(" V");
  display.display();
  delay(1500);
}

// ======================================================

void loop() {
  long sum1 = 0;
  long sum2 = 0;

  for (uint16_t i = 0; i < NUM_SAMPLES; i++) {
    sum1 += ads1.readADC_SingleEnded(ADC_CHANNEL);
    sum2 += ads2.readADC_SingleEnded(ADC_CHANNEL);
    if (SAMPLE_DELAY_MS > 0) delay(SAMPLE_DELAY_MS);
  }

  float avgRaw1 = (float)sum1 / NUM_SAMPLES;
  float avgRaw2 = (float)sum2 / NUM_SAMPLES;

  float V_adc1 = avgRaw1 * ADS_LSB_VOLTS;
  float V_adc2 = avgRaw2 * ADS_LSB_VOLTS;

  float diff1 = V_adc1 - V_set_dac;
  float diff2 = V_adc2 - V_set_dac;

  // ---------------- Serial ----------------
  Serial.print("ADS1(0x48) A0 avg raw: ");
  Serial.print(avgRaw1, 2);
  Serial.print("\tV: ");
  Serial.print(V_adc1, 5);
  Serial.print(" V\tDiff_vs_DAC: ");
  Serial.print(diff1, 5);
  Serial.println(" V");

  Serial.print("ADS2(0x49) A0 avg raw: ");
  Serial.print(avgRaw2, 2);
  Serial.print("\tV: ");
  Serial.print(V_adc2, 5);
  Serial.print(" V\tDiff_vs_DAC: ");
  Serial.print(diff2, 5);
  Serial.println(" V");

  Serial.print("V_set_dac: ");
  Serial.print(V_set_dac, 4);
  Serial.println(" V");
  Serial.println();

  // ---------------- OLED ----------------
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("DAC:");
  display.print(V_set_dac, 3);
  display.println("V");

  display.setCursor(0, 12);
  display.print("A0_48:");
  display.print(V_adc1, 3);
  display.println("V");

  display.setCursor(0, 24);
  display.print("A0_49:");
  display.print(V_adc2, 3);
  display.println("V");

  display.setCursor(0, 36);
  display.print("D1:");
  display.print(diff1, 3);
  display.println("V");

  display.setCursor(0, 48);
  display.print("D2:");
  display.print(diff2, 3);
  display.println("V");

  display.display();

  delay(500);
}
