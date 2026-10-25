// modbus_rtu.h - minimal Modbus RTU master helpers (function 0x03, Read Holding Registers).
//
// Pure C++ with no Arduino dependency, so the same code runs on the ESP32 and in the host
// unit tests (tests/host/test_modbus.cpp).
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace modbus {

enum class Status : uint8_t {
  Ok = 0,
  Timeout,        // no bytes received before the deadline
  ShortFrame,     // fewer bytes than the minimum response length
  BadCrc,         // CRC-16 mismatch, frame corrupted on the bus
  WrongAddress,   // response came from another slave
  Exception,      // slave answered with an exception (function | 0x80)
  WrongLength,    // byte-count field does not match the requested registers
};

// CRC-16/MODBUS: polynomial 0xA001 (reflected 0x8005), initial value 0xFFFF.
// The result is sent low byte first.
inline uint16_t crc16(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; ++b) {
      crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
    }
  }
  return crc;
}

// Build an 8-byte "Read Holding Registers" request into `out`. Returns the frame length (8).
inline size_t buildReadRequest(uint8_t slave, uint16_t start, uint16_t count, uint8_t out[8]) {
  out[0] = slave;
  out[1] = 0x03;
  out[2] = static_cast<uint8_t>(start >> 8);
  out[3] = static_cast<uint8_t>(start & 0xFF);
  out[4] = static_cast<uint8_t>(count >> 8);
  out[5] = static_cast<uint8_t>(count & 0xFF);
  const uint16_t crc = crc16(out, 6);
  out[6] = static_cast<uint8_t>(crc & 0xFF);  // CRC low byte first
  out[7] = static_cast<uint8_t>(crc >> 8);
  return 8;
}

// Expected response length for `count` registers: addr + fn + bytecount + 2*count + crc(2).
constexpr size_t expectedResponseLength(uint16_t count) { return 5 + 2 * static_cast<size_t>(count); }

// Validate a 0x03 response and decode its big-endian 16-bit registers into `regs`.
inline Status parseReadResponse(const uint8_t* frame, size_t len, uint8_t slave,
                                uint16_t count, uint16_t* regs) {
  if (len == 0) return Status::Timeout;
  if (len >= 5 && (frame[1] & 0x80)) {  // exception frame: addr, fn|0x80, code, crc
    return crc16(frame, 3) == static_cast<uint16_t>(frame[3] | (frame[4] << 8))
               ? Status::Exception : Status::BadCrc;
  }
  if (len < expectedResponseLength(count)) return Status::ShortFrame;

  const size_t n = expectedResponseLength(count);
  const uint16_t rx_crc = static_cast<uint16_t>(frame[n - 2] | (frame[n - 1] << 8));
  if (crc16(frame, n - 2) != rx_crc) return Status::BadCrc;
  if (frame[0] != slave) return Status::WrongAddress;
  if (frame[2] != 2 * count) return Status::WrongLength;

  for (uint16_t i = 0; i < count; ++i) {
    regs[i] = static_cast<uint16_t>((frame[3 + 2 * i] << 8) | frame[4 + 2 * i]);
  }
  return Status::Ok;
}

inline const char* statusName(Status s) {
  switch (s) {
    case Status::Ok: return "OK";
    case Status::Timeout: return "TIMEOUT";
    case Status::ShortFrame: return "SHORT_FRAME";
    case Status::BadCrc: return "BAD_CRC";
    case Status::WrongAddress: return "WRONG_ADDRESS";
    case Status::Exception: return "EXCEPTION";
    case Status::WrongLength: return "WRONG_LENGTH";
  }
  return "UNKNOWN";
}

}  // namespace modbus
