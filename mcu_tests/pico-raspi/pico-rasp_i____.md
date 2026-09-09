#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MCP4725.h>

#define LED_GPIO 25
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;

/*const uint8_t ADC_CHANNEL_0 = 0;
const uint8_t ADC_CHANNEL_1 = 1;
const uint8_t ADC_CHANNEL_2 = 2;
const uint8_t ADC_CHANNEL_3 = 3;
*/
const float ADS_LSB_VOLTS = 0.0001875;

Adafruit_MCP4725 dac1;
Adafruit_MCP4725 dac2;

const uint8_t MCP_ADDR = 0x60;
const uint16_t MAX_dac1_BITS = 4095;

float V_set_dac1;
float V_set_dac2;

const uint16_t NUM_SAMPLES = 100;
const uint16_t SAMPLE_DELAY_MS = 0;
const uint16_t SETTLE_DELAY_MS = 2;
const uint16_t PRINT_EVERY_N = 45;

void setup() {
  
  pinMode(LED_GPIO, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin();

  Wire1.setSDA(18);
  Wire1.setSCL(19);
  Wire1.begin();

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
/*
  if (!ads2.begin(0x49)) {
    Serial.println("ADS1115 not found!**** left side");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ADS1115 ERROR**** left side pair");
    display.display();
    while (1) {
      delay(10);
    }
  }
*/
  ads1.setGain(GAIN_TWOTHIRDS);
//  ads2.setGain(GAIN_TWOTHIRDS);

  if (!dac1.begin(MCP_ADDR, &Wire)) {
    Serial.println("MCP4725 (0x48) not found!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MCP4725(0x48) ERROR");
    display.display();
    while (1) {
      delay(10);
    }
  }
/*
  if (!dac2.begin(MCP_ADDR, &Wire1)) {
    Serial.println("MCP4725(0x49) not found!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MCP4725(0x49) ERROR");
    display.display();
    while (1) {
      delay(10);
    }
  }
*/

  display.clearDisplay();
  display.display();
  delay(500);
}

void loop() {
  
  digitalWrite(LED_GPIO, HIGH);
  //delay(1000);
  //digitalWrite(LED_GPIO, LOW);
  //delay(1000);


  uint16_t dac1SetValue=2000;
//  uint16_t dac2SetValue=3000;

/*  for (uint16_t dac1SetValue = 45, dac2SetValue = 45;
       dac1SetValue < MAX_dac1_BITS && dac2SetValue < MAX_dac1_BITS;
       dac1SetValue++, dac2SetValue++) {
*/
  for (uint16_t dac1SetValue = 45;
        dac1SetValue < MAX_dac1_BITS;
        dac1SetValue++) {

       
    dac1.setVoltage(dac1SetValue, false);
  //  dac2.setVoltage(dac2SetValue, false);
    delay(SETTLE_DELAY_MS);

    long sum0_0 = 0;
    long sum0_1 = 0;
    long sum0_3 = 0;
/*
    long sum1_0 = 0;
    long sum1_1 = 0;
    long sum1_3 = 0;
*/
    for (uint16_t i = 0; i < NUM_SAMPLES; i++) {

      sum0_0 += ads1.readADC_Differential_0_3();//Vset_adc
      sum0_1 += ads1.readADC_Differential_1_3();//Vref
      sum0_3 += ads1.readADC_SingleEnded(3);
/*
      sum1_0 += ads2.readADC_Differential_0_3();//Vset_adc
      sum1_1 += ads2.readADC_Differential_1_3();//Vref
      sum1_3 += ads2.readADC_SingleEnded(3);
*/
      if (SAMPLE_DELAY_MS > 0) {
        delay(SAMPLE_DELAY_MS);
      }
    }

    float avgRaw0_0 = (float)sum0_0 / NUM_SAMPLES;
    float avgRaw0_1 = (float)sum0_1 / NUM_SAMPLES;
    float avgRaw0_3 = (float)sum0_3 / NUM_SAMPLES;
/*
    float avgRaw1_0 = (float)sum1_0 / NUM_SAMPLES;
    float avgRaw1_1 = (float)sum1_1 / NUM_SAMPLES;
    float avgRaw1_3 = (float)sum1_3 / NUM_SAMPLES;
*/

    float V_ads1_a0 = avgRaw0_0 * ADS_LSB_VOLTS;
    float V_ads1_a1 = avgRaw0_1 * ADS_LSB_VOLTS;
    float V_ads1_a3 = avgRaw0_3 * ADS_LSB_VOLTS;
/*
    float V_ads2_a0 = avgRaw1_0 * ADS_LSB_VOLTS;
    float V_ads2_a1 = avgRaw1_1 * ADS_LSB_VOLTS;
    float V_ads2_a3 = avgRaw1_3 * ADS_LSB_VOLTS;
*/
   /* float diff3_1_V_ref_1 = V_ads1_a1 - V_ads1_a0;
    float diff3_2_V_ref_2 = V_ads2_a1 - V_ads2_a0;
    float diff2_1 = V_ads1_a0 - V_ads1_a0;
    float diff2_2 = V_ads2_a0 - V_ads2_a0;
*/
    V_set_dac1 = V_ads1_a1 * ((float)dac1SetValue / MAX_dac1_BITS);
//    V_set_dac2 = V_ads2_a1 * ((float)dac2SetValue / MAX_dac1_BITS);
    float diff1_1 = V_set_dac1 - V_ads1_a0;//error1
//    float diff1_2 = V_set_dac2 - V_ads2_a0;//error2
  //  float diff3_1 = V_ads1_a3 - V_ads1_a0;
  //  float diff3_2 = V_ads2_a3 - V_ads2_a0;


 //
 //if ((dac1SetValue % PRINT_EVERY_N == 0) && (dac2SetValue % PRINT_EVERY_N == 0)) {
if ((dac1SetValue % PRINT_EVERY_N == 0)) {

      Serial.print("Vsetexpected1,");
      Serial.print(V_set_dac1, 5);
      Serial.println(" V");

/*      Serial.print("Vsetexpected2,");
      Serial.print(V_set_dac2, 5);
      Serial.println(" V");
*/
      Serial.print("Vdac1measured,");
      Serial.print(V_ads1_a0, 5);
      Serial.println(" V");
/*
      Serial.print("Vdac2measured,");
      Serial.print(V_ads2_a0, 5);
      Serial.println(" V");
*/
      Serial.print("Vset1-Vdac1,");
      Serial.print(diff1_1, 5);
      Serial.println(" V");
/*
      Serial.print("Vset2-Vdac2,");
      Serial.print(diff1_2, 5);
      Serial.println(" V");
*/    
      Serial.print("Vref1,");
      Serial.print(V_ads1_a1, 5);
      Serial.println(" V");
/*
      Serial.print("Vref2,");
      Serial.print(V_ads2_a1, 5);
      Serial.println(" V");     
*/
      Serial.print("Gref1,");
      Serial.print(V_ads1_a3, 5);
      Serial.println(" V");
/*
      Serial.print("Gref2,");
      Serial.print(V_ads2_a3, 5);
      Serial.println(" V");    
*/
 //     Serial.print("dac2bits,");
 //     Serial.println(dac2SetValue);
      Serial.print("dac1bits,");
      Serial.println(dac1SetValue);

      display.clearDisplay();
      display.setTextSize(1);

      display.setCursor(0, 0);
      display.print("Vset1,");
      display.print(V_set_dac1, 3);
      display.println(" V");
/*
      display.setCursor(0,11);
      display.print("Vset2,");
      display.print(V_set_dac2, 3);
      display.println(" V");
*/
/*
      display.setCursor(0, 22);
      display.print("Vadc1,");
      display.print(V_ads2_a0, 3);
      display.println(" V");

      display.setCursor(0, 33);
      display.print("Vadc2,");
      display.print(V_ads1_a0, 3);
      display.println(" V");
*/

      display.setCursor(0, 22);
      display.print("Error1,");
      display.print(diff1_1, 3);
      display.println(" V");
/*
      display.setCursor(0, 33);
      display.print("Error2,");
      display.print(diff1_2, 3);
      display.println(" V");
*/
      display.display();
    //}
  }

  }

}
