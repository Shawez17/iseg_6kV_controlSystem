#include "display_ui.h"

#include "display_images.h"
#include "error_handling.h"

namespace {

constexpr int16_t kTextLineHeight = lineHeight;
constexpr int16_t kLiveValueX = valueX;
constexpr int16_t kLiveValueWidth = valueWidth;
constexpr uint16_t kDarkNavy = 0x0010;

constexpr int16_t kTrendGraphWidth = trendGraphWidth;
constexpr int16_t kTrendGraphHeight = trendGraphHeight;

struct LiveDisplayCache {
  bool initialized = false;
  float vset_pos = 0.0f;
  float vmon_pos = 0.0f;
  float hv_pos = 0.0f;
  float vset_neg = 0.0f;
  float vmon_neg = 0.0f;
  float hv_neg = 0.0f;
  bool debug_mode = false;
  TransportMode transport_mode = TransportMode::Usb;
  DisplayMode display_mode = DisplayMode::Live;
};

const char* transportModeText(TransportMode mode) {
  return (mode == TransportMode::Ethernet) ? "ETH" : "USB";
}

const char* displayModeText(DisplayMode mode) {
  return (mode == DisplayMode::Trend) ? "TREND" : "LIVE";
}

uint8_t trendSampleCount(const SystemState& state) {
  return state.trend_full ? trendSamples : state.trend_head;
}

uint8_t trendIndexFromOldest(const SystemState& state, uint8_t offset) {
  const uint8_t count = trendSampleCount(state);
  if (count == 0) {
    return 0;
  }

  return static_cast<uint8_t>((state.trend_full ? state.trend_head : 0) + offset) % trendSamples;
}

bool nearlyEqual(float lhs, float rhs, float epsilon = valueEpsilon) {
  return fabs(lhs - rhs) <= epsilon;
}

void drawWarningSign(Adafruit_ST7789& tft) {
  const int16_t x = (screenWidth - 170) / 2;
  const int16_t y = (screenHeight - 143) / 2;
  tft.drawRGBBitmap(x, y, dangerHighVoltageData, 170, 143);
}

void drawTifrLogo(Adafruit_ST7789& tft) {
  const int16_t x = 0;
  const int16_t y = (screenHeight - 68) / 2;
  tft.fillRect(0, 0, screenWidth, screenHeight, ST77XX_WHITE);
  tft.drawRGBBitmap(x, y, tifrhLogoData, 320, 68);
}

void showWelcomeLogo(Adafruit_ST7789& tft) {
  tft.fillScreen(ST77XX_WHITE);
  drawTifrLogo(tft);
}

void showWarningScreen(Adafruit_ST7789& tft) {
  tft.fillScreen(ST77XX_WHITE);
  drawWarningSign(tft);

  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(102, 194);
  tft.println("WARNING");

  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(72, 214);
  tft.println("HIGH VOLTAGE");
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
    tft.fillRect(x, y, w, h, ST77XX_BLACK);
    tft.drawRect(x, y, w, h, ST77XX_WHITE);
    tft.setCursor(x + trendPlotOffset, y + trendPlotOffset);
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

  tft.fillRect(x, y, w, h, ST77XX_BLACK);
  tft.drawRect(x, y, w, h, ST77XX_WHITE);

  const int16_t plotWidth = w - trendPlotPadding;
  const int16_t plotHeight = h - lineHeight;
  const int16_t plotX = x + trendPlotOffset;
  const int16_t plotY = y + trendLabelOffset;

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

void drawLiveValueLine(Adafruit_ST7789& tft,
                      int16_t y,
                      const char* label,
                      float value,
                      uint8_t precision,
                      float& lastValue,
                      bool initialized) {
  if (initialized && nearlyEqual(lastValue, value)) {
    return;
  }

  tft.fillRect(kLiveValueX, y, kLiveValueWidth, kTextLineHeight, ST77XX_BLACK);
  tft.setCursor(kLiveValueX, y);
  tft.print(label);
  tft.print(value, precision);
  lastValue = value;
}

void renderLiveView(Adafruit_ST7789& tft, const SystemState& state, LiveDisplayCache& cache) {
  static const char* debugOn = "ON";
  static const char* debugOff = "OFF";

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(textSize);

  drawLiveValueLine(tft, liveTopY + 0 * lineHeight, "+Vset: ", state.vset_pos, voltagePrecision, cache.vset_pos, cache.initialized);
  drawLiveValueLine(tft, liveTopY + 1 * lineHeight, "+Vmon: ", state.vmon_pos, voltagePrecision, cache.vmon_pos, cache.initialized);
  drawLiveValueLine(tft, liveTopY + 2 * lineHeight, "+HV: ", state.hv_pos, hvPrecision, cache.hv_pos, cache.initialized);
  drawLiveValueLine(tft, liveTopY + 3 * lineHeight, "-Vset: ", state.vset_neg, voltagePrecision, cache.vset_neg, cache.initialized);
  drawLiveValueLine(tft, liveTopY + 4 * lineHeight, "-Vmon: ", state.vmon_neg, voltagePrecision, cache.vmon_neg, cache.initialized);
  drawLiveValueLine(tft, liveTopY + 5 * lineHeight, "-HV: ", state.hv_neg, hvPrecision, cache.hv_neg, cache.initialized);

  const char* debugText = state.debug_mode ? debugOn : debugOff;
  if (!cache.initialized || cache.debug_mode != state.debug_mode) {
    tft.fillRect(kLiveValueX, debugRowY, kLiveValueWidth, kTextLineHeight, ST77XX_BLACK);
    tft.setCursor(kLiveValueX, debugRowY);
    tft.print("Debug: ");
    tft.print(debugText);
    cache.debug_mode = state.debug_mode;
  }

  const char* transportText = transportModeText(state.transport_mode);
  if (!cache.initialized || cache.transport_mode != state.transport_mode) {
    tft.fillRect(kLiveValueX, transportRowY, kLiveValueWidth, kTextLineHeight, ST77XX_BLACK);
    tft.setCursor(kLiveValueX, transportRowY);
    tft.print("Transport: ");
    tft.print(transportText);
    cache.transport_mode = state.transport_mode;
  }

  const char* modeText = displayModeText(state.display_mode);
  if (!cache.initialized || cache.display_mode != state.display_mode) {
    tft.fillRect(kLiveValueX, modeRowY, kLiveValueWidth, kTextLineHeight, ST77XX_BLACK);
    tft.setCursor(kLiveValueX, modeRowY);
    tft.print("Mode: ");
    tft.print(modeText);
    cache.display_mode = state.display_mode;
  }

  cache.initialized = true;
}

void renderTrendView(Adafruit_ST7789& tft, const SystemState& state) {
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(textSize);
  tft.setCursor(trendHeaderY, trendHeaderY);
  tft.print("Trend View");
  drawTrendGraph(tft, 0, trendGraphTop1, kTrendGraphWidth, kTrendGraphHeight, state, state.trend_hv_pos, state.trend_hv_neg);
  drawTrendGraph(tft, 0, trendGraphTop2, kTrendGraphWidth, kTrendGraphHeight, state, state.trend_imon_pos, state.trend_imon_neg);
}

}  // namespace

void initDisplay(Adafruit_ST7789& tft) {
  tft.init(screenHeight, screenWidth);
  tft.setRotation(-1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(84, 108);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.println("INITIALIZING");
}

void renderDisplay(Adafruit_ST7789& tft, const SystemState& state) {
  static LiveDisplayCache cache;
  static DisplayMode lastDisplayMode = DisplayMode::Live;
  static bool initialFrame = true;
  static uint32_t welcomeStartMs = 0;
  static uint32_t warningStartMs = 0;
  static enum class StartupPhase {
    Welcome,
    Warning,
    Live
  } startupPhase = StartupPhase::Welcome;

  if (hasActiveError()) {
    renderErrorScreen(tft);
    return;
  }

  const uint32_t now = millis();

  if (startupPhase == StartupPhase::Welcome) {
    if (welcomeStartMs == 0) {
      welcomeStartMs = now;
    }

    if (now - welcomeStartMs < welcomeScreenMs) {
      showWelcomeLogo(tft);
      return;
    }

    startupPhase = StartupPhase::Warning;
    warningStartMs = now;
    showWarningScreen(tft);
    return;
  }

  if (startupPhase == StartupPhase::Warning) {
    if (warningStartMs == 0) {
      warningStartMs = now;
    }

    if (now - warningStartMs < warningScreenMs) {
      showWarningScreen(tft);
      return;
    }

    startupPhase = StartupPhase::Live;
    initialFrame = true;
    lastDisplayMode = DisplayMode::Live;
  }

  if (initialFrame || lastDisplayMode != state.display_mode) {
    tft.fillScreen(ST77XX_BLACK);
    lastDisplayMode = state.display_mode;
    initialFrame = false;
    cache.initialized = false;

    if (state.display_mode == DisplayMode::Trend) {
      renderTrendView(tft, state);
    } else {
      renderLiveView(tft, state, cache);
    }
    return;
  }

  if (state.display_mode == DisplayMode::Trend) {
    tft.fillRect(0, trendGraphTop1, screenWidth, trendGraphHeight, ST77XX_BLACK);
    tft.fillRect(0, trendGraphTop2, screenWidth, trendGraphHeight, ST77XX_BLACK);
    renderTrendView(tft, state);
    return;
  }

  renderLiveView(tft, state, cache);
}
