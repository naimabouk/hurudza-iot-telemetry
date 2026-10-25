#include "sensors.h"

#include <Adafruit_BME280.h>
#include <Wire.h>

#include "config.h"

namespace {

Adafruit_BME280 bme;
bool bme_present = false;

const char* classifyWater(int raw) {
  if (raw < WATER_LOW) return "empty";
  if (raw < WATER_MEDIUM) return "low";
  if (raw < WATER_HIGH) return "medium";
  return "high";
}

const char* classifyLight(int raw) {
  if (raw < LIGHT_DARK) return "dark";
  if (raw < LIGHT_DIM) return "dim";
  if (raw < LIGHT_LIGHT) return "light";
  if (raw < LIGHT_BRIGHT) return "bright";
  return "very bright";
}

// Resistive water probes corrode quickly under constant DC (electrolysis). The probe is
// powered from a GPIO only for the few milliseconds needed to take one sample.
int sampleWaterLevel() {
  digitalWrite(PIN_WATER_POWER, HIGH);
  delay(WATER_SETTLE_MS);
  const int raw = analogRead(PIN_WATER_SIGNAL);
  digitalWrite(PIN_WATER_POWER, LOW);
  return raw;
}

}  // namespace

namespace sensors {

bool begin() {
  pinMode(PIN_RAIN_DIGITAL, INPUT);
  pinMode(PIN_WATER_POWER, OUTPUT);
  digitalWrite(PIN_WATER_POWER, LOW);
  analogReadResolution(12);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  // Breakout boards ship with address 0x76 or 0x77 depending on the SDO strap.
  bme_present = bme.begin(0x76) || bme.begin(0x77);
  return bme_present;
}

void read(EnvReading& r) {
  r.bme_ok = bme_present;
  if (bme_present) {
    r.air_temp_c = bme.readTemperature();
    r.air_humidity_pct = bme.readHumidity();
    r.pressure_hpa = bme.readPressure() / 100.0f;
    r.altitude_m = bme.readAltitude(SEA_LEVEL_HPA);
  }

  r.air_quality_raw = analogRead(PIN_MQ135);
  r.poor_air = r.air_quality_raw >= MQ135_POOR_AIR;

  r.rain_raw = analogRead(PIN_RAIN_ANALOG);
  r.raining = digitalRead(PIN_RAIN_DIGITAL) == LOW;

  r.water_raw = sampleWaterLevel();
  r.water_level = classifyWater(r.water_raw);

  r.light_raw = analogRead(PIN_LDR);
  r.light_level = classifyLight(r.light_raw);
}

}  // namespace sensors
