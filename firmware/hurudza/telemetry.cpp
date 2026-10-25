#include "telemetry.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "config.h"
#include "secrets.h"

namespace {
uint32_t last_attempt = 0;

void appendField(String& body, const char* key, const String& value) {
  body += '&';
  body += key;
  body += '=';
  body += value;
}
}  // namespace

namespace telemetry {

void beginWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  last_attempt = millis();
}

void maintainWifi() {
  // Unlike the v1 firmware, a lost access point never blocks the main loop:
  // sensing and the LCD keep running while reconnection is retried in the background.
  if (WiFi.status() != WL_CONNECTED && millis() - last_attempt > WIFI_RETRY_INTERVAL_MS) {
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    last_attempt = millis();
  }
}

bool wifiConnected() { return WiFi.status() == WL_CONNECTED; }

int upload(const soil::Reading& s, const EnvReading& e) {
  if (!wifiConnected()) return -1;

  // Field names val1..val7 are kept for compatibility with the existing sm.php endpoint.
  String body = String("api_key=") + TELEMETRY_KEY;
  if (s.valid) {
    appendField(body, "val1", String(s.nitrogen_mg_kg));
    appendField(body, "val2", String(s.phosphorus_mg_kg));
    appendField(body, "val3", String(s.potassium_mg_kg));
    appendField(body, "val4", String(s.ph, 1));
    appendField(body, "val5", String(s.moisture_pct, 1));
    appendField(body, "val6", String(s.temperature_c, 1));
    appendField(body, "val7", String(s.ec_us_cm));
  }
  if (e.bme_ok) {
    appendField(body, "env_temp", String(e.air_temp_c, 2));
    appendField(body, "env_hum", String(e.air_humidity_pct, 2));
    appendField(body, "pressure", String(e.pressure_hpa, 2));
  }
  appendField(body, "air_raw", String(e.air_quality_raw));
  appendField(body, "rain", e.raining ? "1" : "0");
  appendField(body, "water", e.water_level);
  appendField(body, "light_raw", String(e.light_raw));

  WiFiClientSecure client;
  client.setInsecure();  // prototype only: pin the server certificate for a real deployment
  HTTPClient http;
  http.setTimeout(5000);
  if (!http.begin(client, TELEMETRY_URL)) return -2;
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  const int code = http.POST(body);
  http.end();
  return code;
}

}  // namespace telemetry
