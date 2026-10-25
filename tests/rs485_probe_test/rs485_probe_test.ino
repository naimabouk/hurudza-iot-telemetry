// rs485_probe_test.ino - bench diagnostic for the CWT NPK probe on the Hurudza wiring.
//
// 1. Scans Modbus slave addresses 1..247 (or a fixed one) at 4800 8N1.
// 2. For a responding probe, dumps the raw response frame in hex, checks the CRC,
//    and prints the 7 decoded soil registers every 2 s.
//
// Typical faults it exposes: A/B swapped (no response at all), wrong slave address,
// wrong baud rate, DE/RE wiring (echo of our own request), noise (CRC errors).
// Self-contained on purpose: open this folder in the Arduino IDE and flash.

#include <Arduino.h>

constexpr int PIN_RX = 26, PIN_TX = 27, PIN_RE = 32, PIN_DE = 33;
constexpr uint32_t BAUD = 4800;
constexpr uint8_t FIXED_ADDRESS = 0;  // 0 = scan; set e.g. 6 to skip scanning

uint8_t slave = 0;

uint16_t crc16(const uint8_t* d, size_t n) {
  uint16_t c = 0xFFFF;
  while (n--) {
    c ^= *d++;
    for (int i = 0; i < 8; ++i) c = (c & 1) ? (c >> 1) ^ 0xA001 : c >> 1;
  }
  return c;
}

size_t transact(uint8_t addr, uint8_t* resp, size_t max_len) {
  uint8_t req[8] = {addr, 0x03, 0x00, 0x00, 0x00, 0x07, 0, 0};
  const uint16_t c = crc16(req, 6);
  req[6] = c & 0xFF;
  req[7] = c >> 8;
  while (Serial2.available()) Serial2.read();
  digitalWrite(PIN_DE, HIGH);
  digitalWrite(PIN_RE, HIGH);
  Serial2.write(req, 8);
  Serial2.flush();
  digitalWrite(PIN_DE, LOW);
  digitalWrite(PIN_RE, LOW);
  size_t n = 0;
  const uint32_t t0 = millis();
  while (n < max_len && millis() - t0 < 300) {
    if (Serial2.available()) resp[n++] = Serial2.read();
  }
  return n;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RE, OUTPUT);
  pinMode(PIN_DE, OUTPUT);
  Serial2.begin(BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(500);

  if (FIXED_ADDRESS) {
    slave = FIXED_ADDRESS;
  } else {
    Serial.println("Scanning Modbus addresses 1..247 @ 4800 8N1 ...");
    uint8_t buf[32];
    for (uint8_t a = 1; a <= 247 && !slave; ++a) {
      if (transact(a, buf, sizeof buf) >= 19 && buf[0] == a) slave = a;
    }
  }
  if (slave) Serial.printf("Probe found at address %u\n", slave);
  else Serial.println("No probe answered (check A/B wiring, 5-30 V supply, baud rate)");
}

void loop() {
  if (!slave) { delay(1000); return; }
  uint8_t r[32];
  const size_t n = transact(slave, r, sizeof r);

  Serial.printf("RX %2u bytes:", static_cast<unsigned>(n));
  for (size_t i = 0; i < n; ++i) Serial.printf(" %02X", r[i]);
  Serial.println();

  if (n >= 19) {
    const bool crc_ok = crc16(r, 17) == (r[17] | (r[18] << 8));
    auto reg = [&](int i) { return static_cast<uint16_t>((r[3 + 2 * i] << 8) | r[4 + 2 * i]); };
    Serial.printf("CRC %s | M=%.1f%% T=%.1fC EC=%u pH=%.1f N=%u P=%u K=%u\n",
                  crc_ok ? "OK " : "BAD", reg(0) / 10.0, static_cast<int16_t>(reg(1)) / 10.0,
                  reg(2), reg(3) / 10.0, reg(4), reg(5), reg(6));
  }
  delay(2000);
}
