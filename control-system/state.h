#pragma once

#include <Arduino.h>

enum class TransportMode : uint8_t {
  Usb,
  Ethernet,
};

enum class DisplayMode : uint8_t {
  Live,
  Trend,
};

struct SystemState {
  float vref_pos = 0.0f;
  float vmon_pos = 0.0f;
  float imon_pos = 0.0f;
  float vset_pos = 0.0f;
  float hv_pos = 0.0f;

  float vref_neg = 0.0f;
  float vmon_neg = 0.0f;
  float imon_neg = 0.0f;
  float vset_neg = 0.0f;
  float hv_neg = 0.0f;

  uint16_t dac_code_1 = 0;
  uint16_t dac_code_0 = 0;

  float target_vset_pos = 0.0f;
  float target_vset_neg = 0.0f;

  float trend_hv_pos[TREND_SAMPLES] = {0.0f};
  float trend_hv_neg[TREND_SAMPLES] = {0.0f};
  float trend_imon_pos[TREND_SAMPLES] = {0.0f};
  float trend_imon_neg[TREND_SAMPLES] = {0.0f};
  uint8_t trend_head = 0;
  bool trend_full = false;

  TransportMode transport_mode = TransportMode::Usb;
  DisplayMode display_mode = DisplayMode::Live;

  bool debug_mode = false;
};