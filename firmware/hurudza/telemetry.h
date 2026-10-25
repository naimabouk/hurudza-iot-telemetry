// telemetry.h - Wi-Fi management and HTTPS upload of readings to the cloud endpoint.
#pragma once

#include "sensors.h"
#include "soil_registers.h"

namespace telemetry {
void beginWifi();
void maintainWifi();  // non-blocking reconnect, call every loop
bool wifiConnected();
// POST one sample. Returns the HTTP status code, or a negative value on transport error.
int upload(const soil::Reading& soil, const EnvReading& env);
}  // namespace telemetry
