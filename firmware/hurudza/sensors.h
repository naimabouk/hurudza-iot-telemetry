// sensors.h - atmospheric and auxiliary sensors (BME280, MQ-135, rain, water level, LDR).
#pragma once

#include <Arduino.h>

struct EnvReading {
  // BME280
  bool bme_ok = false;
  float air_temp_c = NAN;
  float air_humidity_pct = NAN;
  float pressure_hpa = NAN;
  float altitude_m = NAN;
  // MQ-135: raw 12-bit ADC count (uncalibrated, used only as a relative air-quality index)
  int air_quality_raw = 0;
  bool poor_air = false;
  // Rain board
  int rain_raw = 0;
  bool raining = false;
  // Water-level probe
  int water_raw = 0;
  const char* water_level = "unknown";  // empty / low / medium / high
  // LDR
  int light_raw = 0;
  const char* light_level = "unknown";  // dark / dim / light / bright / very bright
};

namespace sensors {
bool begin();             // returns false if the BME280 is not found
void read(EnvReading& r);
}  // namespace sensors
