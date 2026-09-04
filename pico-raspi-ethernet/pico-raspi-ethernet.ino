#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Wire.h>
#define LED_GPIO 25
#include <DFRobot_GP8XXX.h>
#include <Adafruit_ADS1X15.h>
#define TFT_CS   13
#define TFT_DC   14
#define TFT_RST  15

const float HVfACTOR=1200;
const float HVfACTOR1=1200;
const uint16_t MAX_DAC_BITS = 32767;
const uint16_t NUMsAMPLES = 10;
const uint16_t SAMPLE_DELAY_MS = 0;
const uint16_t SETTLE_DELAY_MS = 2;
//const int navg = 50;
float read_delay = 20;
// readout array 
//float v_readout[4] = {0.0, 0.0, 0.0, 0.0};
// variables for voltages
float v_vmon;
float v_imon;
float v_vset0;
float hv_out;

long adc[4] = {0,0,0,0};
long adc1[4] = {0,0,0,0};

long adc_[4] = {0,0,0,0};
long adc_1[4] = {0,0,0,0};


//float v_readout1[4] = {0.0, 0.0, 0.0, 0.0};
// variables for voltages
float v_vmon1;
float v_imon1;
float v_vset01;
float hv_out1;
int debug_mode = 0; // Debug mode flag

//Instances
Adafruit_ADS1115 ads;
Adafruit_ADS1115 ads1;
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, TFT_CS, TFT_DC, TFT_RST);
DFRobot_GP8XXX_IIC GP8413(
  RESOLUTION_15_BIT,
  0x58,
  &Wire1
);

void findI2c(){
  byte error, address;
  int nDevices = 0;

  delay(1000);
  // --- Scan Wire1 (I2C1) ---
  Serial.println("Scanning Wire (I2C1) ...");
  nDevices = 0;
  for (address = 0x01; address < 0x7f; address++) {
    Wire1.beginTransmission(address);
    error = Wire1.endTransmission();
    if (error == 0) {
      Serial.printf("I2C1: device found at address 0x%02X\n", address);
      nDevices++;
      break;
    } else if (error != 2) {
      Serial.printf("I2C1: error %u at address 0x%02X\n", error, address);
      }
    
    if (nDevices == 0) {
      Serial.println("I2C1: No devices found");
      }
    }
  }


void setup() {
  pinMode(LED_GPIO, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {
    delay(10);   // waits until Serial Monitor is actually connected
  }
  delay(500);    // small buffer after connection
  Serial.println("Starting...");
  
  // SPI interface
  SPI1.setSCK(10);
  SPI1.setTX(11);
  SPI1.setRX(12);
  SPI1.setCS(13);
  tft.init(240, 320);   // adjust to your panel's resolution
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(40, 100);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.println("HV Supply");
  delay(1000);

  // I2C
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();
  findI2c();
}

void set_voltages(){
    //DAC INPUT LOGIC

    // Set DAC outputs
    GP8413.setDACOutVoltage(DAC_CODE_0, 0);
    GP8413.setDACOutVoltage(DAC_CODE_1, 1);
    delay(SETTLE_DELAY_MS);
}

void read_voltages(){
    
    // Take samples
    for (uint16_t i = 0; i < NUMsAMPLES; i++) {
//      adc[0] += ads.readADC_Differential_0_3();
//      adc1 += ads.readADC_Differential_1_3();
//      adc2 += ads.readADC_Differential_2_3();

        adc[0] += ads.readADC_SingleEnded(0);
        adc[1] += ads.readADC_SingleEnded(1);
        adc[2] += ads.readADC_SingleEnded(2);
        adc[3] += ads.readADC_SingleEnded(3);
        
        adc1[0] += ads1.readADC_SingleEnded(0);
        adc1[1] += ads1.readADC_SingleEnded(1);
        adc1[2] += ads1.readADC_SingleEnded(2);
        adc1[3] += ads1.readADC_SingleEnded(3);
        
      if (SAMPLE_DELAY_MS > 0) {
        delay(SAMPLE_DELAY_MS);
      }
      //Serial.print("summation...");
      }
   
    // Average ADC bits
    float adc_[0] =
        (float)adc[0] / NUMsAMPLES;
    float adc_[1] =
        (float)adc[1] / NUMsAMPLES;
    float adc_[2] =
        (float)adc[2] / NUMsAMPLES;
    float adc_[3] =
        (float)adc[3] / NUMsAMPLES;

    float adc_1[0] =
        (float)adc1[0] / NUMsAMPLES;
    float adc_1[1] =
        (float)adc1[1] / NUMsAMPLES;
    float adc_1[2] =
        (float)adc1[2] / NUMsAMPLES;
    float adc_1[3] =
        (float)adc1[3] / NUMsAMPLES;   

    v_vref = ads.computeVolts(adc_[0]);
    v_vmon = ads.computeVolts(adc_[1]);
    v_imon = ads.computeVolts(adc_[2]);
    v_vset0 = ads.computeVolts(adc_[3]);

    v_vref1 = ads.computeVolts(adc_1[0]);
    v_vmon1 = ads.computeVolts(adc_1[1]);
    v_imon1 = ads.computeVolts(adc_1[2]);
    v_vset01 = ads.computeVolts(adc_1[3]);

    hv_out = HVfACTOR * v_vset0;
    hv_out1 = HVfACTOR1 * v_vset01;
    // Difference between measured voltage and DAC setting
//    float diff0 = 
//        -V_ads0_3 + dacVoltage0;
//    float diff1 =
//        -V_ads1_3 + dacVoltage1;

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


void check_serial_command(){
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil("\r");
        command.trim();
        Serial.println(command);
        if (command == "DEBUG"){
          Serial.println("Entering debug mode...\n");
          debug_mode = 1;
        }
        else if (command == "STOP"){
          Serial.println("Exiting debug mode...\n");
          debug_mode = 0;
        }
        else{
            Serial.println("ERROR\n");
        }
    }
    if (debug_mode){
      flush_to_serial_monitor();
    }
}

void ledBlink(){
  digitalWrite(LED_GPIO, HIGH);
  delay(1000);
  digitalWrite(LED_GPIO, LOW);
  delay(1000); 
}



void loop() {
  ledBlink();
  
  read_voltages();
  flush_to_serial_monitor();


}
