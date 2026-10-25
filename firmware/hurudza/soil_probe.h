// soil_probe.h - CWT NPK soil probe over half-duplex RS-485 (MAX485 transceiver).
#pragma once

#include <Arduino.h>

#include "modbus_rtu.h"
#include "soil_registers.h"

class SoilProbe {
 public:
  SoilProbe(HardwareSerial& port, uint8_t slave) : port_(port), slave_(slave) {}

  void begin();

  // Read all 7 soil registers in one Modbus transaction.
  // Returns true on success; on failure `lastStatus()` tells why and `out.valid` is false.
  bool read(soil::Reading& out);

  modbus::Status lastStatus() const { return status_; }
  uint32_t errorCount() const { return errors_; }

 private:
  void transmitMode();
  void receiveMode();
  size_t receive(uint8_t* buf, size_t expected, uint32_t timeout_ms);

  HardwareSerial& port_;
  uint8_t slave_;
  modbus::Status status_ = modbus::Status::Timeout;
  uint32_t errors_ = 0;
};
