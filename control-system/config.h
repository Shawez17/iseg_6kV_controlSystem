#pragma once

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <DFRobot_GP8XXX.h>
#include <SPI.h>
#include <Wire.h>

constexpr uint32_t I2C_WATCHDOG_TIMEOUT_MS = 300;
constexpr uint32_t SERIAL_BAUD_RATE = 9600;

constexpr int WNET_RX_PIN = 16;
constexpr int WNET_CS_PIN = 17;
constexpr int WNET_SCK_PIN = 18;
constexpr int WNET_TX_PIN = 19;
constexpr int WNET_RST_PIN = 20;
constexpr int WNET_INT_PIN = 21;

constexpr int I2C0_SDA = 4;
constexpr int I2C0_SCL = 5;
constexpr int I2C1_SDA = 6;
constexpr int I2C1_SCL = 7;

constexpr int TFT_SCK = 10;
constexpr int TFT_TX = 11;
constexpr int TFT_RX = 12;
constexpr int TFT_CS = 13;
constexpr int TFT_DC = 14;
constexpr int TFT_RST = 15;

constexpr int LED_GPIO = 25;
constexpr int BUTTON_GPIO = 26;
constexpr int COMM_SEL_GPIO = 1;
constexpr bool COMM_SEL_USE_PULLUP = false;
constexpr int DISPLAY_SEL_GPIO = -1;
constexpr bool DISPLAY_SEL_USE_PULLUP = false;

constexpr uint8_t ADDR_GP8413 = 0x58;
constexpr uint8_t ADDR_ADS1115_CH1 = 0x4A;
constexpr uint8_t ADDR_ADS1115_CH2 = 0x48;

constexpr float CH1_HV_FACTOR = 1200.0f;
constexpr float CH2_HV_FACTOR = 1200.0f;
constexpr float CH1_CURRENT_FACTOR = 0.757f;
constexpr float CH2_CURRENT_FACTOR = 0.757f;
constexpr float CH1_MAX_VOLTS = 6000.0f;
constexpr float CH2_MAX_VOLTS = 6000.0f;
constexpr float CH1_MAX_CURRENT = 0.0f;
constexpr float CH2_MAX_CURRENT = 0.0f;

constexpr uint16_t MAX_DAC_BITS = 32767;
constexpr uint16_t DAC_BOOT_CODE = 655;
constexpr uint16_t NUM_SAMPLES = 50;
constexpr uint16_t SAMPLE_DELAY_MS = 0;
constexpr uint16_t SETTLE_DELAY_MS = 2;
constexpr uint8_t TREND_SAMPLES = 64;

constexpr uint8_t ETHERNET_MAC_ADDRESS[6] = {0x02, 0x60, 0x51, 0x10, 0x50, 0x01};
constexpr uint8_t DEFAULT_IP[4] = {10, 10, 7, 254};
constexpr uint8_t DEFAULT_SUBNET[4] = {255, 255, 255, 0};
constexpr uint8_t DEFAULT_GATEWAY[4] = {10, 10, 7, 1};
constexpr uint16_t TCP_PORT = 5025;

constexpr int I2C_SDA_PIN = I2C1_SDA;
constexpr int I2C_SCL_PIN = I2C1_SCL;
constexpr int TRANSPORT_SEL_PIN = COMM_SEL_GPIO;
constexpr bool TRANSPORT_SEL_USE_PULLUP = COMM_SEL_USE_PULLUP;
constexpr int DISPLAY_SEL_PIN = DISPLAY_SEL_GPIO;
constexpr bool DISPLAY_SEL_USE_PULLUP_ALIAS = DISPLAY_SEL_USE_PULLUP;
constexpr float HV_FACTOR_POS = CH1_HV_FACTOR;
constexpr float HV_FACTOR_NEG = CH2_HV_FACTOR;
constexpr float VSET_MAX_VOLTS = CH1_MAX_VOLTS;