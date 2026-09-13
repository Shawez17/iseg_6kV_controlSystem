#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <DFRobot_GP8XXX.h>
//W5100S pins
constexpr int WIZNET_RX_PIN  = 16; 
constexpr int WIZNET_CS_PIN  = 17;
constexpr int WIZNET_SCK_PIN = 18; 
constexpr int WIZNET_TX_PIN  = 19; 
constexpr int WIZNET_RST_PIN = 20; 
constexpr int WIZNET_INT_PIN = 21; 
// I2C pins for ADS1115 and GP8413 modules
constexpr int I2C0_SDA = 4;
constexpr int I2C0_SCL = 5;
constexpr int I2C1_SDA = 2;
constexpr int I2C1_SCL = 3;

// TFT display pins
constexpr int TFT_SCK = 10;
constexpr int TFT_TX  = 11;
constexpr int TFT_RX  = 12;
constexpr int TFT_CS  = 13;
constexpr int TFT_DC  = 14;
constexpr int TFT_RST = 15;

// // Other GPIO pin assignments
constexpr int LED_GPIO = 25;
constexpr int TRANSPORT_SEL_PIN = 0; // Pin for selecting transport mode (Ethernet/USB)
constexpr bool TRANSPORT_SEL_USE_PULLUP = true; // Use internal pull-up resistor for transport select pin
constexpr bool DISPLAY_SEL_USE_PULLUP = true; // Use internal pull-up resistor for display select 
constexpr int DISPLAY_SEL_PIN = 1; // Pin for selecting display mode (Trend/Live)

// // Calibration and operational constants
constexpr float HV_FACTOR_POS = 1200.0f;
constexpr float HV_FACTOR_NEG = 1200.0f;
constexpr float VSET_MAX_VOLTS = 6000.0f;
constexpr uint16_t MAX_DAC_BITS = 16383; // 2^14 - 1 for 15-bit DAC clamped at 5V
constexpr uint16_t NUM_SAMPLES = 2;
constexpr uint16_t SAMPLE_DELAY_MS = 0;
constexpr uint16_t SETTLE_DELAY_MS = 2;
constexpr uint8_t TREND_SAMPLES = 64;

// Network defaults — update these to match your network. If not set,
// the network transport will remain a no-op unless compiled with
// -DUSE_WIZNET and these values are updated.
constexpr uint8_t DEFAULT_IP[4] = {10, 10, 7, 254};
constexpr uint8_t DEFAULT_SUBNET[4] = {255, 255, 255, 0};
constexpr uint8_t DEFAULT_GATEWAY[4] = {10, 10, 7, 1};
constexpr uint16_t TCP_PORT = 5025;
