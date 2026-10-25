#include "web_ui.h"

#include <WebServer.h>

namespace {
WebServer server(80);
const soil::Reading* soil_ptr = nullptr;
const EnvReading* env_ptr = nullptr;

void row(String& html, const char* name, const String& value) {
  html += "<tr><td>";
  html += name;
  html += "</td><td><span class='v'>";
  html += value;
  html += "</span></td></tr>";
}

void handleRoot() {
  const soil::Reading& s = *soil_ptr;
  const EnvReading& e = *env_ptr;
  String html;
  html.reserve(3000);
  html += F("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<meta http-equiv='refresh' content='5'><title>Hurudza</title><style>"
            "body{font-family:Arial;text-align:center}table{border-collapse:collapse;margin:auto;min-width:320px}"
            "th{padding:10px;background:#0043af;color:#fff}td{padding:8px;border-bottom:1px solid #ddd}"
            ".v{font-weight:bold}</style></head><body><h2>Hurudza node</h2>"
            "<table><tr><th>Reading</th><th>Value</th></tr>");
  if (s.valid) {
    row(html, "Nitrogen N", String(s.nitrogen_mg_kg) + " mg/kg");
    row(html, "Phosphorus P", String(s.phosphorus_mg_kg) + " mg/kg");
    row(html, "Potassium K", String(s.potassium_mg_kg) + " mg/kg");
    row(html, "Soil pH", String(s.ph, 1));
    row(html, "Soil moisture", String(s.moisture_pct, 1) + " %");
    row(html, "Soil temperature", String(s.temperature_c, 1) + " &deg;C");
    row(html, "Soil EC", String(s.ec_us_cm) + " &micro;S/cm");
  } else {
    row(html, "Soil probe", "no response");
  }
  if (e.bme_ok) {
    row(html, "Air temperature", String(e.air_temp_c, 1) + " &deg;C");
    row(html, "Air humidity", String(e.air_humidity_pct, 1) + " %");
    row(html, "Pressure", String(e.pressure_hpa, 1) + " hPa");
    row(html, "Altitude (est.)", String(e.altitude_m, 1) + " m");
  }
  row(html, "Water level", e.water_level);
  row(html, "Air quality (raw ADC)", String(e.air_quality_raw) + (e.poor_air ? " (poor)" : " (fresh)"));
  row(html, "Rainfall", e.raining ? "raining" : "not raining");
  row(html, "Light", e.light_level);
  html += F("</table></body></html>");
  server.send(200, "text/html", html);
}

void handleJson() {
  const soil::Reading& s = *soil_ptr;
  const EnvReading& e = *env_ptr;
  char buf[512];
  snprintf(buf, sizeof buf,
           "{\"soil_valid\":%s,\"n\":%u,\"p\":%u,\"k\":%u,\"ph\":%.1f,\"moisture\":%.1f,"
           "\"soil_temp\":%.1f,\"ec\":%u,\"air_temp\":%.2f,\"air_hum\":%.2f,\"pressure\":%.2f,"
           "\"air_raw\":%d,\"raining\":%s,\"water\":\"%s\",\"light_raw\":%d}",
           s.valid ? "true" : "false", s.nitrogen_mg_kg, s.phosphorus_mg_kg, s.potassium_mg_kg,
           s.ph, s.moisture_pct, s.temperature_c, s.ec_us_cm, e.air_temp_c, e.air_humidity_pct,
           e.pressure_hpa, e.air_quality_raw, e.raining ? "true" : "false", e.water_level,
           e.light_raw);
  server.send(200, "application/json", buf);
}
}  // namespace

namespace web_ui {

void begin(const soil::Reading* soil, const EnvReading* env) {
  soil_ptr = soil;
  env_ptr = env;
  server.on("/", handleRoot);
  server.on("/api/readings", handleJson);
  server.begin();
}

void handle() { server.handleClient(); }

}  // namespace web_ui
