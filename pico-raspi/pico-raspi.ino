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
X9C10X pot(12345);  // 100K ohm

// ================= ADS1115 =================
Adafruit_ADS1115 ads;

// ================= MCP4725 DAC =================
Adafruit_MCP4725 dac;
const uint8_t MCP_ADDR = 0x60;
const uint8_t ADS_ADDR = 0x48;

// ================= PICO PINS =================
// I2C0 on the Pico. GP4/GP5 are the default I2C0 pins on the
// earlephilhower Arduino-Pico core, so this needs no remapping,
// but we set them explicitly to be safe/portable.
const uint8_t I2C_SDA_PIN = 4;   // was ESP32 SDA (default GPIO21)
const uint8_t I2C_SCL_PIN = 5;   // was ESP32 SCL (default GPIO22)

// X9C10X digital pot control lines.
// Picked free GPIOs that don't collide with I2C0 (GP4/GP5) or the
// USB-Serial UART0 pins (GP0/GP1), which the Pico core uses for Serial.
const uint8_t INC_PIN = 2;   // was ESP32 GPIO25
const uint8_t UD_PIN  = 3;   // was ESP32 GPIO27
const uint8_t CS_PIN  = 6;   // was ESP32 GPIO33

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
  // RP2040 needs the SDA/SCL pins set BEFORE Wire.begin(); ESP32 used
  // the fixed default Wire pins (21/22), so this call is new.
  Wire.setSDA(I2C_SDA_PIN);
  Wire.setSCL(I2C_SCL_PIN);
  Wire.begin();

  // ---------------- OLED ----------------
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("OLED not found - check wiring/address");
    while (1) { delay(10); }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("X9C10X + ADS1115 + MCP4725 (Pico)");
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

    while (1) { delay(10); }
  }

  ads.setGain(GAIN_TWOTHIRDS);

  // ---------------- MCP4725 DAC ----------------
  if (!dac.begin(MCP_ADDR))
  {
    Serial.println("MCP4725 not found - check wiring/address");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MCP4725 ERROR!");
    display.println();
    display.println("Check wiring");
    display.display();

    while (1) { delay(10); }
  }

  dac.setVoltage(0, false);

  Serial.println();
  Serial.println("Ramping pot (100 pos) + DAC (4096 codes, full resolution)...");
  Serial.println("Position\tDAC_Bits\tV_set_dac\tV_set_pot\tADC_A2_raw\tV_adc_pot\tADC_A3_raw\tV_adc_dac\tADC_A0_raw\tV_adc_Apot");
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

  // Read ADS1115 A2 (digi pot), A3 (dac), A0 (analog pot)
  int16_t adc_bits_a2_pot  = ads.readADC_SingleEnded(2); // digi pot
  int16_t adc_bits_a3_dac  = ads.readADC_SingleEnded(3); // dac
  int16_t adc_bits_a0_Apot = ads.readADC_SingleEnded(0); // analog pot

  // Convert ADC readings to measured voltages (SAME LSB, same ADC/gain)
  float V_adc_pot  = adc_bits_a2_pot  * ADS_LSB_VOLTS;
  float V_adc_dac  = adc_bits_a3_dac  * ADS_LSB_VOLTS;
  float V_adc_Apot = adc_bits_a0_Apot * ADS_LSB_VOLTS;

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

  Serial.print("\tADC_A2_raw: ");
  Serial.print(adc_bits_a2_pot);

  Serial.print("\tV_adc_pot: ");
  Serial.print(V_adc_pot, 3);
  Serial.print(" V");

  Serial.print("\tADC_A3_raw: ");
  Serial.print(adc_bits_a3_dac);

  Serial.print("\tV_adc_dac: ");
  Serial.print(V_adc_dac, 4);
  Serial.print(" V");

  Serial.print("\tADC_A0_raw: ");
  Serial.print(adc_bits_a0_Apot);

  Serial.print("\tV_adc_Apot: ");
  Serial.print(V_adc_Apot, 4);
  Serial.println(" V");

  // ==================================================
  // OLED OUTPUT
  // ==================================================

  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Pos:");
  display.print(position);
  display.print(" DACbits:");
  display.println(dacBits);

  display.setCursor(0, 12);
  display.print("Vset_dac:");
  display.print(V_set_dac, 3);
  display.println("V");

  display.setCursor(0, 24);
  display.print("Vset_pot:");
  display.print(V_set_pot, 3);
  display.println("V");

  display.setCursor(0, 36);
  display.print("Vadc_pot:");
  display.print(V_adc_pot, 3);
  display.println("V");

  display.setCursor(0, 48);
  display.print("Vadc_dac:");
  display.print(V_adc_dac, 3);
  display.println("V");

  display.display();
}
