#include "display.h"

#include <LiquidCrystal.h>

#include "config.h"

namespace {
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
bool soil_page = true;

void printLine(uint8_t row, const char* text) {
  char buf[17];
  snprintf(buf, sizeof buf, "%-16s", text);  // pad to 16 chars so old text is overwritten
  lcd.setCursor(0, row);
  lcd.print(buf);
}
}  // namespace

namespace display {

void begin() {
  lcd.begin(16, 2);
  lcd.clear();
}

void message(const char* line1, const char* line2) {
  printLine(0, line1);
  printLine(1, line2);
}

void showReadings(const soil::Reading& s, const EnvReading& e) {
  char l1[24], l2[24];
  if (soil_page) {
    if (s.valid) {
      snprintf(l1, sizeof l1, "N%u P%u K%u", s.nitrogen_mg_kg, s.phosphorus_mg_kg, s.potassium_mg_kg);
      snprintf(l2, sizeof l2, "pH%.1f M%.0f%% %.0fC", s.ph, s.moisture_pct, s.temperature_c);
    } else {
      snprintf(l1, sizeof l1, "Soil probe");
      snprintf(l2, sizeof l2, "no response");
    }
  } else {
    if (e.bme_ok) snprintf(l1, sizeof l1, "Air %.1fC %.0f%%", e.air_temp_c, e.air_humidity_pct);
    else snprintf(l1, sizeof l1, "BME280 missing");
    snprintf(l2, sizeof l2, "W:%s %s", e.water_level, e.raining ? "RAIN" : "dry");
  }
  message(l1, l2);
  soil_page = !soil_page;
}

}  // namespace display
