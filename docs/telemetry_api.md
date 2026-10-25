# Telemetry interfaces

## 1. Cloud upload (node → server)

Every 30 s the node sends an HTTPS `POST` with `Content-Type: application/x-www-form-urlencoded`
to `TELEMETRY_URL` (set in `secrets.h`).

| Field | Unit | Source |
|---|---|---|
| `api_key` | – | shared secret checked by the server |
| `val1` / `val2` / `val3` | mg/kg | Nitrogen / Phosphorus / Potassium |
| `val4` | pH | soil pH (0.1 resolution) |
| `val5` | % | soil moisture |
| `val6` | °C | soil temperature |
| `val7` | µS/cm | soil electrical conductivity |
| `env_temp`, `env_hum`, `pressure` | °C, %, hPa | BME280 (sent only if the sensor is detected) |
| `air_raw` | ADC count (0–4095) | MQ-135, uncalibrated |
| `rain` | 0/1 | rain board digital output |
| `water` | empty / low / medium / high | water-level class |
| `light_raw` | ADC count (0–4095) | LDR divider |

Fields `val1`–`val7` keep the names used by the original `sm.php` endpoint. Soil fields are
left out when the probe doesn't respond, so the server never stores stale or zero values as
real measurements.

## 2. Local dashboard (on the node)

| Route | Content |
|---|---|
| `GET /` | HTML table, auto-refresh every 5 s ([screenshot](images/local_web_ui.jpg)) |
| `GET /api/readings` | JSON snapshot of the latest sample |

Example `/api/readings` response (format illustration; values are examples):

```json
{"soil_valid":true,"n":32,"p":88,"k":104,"ph":6.9,"moisture":46.4,"soil_temp":25.9,
 "ec":242,"air_temp":18.90,"air_hum":68.90,"pressure":1012.86,"air_raw":453,
 "raining":false,"water":"medium","light_raw":2870}
```

## 3. Serial log (115 200 baud)

Format illustration (values are examples):

```
[soil] N=32 P=88 K=104 mg/kg  pH=6.9  M=46.4%  T=25.9C  EC=242 uS/cm
[env ] T=18.90C RH=68.90% P=1012.86hPa  air=453  rain=0  water=medium(1934)  light=bright(2870)
[http] upload -> 200
```
