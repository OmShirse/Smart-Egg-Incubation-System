# 🥚 Smart Egg Incubation System

> **IoT-based automated egg incubator** — ESP8266 + DHT11 + Blynk with stage-based temperature/humidity control over a 21-day cycle.

[![GitHub](https://img.shields.io/badge/GitHub-OmShirse-blue?logo=github)](https://github.com/OmShirse/Smart-Egg-Incubation-System)

---

## 📌 Project Description

The **Smart Egg Incubation System** automates the egg incubation process by maintaining **precise temperature (37 °C) and humidity** conditions through a **21-day cycle**. It uses:

- **ESP8266 (NodeMCU)** — main microcontroller with Wi-Fi
- **DHT11** — temperature & humidity sensor
- **Relay-controlled heater** and **mist humidifier**
- **Blynk IoT** — real-time mobile monitoring & control

The incubation cycle is divided into **two stages** with automatic humidity adjustment while keeping temperature constant.

---

## 🎯 Objectives

| Goal | Implementation |
|---|---|
| Constant 37 °C temperature | PID/threshold relay control of heater |
| Stage-based humidity | Stage 1: 55–60% RH · Stage 2: 65–70% RH |
| 21-day automation | Internal timer with day-tracking |
| Real-time monitoring | Blynk app dashboard (temp, humidity, stage) |
| Minimal human intervention | Fully autonomous with fault alerts |

---

## ⚙️ System Architecture

```
[DHT11 Sensor]
      │  temp + humidity
      ▼
[ESP8266 NodeMCU]
   ├─► [Relay 1] ──► [Heating Element]
   ├─► [Relay 2] ──► [Mist Humidifier / Water Pump]
   ├─► [LCD 16x2] (local display)
   └─► [Blynk Cloud] ──► [Mobile App]
```

**Block Diagram:** [View on GitHub](https://github.com/OmShirse/Smart-Egg-Incubation-System/blob/main/Smart%20Egg%20Incubation%20System.png)

---

## 🧩 Hardware Components

| Component | Description |
|---|---|
| ESP8266 NodeMCU | Main controller (Wi-Fi built-in) |
| DHT11 | Temperature & humidity sensor |
| Relay Module (2-ch) | Controls heater and humidifier |
| Heating Element / Bulb | 25W–40W incandescent or heating pad |
| Mist Humidifier / Pump | Ultrasonic mist maker or water pump |
| LCD 16×2 (I2C) | Local status display |
| Power Supply | 5V/3A for ESP8266 + 12V for relays |

---

## 💻 Software Requirements

- Arduino IDE with **ESP8266 board package**
- Libraries:
  - `DHT sensor library` (Adafruit)
  - `Blynk` (latest)
  - `LiquidCrystal_I2C`
  - `NTPClient` (for real-time clock sync)

---

## 🚀 Quick Start

### 1. Set up Arduino IDE
```
Board: NodeMCU 1.0 (ESP-12E Module)
Upload Speed: 115200
```

### 2. Configure credentials
Edit `config.h`:
```cpp
#define WIFI_SSID     "your_ssid"
#define WIFI_PASS     "your_password"
#define BLYNK_AUTH    "your_blynk_token"
```

### 3. Upload & monitor
```
Tools → Serial Monitor → 115200 baud
```

---

## 📁 Project Structure

```
Smart-Egg-Incubation-System/
├── SmartIncubator.ino     # Main Arduino sketch
├── config.h               # Wi-Fi, Blynk credentials, thresholds
├── stages.h               # Stage definitions (day ranges, humidity targets)
├── README.md
└── Smart Egg Incubation System.png  # Block diagram
```

---

## 🌡️ Incubation Stages

| Stage | Days | Temperature | Humidity |
|---|---|---|---|
| Stage 1 (Development) | Day 1–18 | 37.5 °C | 55–60% RH |
| Stage 2 (Hatching) | Day 19–21 | 37.2 °C | 65–70% RH |

---

## 🔭 Further Scope / Improvements

- [ ] **Egg turner motor** — auto-rotate eggs every 4 hours (servo/motor relay)
- [ ] **PID controller** — replace threshold relay with PID for stable temp
- [ ] **OTA firmware updates** — update ESP8266 remotely via ArduinoOTA
- [ ] **Data logging** — log temp/humidity to SD card or Firebase for analysis
- [ ] **Hatch detection** — use IR sensor or microphone to detect hatch events
- [ ] **Emergency SMS alerts** — Twilio API if temp goes out of range
- [ ] **Upgrade to DHT22** — ±0.5 °C accuracy vs DHT11's ±2 °C
- [ ] **Web dashboard** — standalone ESP8266 web server (no Blynk dependency)
- [ ] **Multi-species profiles** — preset profiles for duck (28 days), quail (17 days)

---

## 🤝 Contributing

See [GitHub repo](https://github.com/OmShirse/Smart-Egg-Incubation-System) for open issues.

---

*Part of the [OmShirse/chess](https://github.com/OmShirse/chess) monorepo*
