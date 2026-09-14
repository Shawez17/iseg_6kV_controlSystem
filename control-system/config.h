#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_ADS1X15.h>
#include <DFRobot_GP8XXX.h>

constexpr uint32_t I2C_WATCHDOG_TIMEOUT_MS = 300; // must be > worst-case normal I2C op time
constexpr int I2C_SDA_PIN = 2;
constexpr int I2C_SCL_PIN = 3;

constexpr int LED_GPIO = 25;
constexpr int TFT_CS = 13;
constexpr int TFT_DC = 14;
constexpr int TFT_RST = 15;
// GPIO selectors (placeholders). Fill these with actual GPIO numbers later.
// - Set to -1 to disable selector and keep defaults (USB active, Live display).
constexpr int TRANSPORT_SEL_PIN = -1; // USB/Ethernet selector: HIGH = Ethernet
constexpr int DISPLAY_SEL_PIN = -1;   // Live/Trend selector: HIGH = Trend

// Selector pull-mode flags: choose true for INPUT_PULLUP, false for INPUT_PULLDOWN.
// Modify these if your board wiring uses pull-up instead of pull-down.
constexpr bool TRANSPORT_SEL_USE_PULLUP = false;
constexpr bool DISPLAY_SEL_USE_PULLUP = false;

constexpr float HV_FACTOR_POS = 1200.0f;
constexpr float HV_FACTOR_NEG = 1200.0f;
constexpr float VSET_MAX_VOLTS = 6000.0f;
constexpr uint16_t MAX_DAC_BITS = 32767;
constexpr uint16_t NUM_SAMPLES = 50;
constexpr uint16_t SAMPLE_DELAY_MS = 0;
constexpr uint16_t SETTLE_DELAY_MS = 2;
constexpr uint8_t TREND_SAMPLES = 32;

// WIZnet / Ethernet hardware pins (placeholders). Fill when wiring is known.
// constexpr int WIZNET_CS_PIN = -1;   // Chip-select for WIZnet SPI (if required)
// constexpr int WIZNET_RST_PIN = -1;  // Reset pin for WIZnet (if required)
// constexpr int WIZNET_INT_PIN = -1;  // Interrupt pin from WIZnet (optional)
//<<<<<<< HEAD

//=======
// 
//>>>>>>> 7e5636d (Initialtes 11-09-26_Wire1)

// Network defaults — update these to match your network. If not set,
// the network transport will remain a no-op unless compiled with
// -DUSE_WIZNET and these values are updated.
constexpr uint8_t DEFAULT_IP[4] = {192, 168, 1, 50};
constexpr uint8_t DEFAULT_SUBNET[4] = {255, 255, 255, 0};
constexpr uint8_t DEFAULT_GATEWAY[4] = {192, 168, 1, 1};
constexpr uint16_t TCP_PORT = 5025;
