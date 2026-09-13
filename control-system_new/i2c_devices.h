#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "state.h"
#include "libraries/Adafruit_ADS1X15.h"
#include "libraries/DFRobot_GP8XXX.h"

struct adcReadout {
    float AN0;
    float AN1;
    float AN2;
    float AN3;
};


DFRobot_GP8413 DAC(ADDR_GP8413);
Adafruit_ADS1115 ADC_CH1(ADDR_ADS1115_CH1);
Adafruit_ADS1115 ADC_CH2(ADDR_ADS1115_CH2);

void i2cScan();
void setupDAC();
void setDACValues(float voltage, uint8_t channel);
void setupADC();
adcReadout readADCReadings();
