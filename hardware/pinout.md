# Pinout and bill of materials

## ESP32 GPIO map

The authoritative source is [`firmware/hurudza/config.h`](../firmware/hurudza/config.h).

| GPIO | Direction | Connected to | Notes |
|---|---|---|---|
| 26 | in (UART2 RX) | MAX485 **RO** | RS-485 receive |
| 27 | out (UART2 TX) | MAX485 **DI** | RS-485 transmit |
| 32 | out | MAX485 **RE̅** | LOW = receive enabled |
| 33 | out | MAX485 **DE** | HIGH = driver enabled |
| 21 | I²C SDA | BME280 SDA | address 0x76 or 0x77 |
| 22 | I²C SCL | BME280 SCL | |
| 34 | in (ADC1_CH6) | LDR / 10 kΩ divider | input-only pin |
| 35 | in (ADC1_CH7) | MQ-135 AO (through resistor network) | input-only pin |
| 36 | in (ADC1_CH0, VP) | Water-level probe signal | input-only pin |
| 39 | in (ADC1_CH3, VN) | Rain board AO | input-only pin |
| 14 | in | Rain board DO | LOW = wet (trimmer-set comparator) |
| 4 | out | Water-level probe V+ | power gate, HIGH only while sampling |
| 19, 23 | out | LCD RS, E | HD44780, 4-bit mode |
| 18, 17, 16, 15 | out | LCD D4–D7 | |

**Why all analog inputs are on ADC1:** the ESP32's ADC2 is shared with the Wi-Fi radio and
can't be read while Wi-Fi is active. Every analog sensor therefore sits on an ADC1 pin
(GPIO 32–39).

**Voltage levels:** the probe, MQ-135 and LCD run from the 5 V rail. ESP32 GPIOs are 3.3 V and
not 5 V tolerant, so the MQ-135 outputs go through a resistor network before reaching the ADC
(see [schematic](schematics/hurudza_schematic.png)).

## RS-485 / Modbus RTU parameters

| Parameter | Value |
|---|---|
| Physical layer | RS-485 half-duplex, MAX485 transceiver |
| Framing | 4800 baud, 8 data bits, no parity, 1 stop bit |
| Slave address | 6 (factory default is 1; re-addressed via register 0x07D0) |
| Function | 0x03 Read Holding Registers, start 0x0000, count 7 |
| Probe wiring | brown = V+ (4.5–30 V DC), black = GND, yellow/green = A+, blue = B− |

| Register | Quantity | Scale | Range |
|---|---|---|---|
| 0x0000 | Soil moisture | 0.1 %RH | 0–100 % |
| 0x0001 | Soil temperature | 0.1 °C (signed) | −40–80 °C |
| 0x0002 | Electrical conductivity | 1 µS/cm | 0–20 000 |
| 0x0003 | pH | 0.1 | 3–9 |
| 0x0004 | Nitrogen | 1 mg/kg | 1–2999 |
| 0x0005 | Phosphorus | 1 mg/kg | 1–2999 |
| 0x0006 | Potassium | 1 mg/kg | 1–2999 |

Source: [CWT NPK probe manual](datasheets/CWT_NPK_soil_sensor_manual.pdf).

## Bill of materials

| Qty | Part | Role |
|---|---|---|
| 1 | ESP32 DevKit V1 (30-pin, ESP-WROOM-32) | MCU, Wi-Fi |
| 1 | CWT 5-pin NPK soil probe, RS-485, IP68 | N, P, K, pH, moisture, temperature, EC |
| 1 | MAX485 TTL↔RS-485 module | Half-duplex bus transceiver |
| 1 | BME280 breakout (I²C) | Air temperature, humidity, pressure |
| 1 | MQ-135 gas sensor module | Relative air-quality index |
| 1 | Rain sensor board + comparator module | Rain detection |
| 1 | Resistive water-level sensor | Reservoir level (4 classes) |
| 1 | LDR + 10 kΩ resistor | Ambient light class |
| 1 | 16×2 HD44780 LCD + 10 kΩ contrast potentiometer | Field display |
| 2 | 1 kΩ resistors | MQ-135 output network |
| 1 | 5 V DC supply | Power rail |
| 1 | 3D-printed enclosure | Field housing with LCD window, optical window and cable exits |
| 1 | NEO-6M V2 GPS module (footprint on PCB rev.) | Geotagging (not yet used by the firmware) |
