#pragma once

#include <Arduino.h>

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

  bool debug_mode = false;
};