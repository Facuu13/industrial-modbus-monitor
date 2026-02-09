
---

# Industrial Modbus Monitor — ESP32-C3 + Modbus RTU + MQTT + Docker

End-to-end industrial IoT project that simulates a **Modbus RTU (RS-485)** device monitor using **ESP32-C3 (ESP-IDF)** and publishes telemetry over **MQTT**.  
A Dockerized backend subscribes to MQTT, stores data in **SQLite**, and exposes a **FastAPI** REST API to query devices and telemetry.

> The Modbus device is simulated in firmware (no RS-485 hardware required). The code is architected with a transport abstraction so real RS-485 can be plugged in later with minimal changes.

---

## ✅ Features

### Firmware (ESP32-C3 / ESP-IDF)
- Modular architecture (Wi-Fi, MQTT, Modbus core, transport layer, model, status)
- **Transport abstraction**:
  - `transport_sim` (current)
  - `transport_rs485` (ready for real UART RS-485)
- Modbus RTU core:
  - CRC16
  - build request (0x03 Read Holding Registers)
  - parse response + exception handling
- Motor telemetry model:
  - voltage, current, rpm, temperature
- Industrial status evaluation:
  - `OK / WARN / CRIT`
  - flags: `COMM_LOSS`, `UNDERVOLT`, `OVERVOLT`, `OVERCURR`, `OVERTEMP`
- Communication robustness:
  - simulated timeouts + bad CRC
  - consecutive-failure threshold -> `COMM_LOSS`
- Configurable thresholds via `menuconfig`
- MQTT publish JSON payload per device

### Backend (Docker)
- Mosquitto broker
- MQTT listener (Python):
  - subscribes to `industrial/modbus/+/telemetry`
  - stores in `data/telemetry.jsonl`
  - stores in `data/telemetry.db` (SQLite)
- REST API (FastAPI) reading SQLite:
  - `GET /health`
  - `GET /devices`
  - `GET /latest?device_id=...`
  - `GET /recent?limit=...`
  - Swagger docs: `/docs`

---

## 🧱 Architecture

```

(Modbus Device)          (simulated in firmware)
│  RS-485 / Modbus RTU
▼
ESP32-C3 (ESP-IDF)

* Modbus RTU core + CRC
* Transport layer (SIM now / RS485 later)
* Telemetry model + status eval
* Wi-Fi + MQTT publish
  │
  │ MQTT
  ▼
  Mosquitto (Docker)
  │
  ▼
  Listener (Docker, Python)
  -> JSONL + SQLite
  │
  ▼
  API (Docker, FastAPI)
  -> /devices /latest /recent

```

---

## 📦 Repo Structure

```

industrial-modbus-monitor/
├─ firmware/                # ESP-IDF project
│  ├─ main/
│  │  ├─ wifi_manager.*     # Wi-Fi STA + retry + eventgroup
│  │  ├─ mqtt_manager.*     # MQTT connect/publish
│  │  ├─ modbus_crc.*       # CRC16 Modbus
│  │  ├─ modbus_rtu.*       # build/parse RTU frames
│  │  ├─ modbus_sim.*       # simulated Modbus slave (holding regs + faults)
│  │  ├─ transport_*.       # sim + rs485
│  │  ├─ motor_model.*      # regs -> physical telemetry
│  │  ├─ motor_status.*     # OK/WARN/CRIT + flags (menuconfig thresholds)
│  │  ├─ modbus_poll.*      # polling pipeline using transport
│  │  └─ ...
├─ docker/
│  ├─ docker-compose.yml
│  └─ mosquitto.conf
├─ backend_listener/
│  ├─ app/listener.py
│  ├─ app/db.py
│  └─ Dockerfile
├─ backend_api/
│  ├─ app/api.py
│  └─ Dockerfile
└─ data/                    # generated (jsonl + sqlite)

````

---

## 🚀 Quick Start

### 1) Start backend (Docker)
```bash
cd docker
docker compose up -d --build
docker compose ps
````

Verify listener is storing:

```bash
docker compose logs -f listener
tail -n 5 ../data/telemetry.jsonl
```

API:

* [http://127.0.0.1:8000/health](http://127.0.0.1:8000/health)
* [http://127.0.0.1:8000/docs](http://127.0.0.1:8000/docs)

---

### 2) ESP32 Firmware (publish telemetry)

Set Wi-Fi and MQTT configs:

```bash
cd firmware
idf.py menuconfig
```

Configure:

* **App WiFi**: SSID / Password
* **App MQTT**:

  * Broker URI: `mqtt://192.168.1.11:1883`
  * Client ID: `esp32c3-modbus-001`
  * Topic Base: `industrial/modbus`
* **Industrial Modbus Monitor - Thresholds**: optional tuning

Build + flash:

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

---

## 📡 MQTT Topics

Telemetry published to:

```
industrial/modbus/<client_id>/telemetry
```

Example payload:

```json
{
  "device_id": "esp32c3-modbus-001",
  "uptime_s": 123,
  "voltage_v": 230.0,
  "current_a": 5.12,
  "rpm": 1450,
  "temp_c": 12.3,
  "level": "OK",
  "flags": 0
}
```

Quick subscribe:

```bash
mosquitto_sub -h 192.168.1.11 -t "industrial/modbus/+/telemetry" -v
```

---

## 🔎 API Endpoints

* `GET /health`
* `GET /devices`
* `GET /latest?device_id=esp32c3-modbus-001`
* `GET /recent?limit=50`

Examples:

```bash
curl http://127.0.0.1:8000/devices
curl "http://127.0.0.1:8000/latest?device_id=esp32c3-modbus-001"
curl "http://127.0.0.1:8000/recent?limit=5"
```

---

## 🧪 Notes on Simulation

This project runs without RS-485 hardware:

* `modbus_sim` provides holding registers and injects faults:

  * timeout (no response)
  * bad CRC
* communication supervisor sets `COMM_LOSS` after N consecutive failures
* `transport_rs485` is already included for future real RS-485

---

