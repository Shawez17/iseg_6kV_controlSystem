#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_ADS1X15.h>
#include <DFRobot_GP8XXX.h>

constexpr int I2C_SDA_PIN = 2;
constexpr int I2C_SCL_PIN = 3;

constexpr int LED_GPIO = 25;
constexpr int TFT_CS = 13;
constexpr int TFT_DC = 14;
constexpr int TFT_RST = 15;

constexpr float HV_FACTOR_POS = 1200.0f;
constexpr float HV_FACTOR_NEG = 1200.0f;
constexpr uint16_t MAX_DAC_BITS = 32767;
constexpr uint16_t NUM_SAMPLES = 10;
constexpr uint16_t SAMPLE_DELAY_MS = 0;
constexpr uint16_t SETTLE_DELAY_MS = 2;

constexpr uint16_t DAC_CODE_0 = 32767;
constexpr uint16_t DAC_CODE_1 = 16383;
