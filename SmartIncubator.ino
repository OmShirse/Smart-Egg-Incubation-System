/*
 * Smart Egg Incubation System
 * ============================
 * ESP8266 NodeMCU + DHT11 + Blynk IoT
 * Stage-based temperature and humidity control over 21-day cycle.
 *
 * Wiring:
 *   DHT11 DATA  → D4 (GPIO2)
 *   Relay 1 IN  → D1 (GPIO5)  — Heater
 *   Relay 2 IN  → D2 (GPIO4)  — Humidifier
 *   LCD SDA     → D6 (GPIO12)
 *   LCD SCL     → D5 (GPIO14)
 *
 * Blynk Virtual Pins:
 *   V0 → Temperature
 *   V1 → Humidity
 *   V2 → Stage (1 or 2)
 *   V3 → Day number
 */

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"

// ─── Pin Definitions ─────────────────────────────────────────────────────────
#define DHT_PIN       2   // D4
#define RELAY_HEATER  5   // D1 — active LOW
#define RELAY_HUMID   4   // D2 — active LOW
#define DHT_TYPE      DHT11

// ─── Objects ──────────────────────────────────────────────────────────────────
DHT              dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiUDP          ntpUDP;
NTPClient        timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST +5:30
BlynkTimer       timer;

// ─── State ────────────────────────────────────────────────────────────────────
float    currentTemp     = 0;
float    currentHumidity = 0;
uint8_t  currentStage    = 1;
uint16_t dayCount        = 1;
uint32_t startEpoch      = 0;   // Epoch when incubation started

// ─── Stage Thresholds ─────────────────────────────────────────────────────────
// Stage 1: Day 1–18
#define STAGE1_TEMP_TARGET  37.5f
#define STAGE1_HUMID_MIN    55.0f
#define STAGE1_HUMID_MAX    60.0f

// Stage 2: Day 19–21 (hatching)
#define STAGE2_TEMP_TARGET  37.2f
#define STAGE2_HUMID_MIN    65.0f
#define STAGE2_HUMID_MAX    70.0f

#define TEMP_HYSTERESIS     0.5f  // ±0.5°C dead-band
#define TOTAL_DAYS          21


// ─── Helpers ──────────────────────────────────────────────────────────────────
void updateStage() {
    timeClient.update();
    if (startEpoch == 0) startEpoch = timeClient.getEpochTime();
    uint32_t elapsed = timeClient.getEpochTime() - startEpoch;
    dayCount = (elapsed / 86400UL) + 1;
    currentStage = (dayCount <= 18) ? 1 : 2;
}

float getTempTarget()    { return (currentStage == 1) ? STAGE1_TEMP_TARGET : STAGE2_TEMP_TARGET; }
float getHumidMin()      { return (currentStage == 1) ? STAGE1_HUMID_MIN   : STAGE2_HUMID_MIN;   }
float getHumidMax()      { return (currentStage == 1) ? STAGE1_HUMID_MAX   : STAGE2_HUMID_MAX;   }

void controlRelays() {
    float tempTarget = getTempTarget();

    // Heater control (active-LOW relay)
    if (currentTemp < tempTarget - TEMP_HYSTERESIS) {
        digitalWrite(RELAY_HEATER, LOW);   // ON
    } else if (currentTemp > tempTarget + TEMP_HYSTERESIS) {
        digitalWrite(RELAY_HEATER, HIGH);  // OFF
    }

    // Humidifier control
    if (currentHumidity < getHumidMin()) {
        digitalWrite(RELAY_HUMID, LOW);    // ON
    } else if (currentHumidity > getHumidMax()) {
        digitalWrite(RELAY_HUMID, HIGH);   // OFF
    }
}

void updateLCD() {
    lcd.setCursor(0, 0);
    lcd.printf("T:%.1fC H:%.0f%% S%d", currentTemp, currentHumidity, currentStage);
    lcd.setCursor(0, 1);
    lcd.printf("Day %d/%d          ", dayCount, TOTAL_DAYS);
}

void sendToBlynk() {
    Blynk.virtualWrite(V0, currentTemp);
    Blynk.virtualWrite(V1, currentHumidity);
    Blynk.virtualWrite(V2, currentStage);
    Blynk.virtualWrite(V3, dayCount);
}

// ─── Main Sensor Read + Control Tick ─────────────────────────────────────────
void sensorTick() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
        Serial.println("[WARN] DHT11 read failed");
        return;
    }

    currentTemp     = t;
    currentHumidity = h;

    updateStage();
    controlRelays();
    updateLCD();
    sendToBlynk();

    // Check hatching complete
    if (dayCount > TOTAL_DAYS) {
        Serial.println("[INFO] Incubation complete — Day 21 reached");
        Blynk.notify("🐣 Incubation complete! Day 21 reached.");
        digitalWrite(RELAY_HEATER, HIGH);  // OFF — safety
        digitalWrite(RELAY_HUMID,  HIGH);  // OFF
    }

    Serial.printf("[DATA] Day:%d Stage:%d T:%.2f H:%.2f\n",
                  dayCount, currentStage, currentTemp, currentHumidity);
}


// ─── Setup & Loop ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    pinMode(RELAY_HEATER, OUTPUT);
    pinMode(RELAY_HUMID,  OUTPUT);
    digitalWrite(RELAY_HEATER, HIGH);  // Start OFF
    digitalWrite(RELAY_HUMID,  HIGH);

    Wire.begin(12, 14);  // SDA=D6, SCL=D5
    lcd.init();
    lcd.backlight();
    lcd.print("Incubator Init..");

    dht.begin();

    Blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASS);
    timeClient.begin();

    timer.setInterval(5000L, sensorTick);  // Read every 5 seconds

    lcd.clear();
    lcd.print("System Ready!");
    Serial.println("[INFO] Smart Incubation System started");
}

void loop() {
    Blynk.run();
    timer.run();
    timeClient.update();
}
