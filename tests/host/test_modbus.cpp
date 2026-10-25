// Host-side unit tests for the Modbus RTU helpers and soil-probe decoding.
// Build & run:  g++ -std=c++17 -I../../firmware/hurudza test_modbus.cpp -o test_modbus && ./test_modbus
#include <cmath>
#include <cstdio>

#include "modbus_rtu.h"
#include "soil_registers.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++failures; } } while (0)

int main() {
  // 1. Request frames used by the v1 firmware (slave 6, one register each) must match
  //    the hard-coded byte arrays in firmware/original/hurudza_v1.ino.
  const uint8_t expected_crc[7][2] = {
      {0x85, 0xbd}, {0xd4, 0x7d}, {0x24, 0x7d}, {0x75, 0xbd},
      {0xc4, 0x7c}, {0x95, 0xbc}, {0x65, 0xbc}};
  for (uint16_t reg = 0; reg < 7; ++reg) {
    uint8_t f[8];
    modbus::buildReadRequest(6, reg, 1, f);
    CHECK(f[6] == expected_crc[reg][0] && f[7] == expected_crc[reg][1]);
  }

  // 2. "Read 7 registers" request from the CWT manual: 01 03 00 00 00 07 04 08
  uint8_t req[8];
  modbus::buildReadRequest(1, soil::kRegStart, soil::kRegCount, req);
  const uint8_t manual_req[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  for (int i = 0; i < 8; ++i) CHECK(req[i] == manual_req[i]);

  // 3. Example response (register values from the CWT manual), CRC appended by us.
  uint8_t resp[19] = {0x01, 0x03, 0x0E,
                      0x01, 0xD0,   // moisture 46.4 %
                      0x01, 0x4C,   // temperature 33.2 C
                      0x00, 0x2C,   // EC 44 uS/cm
                      0x00, 0x5A,   // pH 9.0
                      0x00, 0x20,   // N 32
                      0x00, 0x58,   // P 88
                      0x00, 0x68,   // K 104
                      0, 0};
  const uint16_t crc = modbus::crc16(resp, 17);
  resp[17] = crc & 0xFF;
  resp[18] = crc >> 8;

  uint16_t regs[7];
  CHECK(modbus::parseReadResponse(resp, sizeof resp, 1, 7, regs) == modbus::Status::Ok);
  const soil::Reading r = soil::decode(regs);
  CHECK(std::fabs(r.moisture_pct - 46.4f) < 1e-3);
  CHECK(std::fabs(r.temperature_c - 33.2f) < 1e-3);
  CHECK(r.ec_us_cm == 44);
  CHECK(std::fabs(r.ph - 9.0f) < 1e-3);
  CHECK(r.nitrogen_mg_kg == 32 && r.phosphorus_mg_kg == 88 && r.potassium_mg_kg == 104);

  // 4. Values above 255 survive (the v1 firmware kept only the low byte).
  regs[soil::kNitrogen] = 1250;
  CHECK(soil::decode(regs).nitrogen_mg_kg == 1250);

  // 5. Negative soil temperature (-5.5 C = 0xFFC9).
  regs[soil::kTemperature] = 0xFFC9;
  CHECK(std::fabs(soil::decode(regs).temperature_c + 5.5f) < 1e-3);

  // 6. Error paths.
  uint8_t bad[19];
  for (int i = 0; i < 19; ++i) bad[i] = resp[i];
  bad[5] ^= 0x01;  // flip one bit
  CHECK(modbus::parseReadResponse(bad, 19, 1, 7, regs) == modbus::Status::BadCrc);
  CHECK(modbus::parseReadResponse(resp, 19, 6, 7, regs) == modbus::Status::WrongAddress);
  CHECK(modbus::parseReadResponse(resp, 10, 1, 7, regs) == modbus::Status::ShortFrame);
  CHECK(modbus::parseReadResponse(resp, 0, 1, 7, regs) == modbus::Status::Timeout);

  uint8_t exc[5] = {0x01, 0x83, 0x02, 0, 0};
  const uint16_t ecrc = modbus::crc16(exc, 3);
  exc[3] = ecrc & 0xFF;
  exc[4] = ecrc >> 8;
  CHECK(modbus::parseReadResponse(exc, 5, 1, 7, regs) == modbus::Status::Exception);

  if (failures == 0) std::printf("All Modbus/soil tests passed.\n");
  return failures == 0 ? 0 : 1;
}
