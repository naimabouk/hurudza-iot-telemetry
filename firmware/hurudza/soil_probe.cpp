#include "soil_probe.h"

#include "config.h"

void SoilProbe::begin() {
  pinMode(PIN_RS485_RE, OUTPUT);
  pinMode(PIN_RS485_DE, OUTPUT);
  receiveMode();
  // ESP32 UART2 remapped to the RS-485 pins (hardware UART instead of SoftwareSerial:
  // no bit-banging, so Wi-Fi interrupts cannot corrupt bytes at 4800 baud).
  port_.begin(RS485_BAUD, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
}

void SoilProbe::transmitMode() {
  digitalWrite(PIN_RS485_DE, HIGH);
  digitalWrite(PIN_RS485_RE, HIGH);
}

void SoilProbe::receiveMode() {
  digitalWrite(PIN_RS485_DE, LOW);
  digitalWrite(PIN_RS485_RE, LOW);
}

size_t SoilProbe::receive(uint8_t* buf, size_t expected, uint32_t timeout_ms) {
  size_t n = 0;
  const uint32_t start = millis();
  // Bounded wait: returns as soon as the full frame arrives, or at the deadline.
  while (n < expected && millis() - start < timeout_ms) {
    if (port_.available()) buf[n++] = static_cast<uint8_t>(port_.read());
    else delay(1);
  }
  return n;
}

bool SoilProbe::read(soil::Reading& out) {
  uint8_t req[8];
  modbus::buildReadRequest(slave_, soil::kRegStart, soil::kRegCount, req);

  while (port_.available()) port_.read();  // drop stale bytes from earlier transactions

  transmitMode();
  port_.write(req, sizeof req);
  port_.flush();  // blocks until the last stop bit leaves the UART
  receiveMode();  // release the bus at once so the slave can answer

  uint8_t resp[modbus::expectedResponseLength(soil::kRegCount)];
  const size_t len = receive(resp, sizeof resp, MODBUS_TIMEOUT_MS);

  uint16_t regs[soil::kRegCount];
  status_ = modbus::parseReadResponse(resp, len, slave_, soil::kRegCount, regs);
  if (status_ != modbus::Status::Ok) {
    ++errors_;
    out.valid = false;
    return false;
  }
  out = soil::decode(regs);
  return true;
}
