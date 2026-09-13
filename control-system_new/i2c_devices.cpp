/**
 * i2c_devices.cpp
 * Authors: Vijay Mocherla & Shawez Shaik
 * 
 * This file contains the implementation of I2C device setup and other methods.
 * 
*/
#include "i2c_devices.h"


void i2cScan() {
    byte error, address;
    int nDevices = 0;
    Serial.println("Scanning I2C bus...");
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
        Serial.print("I2C device found at address 0x");
        if (address < 16) Serial.print("0");
        Serial.print(address, HEX);
        Serial.println(" !");
        nDevices++;
        }
        else if (error == 4) {
        Serial.print("Unknown error at address 0x");
        if (address < 16) Serial.print("0");
        Serial.println(address, HEX);
        }    
    }
    
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("Done\n");
}

// DAC functions
void setupDAC(){
    if(DAC.begin()!=0) {
        if (!Serial) {
            Serial.println(" Communication with the device failed. Please check if the DAC module is connected correctly.");
            // if (systemConfig.debugMode) {
            //     Serial.println("Debug mode is enabled.");
            //     i2cScan();
            // }
            // delay(100);
        }
    }
    else {
        // Notes: 
        //      1. The DAC output range is set to 0-5V with code range of 0-32767
        //      2. On power loss, DAC will revert to 0V DAC code.
        DAC.setDACOutRange(DFRobot_GP8XXX::eOutputRange5V);
        DAC.setDACOutVoltage(0, 2); // Set both channels to 0V
        DAC.store(); // Store the voltage settings in the DAC
        Serial.println("DAC initialized successfully.");
    }
    delay(1000);
}

void setDACValues(float voltage, uint8_t channel) {
    if (voltage < 0.0f || voltage > 5.0f) {
        Serial.println("Error: Voltage out of range (0-5V).");
        return;
    }
    uint16_t dacValue = static_cast<uint16_t>((voltage / 5.0) * MAX_DAC_BITS);
    DAC.setDACOutVoltage(dacValue, channel);
}


void setupADC() {
    // ADS GAIN set to 2/3x gain +/- 6.144V  1 bit = 0.1875mV 
    ADC_CH1.setGain(GAIN_TWOTHIRDS);  
    ADC_CH2.setGain(GAIN_TWOTHIRDS);  
    
    if (!ADC_CH1.begin()) {
        Serial.println("Failed to initialize ADC_CH1. Please check the connection.");
    } 
    else {
        Serial.println("ADC_CH1 initialized successfully.");
    }

    if (!ADC_CH2.begin()) {
        Serial.println("Failed to initialize ADC_CH2. Please check the connection.");
    } 
    else {
        Serial.println("ADC_CH2 initialized successfully.");
    }
}


adcReadout readADCVoltages(Adafruit_ADS1115 ADC) {
    adcReadout readout;
    float voltages = {0.0f, 0.0f, 0.0f, 0.0f};
    for (int i = 0; i < NUM_SAMPLES; i++) {
        voltages[0] += ADC.readADC_SingleEnded(0);
        voltages[1] += ADC.readADC_SingleEnded(1);
        voltages[2] += ADC.readADC_SingleEnded(2);
        voltages[3] += ADC.readADC_SingleEnded(3);
    }
    readout.AN0 = voltages[0]/NUM_SAMPLES;
    readout.AN1 = voltages[1]/NUM_SAMPLES;
    readout.AN2 = voltages[2]/NUM_SAMPLES;
    readout.AN3 = voltages[3]/NUM_SAMPLES;
    return readout;
}