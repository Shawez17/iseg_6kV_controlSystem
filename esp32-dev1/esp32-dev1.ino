#include "Arduino.h"
#include "X9C10X.h"
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

// ================= X9C10X =================
X9C10X pot(12345);  // 100KΩ

// ================= ADS1115 =================
Adafruit_ADS1115 ads;

// ================= MCP4725 DAC =================
Adafruit_MCP4725 dac;
const uint8_t DAC_ADDR = 0x60;

// ================= PINS =================
const uint8_t INC_PIN = 25;
const uint8_t UD_PIN  = 27;
const uint8_t CS_PIN  = 33;

const uint8_t ADS_ADDR = 0x48;

// ================= VOLTAGE =================
const float VL = 0.0;
const float VH = 5.0;

const uint8_t  MAX_POS      = 99;   // pot: 100 positions (0-99)
const uint16_t MAX_DAC_BITS = 4095; // dac: 4096 codes (0-4095)

// How many DAC codes to sweep through per single pot position
const uint16_t DAC_STEPS_PER_POS = (MAX_DAC_BITS + 1) / (MAX_POS + 1); // ~41

// ADS1115 GAIN_TWOTHIRDS
const float ADS_LSB_VOLTS = 0.0001875;

const int RAMP_DELAY     = 100; // delay after moving pot to a new position
const int DAC_STEP_DELAY = 15;  // delay between each DAC sub-step

// ======================================================

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  Serial.println();
  Serial.println(__FILE__);
  Serial.print("X9C10X_LIB_VERSION: ");
  Serial.println(X9C10X_LIB_VERSION);
  Serial.println();

  // ---------------- X9C10X ----------------
  Serial.println(pot.getType());

  pot.begin(INC_PIN, UD_PIN, CS_PIN);
  pot.setPosition(0);

  // ---------------- I2C ----------------
  Wire.begin();

  // ---------------- OLED ----------------
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("OLED not found!");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("X9C10X + ADS1115 + MCP4725");
  display.println();
  display.println("Initializing...");
  display.display();

  // ---------------- ADS1115 ----------------
  if (!ads.begin(ADS_ADDR))
  {
    Serial.println("ADS1115 not found - check wiring/address");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ADS1115 ERROR!");
    display.println();
    display.println("Check wiring");
    display.display();

    while (1)
    {
      delay(10);
    }
  }

  ads.setGain(GAIN_TWOTHIRDS);

  // ---------------- MCP4725 DAC ----------------
  if (!dac.begin(DAC_ADDR))
  {
    Serial.println("MCP4725 not found - check wiring/address");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MCP4725 ERROR!");
    display.println();
    display.println("Check wiring");
    display.display();

    while (1)
    {
      delay(10);
    }
  }

  dac.setVoltage(0, false);

  Serial.println();
  Serial.println("Ramping pot (100 pos) + DAC (4096 codes, full resolution)...");
  Serial.println("Position\tDAC_Bits\tV_set_dac\tV_set_pot\tADC_A0_raw\tV_adc_pot\tADC_A1_raw\tV_adc_a1");
  Serial.println();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Ramping VW + DAC");
  display.println("Full 12-bit DAC");
  display.display();

  delay(1000);
}

// ======================================================

void loop()
{
  // ---------------- Ramp UP: pot 0 -> 99, dac 0 -> 4095 ----------------
  for (uint8_t position = 0; position <= MAX_POS; position++)
  {
    pot.setPosition(position);
    delay(RAMP_DELAY); // let wiper settle once per new pot position

    uint32_t dacStart = (uint32_t)position * DAC_STEPS_PER_POS;

    for (uint16_t sub = 0; sub < DAC_STEPS_PER_POS; sub++)
    {
      uint32_t dacBits = dacStart + sub;
      if (dacBits > MAX_DAC_BITS) dacBits = MAX_DAC_BITS;

      rampStep(position, (uint16_t)dacBits);
    }
  }

  delay(500);

  // ---------------- Ramp DOWN: pot 99 -> 0, dac 4095 -> 0 ----------------
  for (int position = MAX_POS; position >= 0; position--)
  {
    pot.setPosition(position);
    delay(RAMP_DELAY);

    int32_t dacStart = (int32_t)position * DAC_STEPS_PER_POS + (DAC_STEPS_PER_POS - 1);

    for (int sub = DAC_STEPS_PER_POS - 1; sub >= 0; sub--)
    {
      int32_t dacBits = (int32_t)position * DAC_STEPS_PER_POS + sub;
      if (dacBits > MAX_DAC_BITS) dacBits = MAX_DAC_BITS;
      if (dacBits < 0) dacBits = 0;

      rampStep(position, (uint16_t)dacBits);
    }
  }

  delay(500);
}

// ======================================================

void rampStep(uint8_t position, uint16_t dacBits)
{
  // Set DAC output at full 12-bit resolution
  dac.setVoltage(dacBits, false);

  delay(DAC_STEP_DELAY); // let DAC output + ADC settle

  // Voltage the DAC is set to output
  float V_set_dac = (VH - VL) * ((float)dacBits / MAX_DAC_BITS);

  // Voltage the pot is set to output
  float V_set_pot = VL + (VH - VL) * ((float)position / MAX_POS);

  // Read ADS1115 A0 (digi pot) and A1 (dac)
  int16_t adc_bits_a0 = ads.readADC_SingleEnded(0); // pot
  int16_t adc_bits_a1 = ads.readADC_SingleEnded(1); // dac

  // Convert ADC readings to measured voltages (SAME LSB, same ADC/gain)
  float V_adc_pot = adc_bits_a0 * ADS_LSB_VOLTS;
  float V_adc_a1 = adc_bits_a1 * ADS_LSB_VOLTS;

  // ==================================================
  // SERIAL OUTPUT
  // ==================================================

  Serial.print("Position: ");
  Serial.print(position);

  Serial.print("\tDAC_Bits: ");
  Serial.print(dacBits);

  Serial.print("\tV_set_dac: ");
  Serial.print(V_set_dac, 4);
  Serial.print(" V");

  Serial.print("\tV_set_pot: ");
  Serial.print(V_set_pot, 3);
  Serial.print(" V");

  Serial.print("\tADC_A0_raw: ");
  Serial.print(adc_bits_a0);

  Serial.print("\tV_adc_pot: ");
  Serial.print(V_adc_pot, 3);
  Serial.print(" V");

  Serial.print("\tADC_A1_raw: ");
  Serial.print(adc_bits_a1);

  Serial.print("\tV_adc_a1: ");
  Serial.print(V_adc_a1, 4);
  Serial.println(" V");

  // ==================================================
  // OLED OUTPUT
  // ==================================================

  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Pos:");
  display.print(position);
  display.print(" DACb:");
  display.println(dacBits);

  display.setCursor(0, 12);
  display.print("Vset_dac:");
  display.print(V_set_dac, 3);
  display.println("V");

  display.setCursor(0, 24);
  display.print("Vset_pot:");
  display.print(V_set_pot, 2);
  display.println("V");

  display.setCursor(0, 36);
  display.print("Vadc_pot:");

  display.print(V_adc_pot, 3);
  display.println("V");

  display.setCursor(0, 48);
  display.print("Vadc_a1:");
  display.print(V_adc_a1, 3);
  display.println("V");

  display.display();
}
