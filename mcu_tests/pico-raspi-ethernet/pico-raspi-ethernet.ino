#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_ST7789.h>
#include <Wire.h>
#include <DFRobot_GP8XXX.h>
#include <Adafruit_ADS1X15.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define LED_GPIO 25
#define TFT_CS   13
#define TFT_DC   14
#define TFT_RST  15

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, TFT_CS, TFT_DC, TFT_RST);


uint16_t DAC_CODE_0 = 32767;
uint16_t DAC_CODE_1 = 0;
const float HVfACTOR=1200;
const float HVfACTOR1=1200;
const uint16_t MAX_DAC_BITS = 32767;
const uint16_t NUMsAMPLES = 10;
const uint16_t SAMPLE_DELAY_MS = 0;
//const int navg = 50;
float read_delay = 20;
// readout array 
//float v_readout[4] = {0.0, 0.0, 0.0, 0.0};
// variables for voltages
float v_vmon;
float v_imon;
float v_vset0;
float v_vref;
float hv_out;

long adc[4] = {0,0,0,0};
long adc1[4] = {0,0,0,0};

long adc_[4] = {0,0,0,0};
long adc_1[4] = {0,0,0,0};
const uint16_t NUM_SAMPLES = 100;
const uint16_t SETTLE_DELAY_MS = 2;
const float ADS_LSB_VOLTS = 0.0001875;




//float v_readout1[4] = {0.0, 0.0, 0.0, 0.0};
// variables for voltages
float v_vmon1;
float v_imon1;
float v_vset01;
float v_vref1;
float hv_out1;
int debug_mode = 0; // Debug mode flag

//Instances
Adafruit_ADS1115 ads;
Adafruit_ADS1115 ads1;

DFRobot_GP8XXX_IIC GP8413(
  RESOLUTION_15_BIT,
  0x58,
  &Wire1
);


void initTFT() {
  SPI1.setSCK(10);
  SPI1.setTX(11);
  SPI1.setRX(12);
  SPI1.setCS(13);

  tft.init(240, 320);   // adjust to your panel's resolution
  tft.setRotation(-1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(40, 100);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.println("There is only one god given ground");
}

void rotateText() {
  for (uint8_t i = 0; i < 4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.setCursor(0, 30);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(1);
    tft.println("There is Only One");

    tft.setTextColor(ST77XX_BLUE);
    tft.setTextSize(4);
    tft.print("God Given Ground");

    while (!Serial.available());
    Serial.read();
    Serial.read();
    Serial.read();

    tft.setRotation(tft.getRotation() + 1);
  }
}

void findI2c(){
  byte error, address;
  int nDevices = 0;

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

void set_voltages(){
    //DAC INPUT LOGIC

    // Set DAC outputs
    GP8413.setDACOutVoltage(DAC_CODE_0, 0);
    GP8413.setDACOutVoltage(DAC_CODE_1, 1);
    delay(SETTLE_DELAY_MS);
}

void read_voltages(){
    
      int sum0_23=0;

  for (uint16_t i = 0; i < NUM_SAMPLES; i++) {

 //     sum0_1 += ads.readADC_Differential_0_3();//
 //     sum0_2 += ads.readADC_Differential_1_3();//
      sum0_23 += ads.readADC_Differential_2_3();// 

 //     sum1_1 += ads2.readADC_Differential_0_3();//Vset_adc
      //sum1_2 += ads1.readADC_Differential_1_3();//Vref
 //     sum1_3 += ads2.readADC_Differential_2_3();//Vgnd

      if (SAMPLE_DELAY_MS > 0) {
        delay(SAMPLE_DELAY_MS);
      }
    }

//    float avgRaw0_1 = (float)sum0_1 / NUM_SAMPLES;
  //  float avgRaw0_2 = (float)sum0_2 / NUM_SAMPLES;
  float avgRaw0_23 = (float)sum0_23 / NUM_SAMPLES;
  //  float avgRaw1_1 = (float)sum1_1 / NUM_SAMPLES;
    //float avgRaw1_2 = (float)sum1_2 / NUM_SAMPLES;
  //  float avgRaw1_3 = (float)sum1_3 / NUM_SAMPLES;



    //float V_ads_a1 = avgRaw0_1 * ADS_LSB_VOLTS;
    //float V_ads_a2 = avgRaw0_2 * ADS_LSB_VOLTS;
  float V_ads_23 = avgRaw0_23 * ADS_LSB_VOLTS;

    //float V_ads2_a1 = avgRaw1_1 * ADS_LSB_VOLTS;
    //float V_ads2_a2 = avgRaw1_2 * ADS_LSB_VOLTS;
    //float V_ads2_a3 = avgRaw1_3 * ADS_LSB_VOLTS;
  delay(2000); // Pause for 2 seco
  Serial.print(V_ads_23,5); 
  Serial.print(" , "); 
  Serial.println(avgRaw0_23,2);   
 
}

void flush_to_serial_monitor(){
    Serial.print("V_vref: ");
    Serial.print(v_vref, 5);
    Serial.print("V_vmon(+ve): ");
    Serial.print(v_vmon, 5);
    Serial.print(" V, V_imon(+ve): ");
    Serial.print(v_imon, 5);
    Serial.print(" V, V_vset0(+ve): ");
    Serial.print(v_vset0, 5);
    Serial.print(" V, HV_Out(+ve): ");
    Serial.println(hv_out, 2);

    Serial.print("V_vref: ");
    Serial.print(v_vref1, 5);
    Serial.print("V_vmon(-ve): ");
    Serial.print(v_vmon1, 5);
    Serial.print(" V, V_imon(-ve): ");
    Serial.print(v_imon1, 5);
    Serial.print(" V, V_vset0(-ve): ");
    Serial.print(v_vset01, 5);
    Serial.print(" V, HV_Out(-ve): ");
    Serial.println(hv_out1, 2);


    //Serial.print(" V, I_Out: ");
    //Serial.print(i_out, 6);
    //Serial.println(" mA");
}




void ledBlink(){
  digitalWrite(LED_GPIO, HIGH);
  delay(1000);
}



void setup() {
  pinMode(LED_GPIO, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {
    delay(10);   // waits until Serial Monitor is actually connected
  }
  delay(500);    // small buffer after connection
  
 
  // I2C
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();
  initTFT();
  ledBlink(); 
  findI2c();
    Serial.println("Trying ADS1115...");

    if (ads.begin(0x48, &Wire1)) {
        Serial.println("ADS1115 FOUND at 0x48");
    } else {
        Serial.println("ADS1115 FAILED");
    }

 ads.setGain(GAIN_TWOTHIRDS);
  //ads1.setGain(GAIN_TWOTHIRDS);



  Serial.println("setup is done");
}



void loop() {
  ledBlink(); 
  //findI2c();
  read_voltages();
  flush_to_serial_monitor();
  rotateText();
}


