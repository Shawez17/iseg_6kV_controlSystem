#include "display_ui.h"

namespace {

const char* transportModeText(TransportMode mode) {
  return (mode == TransportMode::Ethernet) ? "ETH" : "USB";
}

const char* displayModeText(DisplayMode mode) {
  return (mode == DisplayMode::Trend) ? "TREND" : "LIVE";
}

uint8_t trendSampleCount(const SystemState& state) {
  return state.trend_full ? TREND_SAMPLES : state.trend_head;
}

uint8_t trendIndexFromOldest(const SystemState& state, uint8_t offset) {
  const uint8_t count = trendSampleCount(state);
  if (count == 0) {
    return 0;
  }

  return static_cast<uint8_t>((state.trend_full ? state.trend_head : 0) + offset) % TREND_SAMPLES;
}

void drawTrendGraph(Adafruit_ST7789& tft,
                    int16_t x,
                    int16_t y,
                    int16_t w,
                    int16_t h,
                    const SystemState& state,
                    const float* a,
                    const float* b) {
  const uint8_t count = trendSampleCount(state);
  if (count < 2) {
    tft.drawRect(x, y, w, h, ST77XX_WHITE);
    tft.setCursor(x + 4, y + 4);
    tft.println("Waiting for samples");
    return;
  }

  float minValue = a[trendIndexFromOldest(state, 0)];
  float maxValue = minValue;

  for (uint8_t i = 0; i < count; ++i) {
    const uint8_t index = trendIndexFromOldest(state, i);
    minValue = min(minValue, min(a[index], b[index]));
    maxValue = max(maxValue, max(a[index], b[index]));
  }

  if (fabs(maxValue - minValue) < 0.001f) {
    maxValue += 1.0f;
    minValue -= 1.0f;
  }

  tft.drawRect(x, y, w, h, ST77XX_WHITE);

  const int16_t plotWidth = w - 6;
  const int16_t plotHeight = h - 18;
  const int16_t plotX = x + 3;
  const int16_t plotY = y + 12;

  auto mapValueToY = [&](float value) {
    const float normalized = (value - minValue) / (maxValue - minValue);
    return static_cast<int16_t>(plotY + plotHeight - (normalized * plotHeight));
  };

  for (uint8_t i = 1; i < count; ++i) {
    const uint8_t prevIndex = trendIndexFromOldest(state, i - 1);
    const uint8_t currentIndex = trendIndexFromOldest(state, i);
    const int16_t x0 = plotX + ((i - 1) * plotWidth) / (count - 1);
    const int16_t x1 = plotX + (i * plotWidth) / (count - 1);

    tft.drawLine(x0, mapValueToY(a[prevIndex]), x1, mapValueToY(a[currentIndex]), ST77XX_CYAN);
    tft.drawLine(x0, mapValueToY(b[prevIndex]), x1, mapValueToY(b[currentIndex]), ST77XX_YELLOW);
  }
}

}  // namespace

void initDisplay(Adafruit_ST7789& tft) {
  tft.init(240, 320);
  tft.setRotation(-1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(30, 100);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.println("There is only one god given ground");
}


void renderDisplay(Adafruit_ST7789& tft, const SystemState& state) {
  if (state.display_mode == DisplayMode::Trend) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.println("Trend View");
    drawTrendGraph(tft, 0, 24, 320, 100, state, state.trend_hv_pos, state.trend_hv_neg);
    drawTrendGraph(tft, 0, 130, 320, 100, state, state.trend_imon_pos, state.trend_imon_neg);
    return;
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);

  tft.print("+Vset: ");
  tft.println(state.vset_pos, 3);
  tft.print("+Vmon: ");
  tft.println(state.vmon_pos, 3);
  tft.print("+HV: ");
  tft.println(state.hv_pos, 1);

  tft.print("-Vset: ");
  tft.println(state.vset_neg, 3);
  tft.print("-Vmon: ");
  tft.println(state.vmon_neg, 3);
  tft.print("-HV: ");
  tft.println(state.hv_neg, 1);

  tft.print("Debug: ");
  tft.println(state.debug_mode ? "ON" : "OFF");
  tft.print("Transport: ");
  tft.println(transportModeText(state.transport_mode));
  tft.print("Mode: ");
  tft.println(displayModeText(state.display_mode));
}
