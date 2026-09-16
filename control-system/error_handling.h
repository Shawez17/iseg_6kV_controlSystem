#pragma once

#include <Arduino.h>
#include <Adafruit_ST7789.h>

enum class ErrorCode : uint8_t {
  None = 0,
  Startup,
  Hardware,
  Command,
  Transport,
};

void clearErrorState();
bool hasActiveError();
ErrorCode activeErrorCode();
const char* activeErrorTitle();
const char* activeErrorMessage();
void raiseError(Stream& channel, ErrorCode code, const char* title, const char* message);
void updateErrorState();
void renderErrorScreen(Adafruit_ST7789& tft);