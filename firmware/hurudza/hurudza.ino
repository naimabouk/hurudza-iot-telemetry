// Hurudza - multi-sensor IoT node for precision agriculture (ESP32).
//
// Data paths:
//   sensors -> 16x2 LCD                 (in-field readout, no network needed)
//   sensors -> local web page / JSON    (http://<node-ip>/ and /api/readings)
//   sensors -> HTTPS POST every 30 s    (cloud database + dashboard)
//
// The loop is cooperative and non-blocking: every task runs on its own millis() schedule,
// so a missing soil probe or lost Wi-Fi never stops the other data paths.

#include <WiFi.h>

#include "config.h"
#include "display.h"
#include "sensors.h"
#include "soil_probe.h"
#include "telemetry.h"
#include "web_ui.h"

SoilProbe soilProbe(Serial2, SOIL_SLAVE_ID);
soil::Reading soilReading;
EnvReading envReading;

uint32_t lastSample = 0;
uint32_t lastUpload = 0;
bool webStarted = false;

void logReadings() {
  if (soilReading.valid) {
    Serial.printf("[soil] N=%u P=%u K=%u mg/kg  pH=%.1f  M=%.1f%%  T=%.1fC  EC=%u uS/cm\n",
                  soilReading.nitrogen_mg_kg, soilReading.phosphorus_mg_kg,
                  soilReading.potassium_mg_kg, soilReading.ph, soilReading.moisture_pct,
                  soilReading.temperature_c, soilReading.ec_us_cm);
  } else {
    Serial.printf("[soil] read failed: %s (errors=%lu)\n",
                  modbus::statusName(soilProbe.lastStatus()),
                  static_cast<unsigned long>(soilProbe.errorCount()));
  }
  Serial.printf("[env ] T=%.2fC RH=%.2f%% P=%.2fhPa  air=%d  rain=%d  water=%s(%d)  light=%s(%d)\n",
                envReading.air_temp_c, envReading.air_humidity_pct, envReading.pressure_hpa,
                envReading.air_quality_raw, envReading.raining, envReading.water_level,
                envReading.water_raw, envReading.light_level, envReading.light_raw);
}

void setup() {
  Serial.begin(115200);
  display::begin();
  display::message("Hurudza", "initializing...");

  if (!sensors::begin()) Serial.println("[warn] BME280 not found on 0x76/0x77");
  soilProbe.begin();

  telemetry::beginWifi();
  display::message("Connecting WiFi", "");
}

void loop() {
  const uint32_t now = millis();

  telemetry::maintainWifi();
  if (telemetry::wifiConnected() && !webStarted) {
    web_ui::begin(&soilReading, &envReading);
    webStarted = true;
    Serial.print("[net ] connected, dashboard at http://");
    Serial.println(WiFi.localIP());
  }
  if (webStarted) web_ui::handle();

  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;
    soilProbe.read(soilReading);
    sensors::read(envReading);
    display::showReadings(soilReading, envReading);
    logReadings();
  }

  if (now - lastUpload >= UPLOAD_INTERVAL_MS && telemetry::wifiConnected()) {
    lastUpload = now;
    const int code = telemetry::upload(soilReading, envReading);
    Serial.printf("[http] upload -> %d\n", code);
  }
}
