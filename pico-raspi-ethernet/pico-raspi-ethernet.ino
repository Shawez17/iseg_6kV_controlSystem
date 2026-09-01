#include <Arduino.h>
#include <Wire.h>
#define LED_GPIO 25
#include <DFRobot_GP8XXX.h>
#include <Adafruit_ADS1X15.h>
const uint16_t PRINT_EVERY_N = 45;
const uint16_t MAX_DAC_BITS = 32767;
const uint16_t NUM_SAMPLES = 100;
Adafruit_ADS1115 ads;
const uint16_t SAMPLE_DELAY_MS = 0;
const uint16_t SETTLE_DELAY_MS = 2;
// DAC output settings
//const uint16_t DAC_CODE_0 = 32767;
//const uint16_t DAC_CODE_1 = 16383;
//const uint16_t DAC_CODE_0 ;
//const uint16_t DAC_CODE_1 ;
//const float DAC_RANGE_VOLTS = 5.0;
// Store DAC voltages globally so loop() can use them
float dacVoltage0 = 0.0;
float dacVoltage1 = 0.0;
DFRobot_GP8XXX_IIC GP8413(
  RESOLUTION_15_BIT,
  0x58,
  &Wire1
);
void setup() {
  pinMode(LED_GPIO, OUTPUT);
  Serial.begin(115200);
  delay(1000);
  // I2C1
  Wire1.setSDA(14);
  Wire1.setSCL(15);
  Wire1.begin();
  Serial.println("Starting GP8413 DAC on Wire1...");
  while (GP8413.begin() != 0) {
    Serial.println("GP8413 communication failed.");
    delay(1000);
  }
  Serial.println("GP8413 initialized successfully.");
  // DAC range = 0–5 V
  GP8413.setDACOutRange(GP8413.eOutputRange5V);
  
   // ADS1115
  ads.setGain(GAIN_TWOTHIRDS);
  if (!ads.begin(ADS1X15_ADDRESS, &Wire1)) {
    Serial.println("Failed to initialize ADS1115.");
    while (1) {
      delay(10);
    }
  }
  Serial.println("ADS1115 initialized successfully.");
  Serial.println("Starting measurements...");
}
void loop() {
  digitalWrite(LED_GPIO, HIGH);
  delay(1000);
  digitalWrite(LED_GPIO, LOW);
  delay(1000);
  const float ADS_LSB_VOLTS = 0.0001875;
  for (uint16_t DAC_CODE_0 = 44, DAC_CODE_1 = 44;
       DAC_CODE_0 < MAX_DAC_BITS && DAC_CODE_1 < MAX_DAC_BITS;
       DAC_CODE_0++, DAC_CODE_1++) {
      
    // Set DAC outputs
      GP8413.setDACOutVoltage(DAC_CODE_0, 0);
      GP8413.setDACOutVoltage(DAC_CODE_1, 1);
    delay(SETTLE_DELAY_MS);
    long sum0_3 = 0;
    long sum1_3 = 0;
    long sum2_3 = 0;
    // Take samples
    for (uint16_t i = 0; i < NUM_SAMPLES; i++) {
      sum0_3 += ads.readADC_Differential_0_3();
      sum1_3 += ads.readADC_Differential_1_3();
      sum2_3 += ads.readADC_Differential_2_3();
      if (SAMPLE_DELAY_MS > 0) {
        delay(SAMPLE_DELAY_MS);
      
      }
      //Serial.print("summation...");
    }
   
    // Average raw ADC values
    float avgRaw0_3 =
        (float)sum0_3 / NUM_SAMPLES;
     float avgRaw1_3 =
        (float)sum1_3 / NUM_SAMPLES;
    float avgRaw2_3 =
        (float)sum2_3 / NUM_SAMPLES;
    // Convert ADC counts → volts
    float V_ads0_3 =
        avgRaw0_3 * ADS_LSB_VOLTS;
     float V_ads1_3 =
        avgRaw1_3 * ADS_LSB_VOLTS;
    float DAC_RANGE_VOLTS =
        avgRaw2_3 * ADS_LSB_VOLTS;
     // Convert DAC codes to actual voltage
    dacVoltage0 =
        ((float)DAC_CODE_0 / 32767.0) * DAC_RANGE_VOLTS;
    dacVoltage1 =
        ((float)DAC_CODE_1 / 32767.0) * DAC_RANGE_VOLTS;
    // Difference between measured voltage and DAC setting
    float diff0 = 
        -V_ads0_3 + dacVoltage0;
    float diff1 =
        -V_ads1_3 + dacVoltage1;
    //Serial.println("DAC CODE");
    //Serial.print(DAC_CODE_1);  
    delay(1000);
    // CSV format
      if (DAC_CODE_0 % PRINT_EVERY_N  == 0 && DAC_CODE_1 % PRINT_EVERY_N == 0) {
        Serial.println("DAC0_V, DAC1_V, Diff0_V, Diff1_V, ADS0_V, ADS1_V, ADS0_Raw, ADS1_Raw, DAC0_Raw, DAC1_Raw, Vref");
        Serial.print( dacVoltage0, 5);
        Serial.print(",");
        Serial.print( dacVoltage1, 5);
        Serial.print(",");
        Serial.print( diff0, 5);
        Serial.print(",");
        Serial.print( diff1, 5);
        Serial.print(",");
        Serial.print( V_ads0_3, 5);
        Serial.print(",");
        Serial.print( V_ads1_3, 5);
        Serial.print(",");
        Serial.print( avgRaw0_3, 2);
        Serial.print(",");
        Serial.print( avgRaw1_3, 2);
        Serial.print(",");
        Serial.print( DAC_CODE_0);
        Serial.print(",");
        Serial.print( DAC_CODE_1);
        Serial.print(",");
        Serial.println( DAC_RANGE_VOLTS);
          delay(1000);
      }
  }
}

