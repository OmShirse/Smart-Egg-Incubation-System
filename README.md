# 🥚 Smart Egg Incubation System

**ESP8266 | DHT11 | Blynk IoT | Stage-Based Control**

---

## 📌 Project Description

The **Smart Egg Incubation System** is an IoT-based embedded system designed to automate the incubation process of eggs by maintaining **precise temperature and humidity conditions** over a **21-day incubation cycle**.

The system uses an **ESP8266 (NodeMCU)** microcontroller, a **DHT11 temperature and humidity sensor**, and **relay-controlled heating and humidification elements**. Real-time monitoring and control are provided through the **Blynk IoT platform**.

The incubation process is divided into **two stages**, with automatic adjustment of humidity levels while maintaining a constant temperature.

---

## 🎯 Objectives

* Maintain **constant temperature at 37 °C**
* Implement **stage-based humidity control**
* Automate incubation timing (21 days)
* Provide **real-time monitoring via mobile app**
* Reduce manual intervention and human error

---

## ⚙️ System Block Diagram

The following block diagram illustrates the overall architecture and signal flow of the system:

🔗 **Block Diagram:**
[Click to view Block Diagram](https://github.com/OmShirse/Smart-Egg-Incubation-System/blob/7ea6b5472672305fc80a21d23a12cebe9834e551/Smart%20Egg%20Incubation%20System.png)

**Description:**

* The **DHT11 sensor** continuously measures temperature and humidity.
* Sensor data is processed by the **ESP8266 microcontroller**.
* Based on predefined thresholds, control signals are sent to:

  * **Heating element** (via relay)
  * **Humidifier / water pump** (via relay)
* Data is transmitted to the **Blynk IoT platform** over Wi-Fi for monitoring.

---

## 🧩 Hardware Components

| Component               | Description                      |
| ----------------------- | -------------------------------- |
| ESP8266 NodeMCU         | Main control unit                |
| DHT11 Sensor            | Temperature and humidity sensing |
| 2-Channel Relay Module  | Controls heater and humidifier   |
| Heating Element         | Maintains incubation temperature |
| Humidifier / Water Pump | Controls humidity                |
| Wi-Fi Network           | IoT connectivity                 |

---

## 💻 Software Requirements

* Arduino IDE
* ESP8266 Board Package
* Required Libraries:

  * `ESP8266WiFi.h`
  * `BlynkSimpleEsp8266.h`
  * `DHT.h`

---

## 📱 Blynk IoT Configuration

The system uses **Blynk virtual pins** for real-time monitoring:

| Virtual Pin | Function               |
| ----------- | ---------------------- |
| V0          | Temperature display    |
| V1          | Humidity display       |
| V2          | Heater status          |
| V3          | Humidifier status      |
| V4          | System status          |
| V5          | Incubation day counter |
| V6          | Stage information      |
| V7          | Reset incubation timer |

---

## 🔄 Incubation Logic

### Temperature Control

* Target temperature: **37 °C**
* Heater ON if temperature drops below tolerance
* Heater OFF if temperature exceeds tolerance

### Humidity Control

| Incubation Stage   | Days  | Humidity Range |
| ------------------ | ----- | -------------- |
| Stage 1            | 1–18  | 50–55 %        |
| Stage 2 (Lockdown) | 19–21 | 65–75 %        |

Automatic switching between stages is based on elapsed time calculated using `millis()`.

---

## ⏱️ Incubation Timeline

* Total duration: **21 days**
* Day counter automatically increments
* Lockdown warning activated on Day 19
* Completion notification after Day 21

---

## ✅ Key Features

* Fully automated incubation control
* Stage-based humidity regulation
* Real-time IoT monitoring
* Sensor error detection
* Manual reset via mobile app
* Hysteresis control to protect relays

---

## ⚠️ Safety Notes

* Do not open the incubator during **lockdown period (Days 19–21)**
* Ensure proper electrical isolation for relay modules
* DHT11 sensor requires minimum 2-second read interval

---

## 📌 Conclusion

This project demonstrates a **reliable, low-cost, and efficient smart incubation system** suitable for academic projects, research prototypes, and small-scale poultry farming. The integration of IoT enhances monitoring accuracy and operational convenience.

---

