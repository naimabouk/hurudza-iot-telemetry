// display.h - 16x2 character LCD for in-field readout.
#pragma once

#include "sensors.h"
#include "soil_registers.h"

namespace display {
void begin();
void message(const char* line1, const char* line2 = "");
// Alternates every call between a soil page (NPK / pH / moisture / temp)
// and an environment page (air temp / humidity / water / rain).
void showReadings(const soil::Reading& soil, const EnvReading& env);
}  // namespace display
