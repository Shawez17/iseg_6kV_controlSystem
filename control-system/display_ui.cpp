#include "display_ui.h"

#include "display_images.h"
#include "error_handling.h"
#include "ethernet_communication.h"
#include "tifrh_logo.h"

namespace {

constexpr int16_t kTextLineHeight = lineHeight;
constexpr int16_t kLiveValueX = valueX;
constexpr int16_t kLiveValueWidth = valueWidth;
constexpr uint16_t kDarkNavy = 0x0010;

constexpr int16_t kTrendGraphWidth = trendGraphWidth;
constexpr int16_t kTrendGraphHeight = trendGraphHeight;

struct LiveDisplayCache {
  bool initialized = false;
  int16_t live_text_start_x = -1;
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

void setDisplayInverted(Adafruit_ST7789& tft, bool inverted) {
  static bool isInverted = false;
  if (isInverted == inverted) {
    return;
  }

  tft.invertDisplay(inverted);
  isInverted = inverted;
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

uint16_t textWidth(Adafruit_ST7789& tft, const char* text) {
  int16_t textX = 0;
  int16_t textY = 0;
  uint16_t textWidth = 0;
  uint16_t textHeight = 0;

  tft.getTextBounds(text, 0, 0, &textX, &textY, &textWidth, &textHeight);
  return textWidth;
}

void drawLeftAlignedTextLine(Adafruit_ST7789& tft, int16_t x, int16_t y, const char* text) {
  tft.setCursor(x, y);
  tft.print(text);
}

void formatLiveValueText(char* lineBuffer,
                         size_t lineBufferSize,
                         const char* label,
                         float value,
                         uint8_t precision,
                         bool padToFourDigits) {
  const bool isNegative = value < 0.0f;

  if (padToFourDigits) {
    char hvBuffer[5];
    const long magnitude = lroundf(fabs(value));
    snprintf(hvBuffer, sizeof(hvBuffer), "%04ld", magnitude);
    snprintf(lineBuffer, lineBufferSize, "%s%c%s", label, isNegative ? '-' : '+', hvBuffer);
    return;
  }

  char valueBuffer[16];
  dtostrf(fabs(value), 0, precision, valueBuffer);
  snprintf(lineBuffer, lineBufferSize, "%s%c%s", label, isNegative ? '-' : '+', valueBuffer);
}

void drawWarningSign(Adafruit_ST7789& tft) {
  const int16_t x = (screenWidth - 170) / 2;
  const int16_t y = (screenHeight - 143) / 2;
  tft.drawRGBBitmap(x, y, dangerHighVoltageData, 170, 143);
}

void drawTifrLogo(Adafruit_ST7789& tft) {
  const int16_t x = 0;
  const int16_t y = (screenHeight - kTifrhJpegLogoHeight) / 2;
  tft.fillRect(0, 0, screenWidth, screenHeight, ST77XX_WHITE);
  tft.drawRGBBitmap(x, y, tifrhJpegLogoData, kTifrhJpegLogoWidth, kTifrhJpegLogoHeight);
}

void showWelcomeLogo(Adafruit_ST7789& tft) {
  setDisplayInverted(tft, true);
  tft.fillScreen(ST77XX_WHITE);
  drawTifrLogo(tft);
}

void showWarningScreen(Adafruit_ST7789& tft) {
  setDisplayInverted(tft, true);
  tft.fillScreen(ST77XX_WHITE);
  drawWarningSign(tft);

  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(120, 194);
  tft.println("WARNING");

  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(95, 214);
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
                      int16_t x,
                      int16_t y,
                      const char* text,
                      float value,
                      float& lastValue,
                      bool initialized,
                      bool forceRedraw) {
  if (!forceRedraw && initialized && nearlyEqual(lastValue, value)) {
    return;
  }

  tft.fillRect(kLiveValueX, y, kLiveValueWidth, kTextLineHeight, ST77XX_BLACK);
  drawLeftAlignedTextLine(tft, x, y, text);

  lastValue = value;
}

void renderLiveView(Adafruit_ST7789& tft, const SystemState& state, LiveDisplayCache& cache) {
  static const char* debugOn = "ON";
  static const char* debugOff = "OFF";

  static bool ethernetIpBannerActive = false;
  static uint32_t ethernetIpBannerUntilMs = 0;
  static IPAddress ethernetIpBannerValue(0, 0, 0, 0);

  if (ethernetInterfaceReady() && !ethernetIpBannerActive) {
    ethernetIpBannerActive = true;
    ethernetIpBannerUntilMs = millis() + 3000;
    ethernetIpBannerValue = ethernetLocalIP();
  }

  if (ethernetIpBannerActive) {
    tft.fillRect(0, 0, screenWidth, 26, ST77XX_BLACK);
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(1);
    tft.setCursor(8, 4);
    tft.print("IP:");
    tft.print(ethernetIpBannerValue);

    if (millis() >= ethernetIpBannerUntilMs) {
      ethernetIpBannerActive = false;
    }
  }

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(textSize);

  char vsetPosLine[32];
  char vmonPosLine[32];
  char hvPosLine[32];
  char vsetNegLine[32];
  char vmonNegLine[32];
  char hvNegLine[32];
  char debugLine[24];
  char transportLine[24];
  char modeLine[24];

  formatLiveValueText(vsetPosLine, sizeof(vsetPosLine), "+Vset : ", state.vset_pos, voltagePrecision, false);
  formatLiveValueText(vmonPosLine, sizeof(vmonPosLine), "+Vmon : ", state.vmon_pos, voltagePrecision, false);
  formatLiveValueText(hvPosLine, sizeof(hvPosLine), "+HV   : ", state.hv_pos, hvPrecision, true);
  formatLiveValueText(vsetNegLine, sizeof(vsetNegLine), "-Vset : ", state.vset_neg, voltagePrecision, false);
  formatLiveValueText(vmonNegLine, sizeof(vmonNegLine), "-Vmon : ", state.vmon_neg, voltagePrecision, false);
  formatLiveValueText(hvNegLine, sizeof(hvNegLine), "-HV   : ", state.hv_neg, hvPrecision, true);

  const char* debugText = state.debug_mode ? debugOn : debugOff;
  snprintf(debugLine, sizeof(debugLine), "Debug : %s", debugText);

  const char* transportText = transportModeText(state.transport_mode);
  snprintf(transportLine, sizeof(transportLine), "Trans.: %s", transportText);

  const char* modeText = displayModeText(state.display_mode);
  snprintf(modeLine, sizeof(modeLine), "Mode  : %s", modeText);

  uint16_t maxLineWidth = textWidth(tft, vsetPosLine);
  maxLineWidth = max(maxLineWidth, textWidth(tft, vmonPosLine));
  maxLineWidth = max(maxLineWidth, textWidth(tft, hvPosLine));
  maxLineWidth = max(maxLineWidth, textWidth(tft, vsetNegLine));
  maxLineWidth = max(maxLineWidth, textWidth(tft, vmonNegLine));
  maxLineWidth = max(maxLineWidth, textWidth(tft, hvNegLine));
  maxLineWidth = max(maxLineWidth, textWidth(tft, debugLine));
  maxLineWidth = max(maxLineWidth, textWidth(tft, transportLine));
  maxLineWidth = max(maxLineWidth, textWidth(tft, modeLine));

  const int16_t textStartX = max<int16_t>(0, static_cast<int16_t>((screenWidth - maxLineWidth) / 2));
  const bool forceRedraw = !cache.initialized || cache.live_text_start_x != textStartX;

  drawLiveValueLine(tft, textStartX, liveTopY + 0 * lineHeight, vsetPosLine, state.vset_pos, cache.vset_pos, cache.initialized, forceRedraw);
  drawLiveValueLine(tft, textStartX, liveTopY + 1 * lineHeight, vmonPosLine, state.vmon_pos, cache.vmon_pos, cache.initialized, forceRedraw);
  drawLiveValueLine(tft, textStartX, liveTopY + 2 * lineHeight, hvPosLine, state.hv_pos, cache.hv_pos, cache.initialized, forceRedraw);
  drawLiveValueLine(tft, textStartX, liveTopY + 3 * lineHeight, vsetNegLine, state.vset_neg, cache.vset_neg, cache.initialized, forceRedraw);
  drawLiveValueLine(tft, textStartX, liveTopY + 4 * lineHeight, vmonNegLine, state.vmon_neg, cache.vmon_neg, cache.initialized, forceRedraw);
  drawLiveValueLine(tft, textStartX, liveTopY + 5 * lineHeight, hvNegLine, state.hv_neg, cache.hv_neg, cache.initialized, forceRedraw);

  if (forceRedraw || cache.debug_mode != state.debug_mode) {
    tft.fillRect(kLiveValueX, debugRowY, kLiveValueWidth, kTextLineHeight, ST77XX_BLACK);
    drawLeftAlignedTextLine(tft, textStartX, debugRowY, debugLine);
    cache.debug_mode = state.debug_mode;
  }

  if (forceRedraw || cache.transport_mode != state.transport_mode) {
    tft.fillRect(kLiveValueX, transportRowY, kLiveValueWidth, kTextLineHeight, ST77XX_BLACK);
    drawLeftAlignedTextLine(tft, textStartX, transportRowY, transportLine);
    cache.transport_mode = state.transport_mode;
  }

  if (forceRedraw || cache.display_mode != state.display_mode) {
    tft.fillRect(kLiveValueX, modeRowY, kLiveValueWidth, kTextLineHeight, ST77XX_BLACK);
    drawLeftAlignedTextLine(tft, textStartX, modeRowY, modeLine);
    cache.display_mode = state.display_mode;
  }

  cache.live_text_start_x = textStartX;
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
  tft.fillScreen(ST77XX_WHITE);
  tft.setCursor(45, 108);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(3);
  // tft.println("INITIALIZING...");
  // delay(3000);
}

void renderDisplay(Adafruit_ST7789& tft, const SystemState& state) {
  static LiveDisplayCache cache;
  static DisplayMode lastDisplayMode = DisplayMode::Live;
  static bool initialFrame = true;
  static bool errorWasActive = false;
  static uint32_t welcomeStartMs = 0;
  static uint32_t warningStartMs = 0;
  static enum class StartupPhase {
    Welcome,
    Warning,
    Live
  } 
  startupPhase = StartupPhase::Welcome;

  const bool errorActive = hasActiveError();
  if (errorActive) {
    setDisplayInverted(tft, false);
    renderErrorScreen(tft);
    errorWasActive = true;
    return;
  }

  if (errorWasActive) {
    // Force the UI back to the live screen once the error is cleared.
    startupPhase = StartupPhase::Live;
    welcomeStartMs = 0;
    warningStartMs = 0;
    initialFrame = false;
    lastDisplayMode = DisplayMode::Live;
    cache.initialized = false;

    setDisplayInverted(tft, false);
    tft.fillScreen(ST77XX_BLACK);
    renderLiveView(tft, state, cache);

    errorWasActive = false;
    return;
  }

  const uint32_t now = millis();

  if (startupPhase == StartupPhase::Welcome) {
    if (welcomeStartMs == 0) {
      welcomeStartMs = now;
      showWelcomeLogo(tft);
      delay(welcomeScreenMs);
    }


    startupPhase = StartupPhase::Warning;
    warningStartMs = 0;
    return;
  }

  if (startupPhase == StartupPhase::Warning) {
    if (warningStartMs == 0) {
      warningStartMs = now;
      showWarningScreen(tft);
      delay(warningScreenMs);
    }

    // if (now - warningStartMs < warningScreenMs) {
    //   showWarningScreen(tft);
    //   return;
    // }

    startupPhase = StartupPhase::Live;
    initialFrame = true;
    lastDisplayMode = DisplayMode::Live;
  }

  if (initialFrame || lastDisplayMode != state.display_mode) {
    setDisplayInverted(tft, false);
    tft.fillScreen(ST77XX_BLACK);
    lastDisplayMode = state.display_mode;
    initialFrame = false;
    cache.initialized = false;

    if (state.display_mode == DisplayMode::Trend) {
      renderTrendView(tft, state);
    } 
    else {
      renderLiveView(tft, state, cache);
    }
    return;
  }

  if (state.display_mode == DisplayMode::Trend) {
    setDisplayInverted(tft, false);
    tft.fillRect(0, trendGraphTop1, screenWidth, trendGraphHeight, ST77XX_BLACK);
    tft.fillRect(0, trendGraphTop2, screenWidth, trendGraphHeight, ST77XX_BLACK);
    renderTrendView(tft, state);
    return;
  }

  renderLiveView(tft, state, cache);
}
