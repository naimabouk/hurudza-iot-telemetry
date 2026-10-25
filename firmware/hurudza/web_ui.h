// web_ui.h - local HTTP dashboard served by the node (works without internet access).
#pragma once

#include "sensors.h"
#include "soil_registers.h"

namespace web_ui {
void begin(const soil::Reading* soil, const EnvReading* env);  // pointers to live readings
void handle();  // call every loop (non-blocking)
}  // namespace web_ui
