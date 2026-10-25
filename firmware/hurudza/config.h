// config.h - pin map, bus settings and timing for the Hurudza node (ESP32 DevKit, 30-pin).
#pragma once

#include <stdint.h>

// ---------------- RS-485 soil probe (CWT NPK 5-pin, via MAX485) ----------------
constexpr int PIN_RS485_RX = 26;   // MAX485 RO -> ESP32
constexpr int PIN_RS485_TX = 27;   // ESP32 -> MAX485 DI
constexpr int PIN_RS485_RE = 32;   // receiver enable (active LOW)
constexpr int PIN_RS485_DE = 33;   // driver enable (active HIGH)
constexpr uint32_t RS485_BAUD = 4800;        // probe default: 4800 8N1
constexpr uint8_t SOIL_SLAVE_ID = 6;         // factory default is 1; this unit was re-addressed to 6
constexpr uint32_t MODBUS_TIMEOUT_MS = 500;  // response deadline per request

// ---------------- Analog / digital sensors ----------------
constexpr int PIN_LDR = 34;             // photoresistor voltage divider (ADC1, input-only)
constexpr int PIN_MQ135 = 35;           // MQ-135 analog out (ADC1, input-only)
constexpr int PIN_WATER_SIGNAL = 36;    // water-level probe (ADC1, VP)
constexpr int PIN_WATER_POWER = 4;      // power gate: probe energized only while sampling
constexpr int PIN_RAIN_ANALOG = 39;     // rain board AO (ADC1, VN)
constexpr int PIN_RAIN_DIGITAL = 14;    // rain board DO (LOW = wet), comparator threshold set by trimmer

// ---------------- I2C (BME280) ----------------
constexpr int PIN_I2C_SDA = 21;
constexpr int PIN_I2C_SCL = 22;
constexpr float SEA_LEVEL_HPA = 1013.25f;

// ---------------- 16x2 LCD, 4-bit parallel mode ----------------
constexpr int PIN_LCD_RS = 19, PIN_LCD_EN = 23;
constexpr int PIN_LCD_D4 = 18, PIN_LCD_D5 = 17, PIN_LCD_D6 = 16, PIN_LCD_D7 = 15;

// ---------------- Timing ----------------
constexpr uint32_t SAMPLE_INTERVAL_MS = 5000;     // sensor acquisition period
constexpr uint32_t UPLOAD_INTERVAL_MS = 30000;    // cloud telemetry period
constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 10000;
constexpr uint32_t WATER_SETTLE_MS = 10;          // probe settle time after power-up

// ---------------- Thresholds (12-bit ADC counts, empirically chosen) ----------------
constexpr int WATER_LOW = 1200, WATER_MEDIUM = 1800, WATER_HIGH = 2100;
constexpr int LIGHT_DARK = 40, LIGHT_DIM = 800, LIGHT_LIGHT = 2000, LIGHT_BRIGHT = 3200;
constexpr int MQ135_POOR_AIR = 1000;
