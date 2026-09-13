/**
 * config.h
 * Authors: Vijay Mocherla & Shawez Shaik
 * 
 * This file contains configuration constants and pin assignments for the HV Supply.
*/

#pragma once
#include <Arduino.h>
//W5100S pins
constexpr int WNET_RX_PIN  = 16; 
constexpr int WNET_CS_PIN  = 17;
constexpr int WNET_SCK_PIN = 18; 
constexpr int WNET_TX_PIN  = 19; 
constexpr int WNET_RST_PIN = 20; 
constexpr int WNET_INT_PIN = 21; 

// I2C pins for ADS1115 and GP8413 modules
constexpr int I2C0_SDA = 4;
constexpr int I2C0_SCL = 5;
constexpr int I2C1_SDA = 6;
constexpr int I2C1_SCL = 7;

// TFT display pins
constexpr int TFT_SCK = 10;
constexpr int TFT_TX  = 11;
constexpr int TFT_RX  = 12;
constexpr int TFT_CS  = 13;
constexpr int TFT_DC  = 14;
constexpr int TFT_RST = 15;

// Other GPIO pin assignments
constexpr int LED_GPIO = 25;
constexpr int BUTTON_GPIO = 26;
// High voltage conversion factors
constexpr float CH1_HV_FACTOR = 1200.0f;
constexpr float CH2_HV_FACTOR = 1200.0f;   
constexpr float CH1_CURRENT_FACTOR = 0.757; //milliamps per volt
constexpr float CH2_CURRENT_FACTOR = 0.757; //milliamps per volt

// Maximum voltage
constexpr float CH1_MAX_VOLTS = 6000.0f; // volts
constexpr float CH2_MAX_VOLTS = 6000.0f; // volts
constexpr float CH1_MAX_CURRENT = 0.0f;  // milliamps
constexpr float CH2_MAX_CURRENT = 0.0f;  // milliamps

// DAC and ADC configuration constants
constexpr uint16_t MAX_DAC_BITS = 32767; // 2^15 - 1 for 15-bit DAC clamped at 5V
constexpr uint16_t NUM_SAMPLES = 50;
constexpr uint16_t SAMPLE_DELAY_MS = 0;
constexpr uint16_t SETTLE_DELAY_MS = 2;
constexpr uint8_t TREND_SAMPLES = 64;

// I2C addresses
constexpr uint8_t ADDR_GP8413 = 0x58;
constexpr uint8_t ADDR_ADS1115_CH1 = 0x48;
constexpr uint8_t ADDR_ADS1115_CH2 = 0x49;

// IP address
constexpr uint8_t DEFAULT_IP[4] = {10, 10, 7, 254};
constexpr uint8_t DEFAULT_SUBNET[4] = {255, 255, 255, 0};
constexpr uint8_t DEFAULT_GATEWAY[4] = {10, 10, 7, 1};
constexpr uint16_t TCP_PORT = 5025;


