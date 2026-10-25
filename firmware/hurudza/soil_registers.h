// soil_registers.h - register map and scaling for the CWT 5-pin NPK soil probe.
// Source: CWT Soil Sensor (NPK type) manual, see hardware/datasheets/.
// Pure C++ (no Arduino dependency) so it can be unit-tested on the host.
#pragma once

#include <stdint.h>

namespace soil {

// Holding registers 0x0000-0x0006 can be read in a single 0x03 request.
constexpr uint16_t kRegStart = 0x0000;
constexpr uint16_t kRegCount = 7;

enum Reg : uint8_t {
  kMoisture = 0,     // 0.1 %RH
  kTemperature = 1,  // 0.1 degC, signed (range -40..80 degC)
  kConductivity = 2, // 1 uS/cm (0..20000)
  kPh = 3,           // 0.1 pH (3..9)
  kNitrogen = 4,     // 1 mg/kg (1..2999)
  kPhosphorus = 5,   // 1 mg/kg
  kPotassium = 6,    // 1 mg/kg
};

struct Reading {
  float moisture_pct = 0;
  float temperature_c = 0;
  uint16_t ec_us_cm = 0;
  float ph = 0;
  uint16_t nitrogen_mg_kg = 0;
  uint16_t phosphorus_mg_kg = 0;
  uint16_t potassium_mg_kg = 0;
  bool valid = false;
};

// Convert raw 16-bit registers into engineering units.
inline Reading decode(const uint16_t r[kRegCount]) {
  Reading out;
  out.moisture_pct = r[kMoisture] / 10.0f;
  out.temperature_c = static_cast<int16_t>(r[kTemperature]) / 10.0f;  // two's complement
  out.ec_us_cm = r[kConductivity];
  out.ph = r[kPh] / 10.0f;
  out.nitrogen_mg_kg = r[kNitrogen];
  out.phosphorus_mg_kg = r[kPhosphorus];
  out.potassium_mg_kg = r[kPotassium];
  out.valid = true;
  return out;
}

}  // namespace soil
