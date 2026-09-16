#include "error_handling.h"

#include "config.h"
#include "display_ui.h"

namespace {

constexpr uint32_t kErrorScreenMs = 3000;

struct ErrorState {
  bool active = false;
  ErrorCode code = ErrorCode::None;
  String title;
  String message;
  uint32_t startedMs = 0;
};

ErrorState errorState;

void drawCenteredText(Adafruit_ST7789& tft, const String& text, int16_t y, uint8_t size, uint16_t color) {
  tft.setTextSize(size);
  tft.setTextColor(color);
  const int16_t textWidth = static_cast<int16_t>(text.length() * 6 * size);
  const int16_t x = (screenWidth - textWidth) / 2;
  tft.setCursor(max<int16_t>(0, x), y);
  tft.print(text);
}

}  // namespace

void clearErrorState() {
  errorState = ErrorState{};
}

bool hasActiveError() {
  return errorState.active;
}

ErrorCode activeErrorCode() {
  return errorState.code;
}

const char* activeErrorTitle() {
  return errorState.title.c_str();
}

const char* activeErrorMessage() {
  return errorState.message.c_str();
}

void raiseError(Stream& channel, ErrorCode code, const char* title, const char* message) {
  if (!errorState.active) {
    errorState.active = true;
    errorState.code = code;
    errorState.title = title;
    errorState.message = message;
    errorState.startedMs = millis();
  }

  channel.print("ERROR[");
  channel.print(static_cast<int>(code));
  channel.print("]: ");
  channel.print(title);
  channel.print(" - ");
  channel.println(message);
}

void updateErrorState() {
  if (!errorState.active) {
    return;
  }

  if (millis() - errorState.startedMs >= kErrorScreenMs) {
    clearErrorState();
  }
}

void renderErrorScreen(Adafruit_ST7789& tft) {
  if (!errorState.active) {
    return;
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(18, 18, screenWidth - 36, screenHeight - 36, 16, ST77XX_RED);
  tft.fillRoundRect(28, 28, screenWidth - 56, screenHeight - 56, 12, ST77XX_BLACK);

  drawCenteredText(tft, "ERROR", 40, 3, ST77XX_RED);
  drawCenteredText(tft, String("CODE ") + static_cast<int>(errorState.code), 76, 2, ST77XX_YELLOW);
  drawCenteredText(tft, errorState.title, 112, 2, ST77XX_WHITE);
  drawCenteredText(tft, errorState.message, 146, 2, ST77XX_WHITE);
  drawCenteredText(tft, "AUTO CLEAR ENABLED", 190, 1, ST77XX_CYAN);
}