#include "display_ui.h"

void initDisplay(Adafruit_ST7789& tft) {
  tft.init(240, 320);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(40, 100);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.println("HV Supply");
}

void renderStatus(Adafruit_ST7789& tft, const SystemState& state) {
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
}
