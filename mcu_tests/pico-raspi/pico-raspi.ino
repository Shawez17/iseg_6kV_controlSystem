#include "Arduino.h" 
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
//#include <Adafruit_MCP4725.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const uint16_t NUM_SAMPLES = 100;
const uint16_t SAMPLE_DELAY_MS = 0;
const uint16_t SETTLE_DELAY_MS = 2;
const float ADS_LSB_VOLTS = 0.0001875;


#define LED_GPIO 25
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define ADC_ADDRESS 0x48 ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1115 ads1;  /* Use this for the 16-bit version */


void setup(){
  Serial.begin(115200);
  Serial.println("Binsoir elliot!");
  pinMode(LED_GPIO,OUTPUT);

  Wire.setSCL(21);
  Wire.setSDA(20);
  Wire.begin();

  delay(500);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  display.drawPixel(10, 10, SSD1306_WHITE);

  ads.setGain(GAIN_TWOTHIRDS);
  //ads1.setGain(GAIN_TWOTHIRDS);

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    //while (1);
  }
  //if (!ads1.begin()) {
  //  Serial.println("Failed to initialize ADS.");
    //while (1);
  }





void loop(void){
  digitalWrite(LED_GPIO, HIGH);
  delay(1000);
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
  display.display();
  delay(2000); // Pause for 2 seco
  Serial.print(V_ads_23,5); 
  Serial.print(" , "); 
  Serial.println(avgRaw0_23,2);   
  
  display.clearDisplay();


  delay(5000);


}


