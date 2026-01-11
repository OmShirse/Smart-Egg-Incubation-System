/*************************************************************
 * Smart Egg Incubation System - Stage-based Control
 * 
 * Hardware Required:
 * - ESP8266 (NodeMCU)
 * - DHT11 Sensor
 * - 2 Relay Modules (for heater and humidifier)
 * - Heating Element
 * - Humidifier/Water pump
 * 
 * Blynk Setup:
 * - Create new template in Blynk IoT
 * - Add Datastreams: V0 (Temperature), V1 (Humidity), V5 (Day Counter)
 * - Add Gauge widgets connected to V0, V1, and V5
 * 
 * Incubation Parameters:
 * - Temperature: 37°C (constant throughout 21 days)
 * - Days 1-18: Humidity 50-55%
 * - Days 19-21: Humidity 65-75% (Lockdown period)
 *************************************************************/

// Include required libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// ============== CONFIGURATION ==============
// Replace with your network credentials
char ssid[] = "Your_WiFi_Name";          // Your WiFi SSID
char pass[] = "Your_WiFi_Password";      // Your WiFi Password

// Replace with your Blynk Auth Token
char auth[] = "Your_Blynk_Auth_Token";

// DHT11 Sensor Configuration
#define DHTPIN D4              // DHT11 connected to GPIO D4
#define DHTTYPE DHT11          // DHT sensor type
DHT dht(DHTPIN, DHTTYPE);

// Relay Pin Configuration
#define HEATER_PIN D5          // Heater relay connected to GPIO D5
#define HUMIDIFIER_PIN D6      // Humidifier relay connected to GPIO D6

// Target Temperature (Constant for all 21 days)
#define TARGET_TEMP 37.0       // Target temperature in Celsius

// Stage-based Humidity Targets
#define HUMIDITY_STAGE1_MIN 50.0   // Days 1-18: Minimum humidity
#define HUMIDITY_STAGE1_MAX 55.0   // Days 1-18: Maximum humidity
#define HUMIDITY_STAGE2_MIN 65.0   // Days 19-21: Minimum humidity (Lockdown)
#define HUMIDITY_STAGE2_MAX 75.0   // Days 19-21: Maximum humidity (Lockdown)

// Control Thresholds (Hysteresis to prevent rapid switching)
#define TEMP_TOLERANCE 0.5     // Temperature tolerance (±0.5°C)
#define HUMID_TOLERANCE 2.0    // Humidity tolerance (±2%)

// Incubation Timeline
#define INCUBATION_STAGE1_DAYS 18  // First stage duration (days)
#define TOTAL_INCUBATION_DAYS 21   // Total incubation period (days)
#define HOURS_PER_DAY 24
#define MILLIS_PER_HOUR 3600000UL  // Milliseconds in one hour

// Timing Configuration
#define READ_INTERVAL 2000     // Read sensor every 2 seconds
#define BLYNK_INTERVAL 5000    // Send to Blynk every 5 seconds

// ============== GLOBAL VARIABLES ==============
float currentTemp = 0;
float currentHumidity = 0;
unsigned long lastReadTime = 0;
unsigned long lastBlynkTime = 0;
bool heaterState = false;
bool humidifierState = false;

// Incubation timing variables
unsigned long incubationStartTime = 0;
int currentDay = 1;
float targetHumidityMin = HUMIDITY_STAGE1_MIN;
float targetHumidityMax = HUMIDITY_STAGE1_MAX;
String currentStage = "Stage 1";

// Virtual Pins for Blynk
#define VPIN_TEMP V0
#define VPIN_HUMIDITY V1
#define VPIN_HEATER_STATUS V2
#define VPIN_HUMIDIFIER_STATUS V3
#define VPIN_SYSTEM_STATUS V4
#define VPIN_DAY_COUNTER V5
#define VPIN_STAGE_INFO V6
#define VPIN_RESET_TIMER V7

// ============== SETUP FUNCTION ==============
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n=================================");
  Serial.println("Smart Egg Incubation System");
  Serial.println("Stage-based Humidity Control");
  Serial.println("=================================\n");
  
  // Initialize relay pins as outputs
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  
  // Initially turn off both relays (HIGH = OFF for active-low relays)
  digitalWrite(HEATER_PIN, HIGH);
  digitalWrite(HUMIDIFIER_PIN, HIGH);
  
  Serial.println("Relay pins initialized");
  
  // Initialize DHT sensor
  dht.begin();
  Serial.println("DHT11 sensor initialized");
  
  // Connect to WiFi and Blynk
  Serial.print("Connecting to WiFi");
  Blynk.begin(auth, ssid, pass);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connected to Blynk!");
  
  // Start incubation timer
  incubationStartTime = millis();
  
  Serial.println("\n=================================");
  Serial.println("INCUBATION STARTED");
  Serial.println("Day 1 of 21");
  Serial.println("Stage 1: Days 1-18");
  Serial.println("Temperature: 37°C");
  Serial.println("Humidity: 50-55%");
  Serial.println("=================================\n");
  
  // Send initial status to Blynk
  Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "System Started - Day 1");
  Blynk.virtualWrite(VPIN_DAY_COUNTER, 1);
  Blynk.virtualWrite(VPIN_STAGE_INFO, "Stage 1: 50-55% RH");
  
  delay(2000); // Wait for sensor to stabilize
}

// ============== MAIN LOOP ==============
void loop() {
  // Run Blynk
  Blynk.run();
  
  // Update incubation day
  updateIncubationDay();
  
  // Read sensor data at specified interval
  if (millis() - lastReadTime >= READ_INTERVAL) {
    lastReadTime = millis();
    readSensorData();
    controlSystem();
  }
  
  // Send data to Blynk at specified interval
  if (millis() - lastBlynkTime >= BLYNK_INTERVAL) {
    lastBlynkTime = millis();
    sendToBlynk();
  }
}

// ============== UPDATE INCUBATION DAY ==============
void updateIncubationDay() {
  unsigned long elapsedTime = millis() - incubationStartTime;
  int calculatedDay = (elapsedTime / MILLIS_PER_HOUR / HOURS_PER_DAY) + 1;
  
  // Check if day has changed
  if (calculatedDay != currentDay && calculatedDay <= TOTAL_INCUBATION_DAYS) {
    currentDay = calculatedDay;
    
    // Update humidity targets based on stage
    if (currentDay <= INCUBATION_STAGE1_DAYS) {
      // Stage 1: Days 1-18
      targetHumidityMin = HUMIDITY_STAGE1_MIN;
      targetHumidityMax = HUMIDITY_STAGE1_MAX;
      currentStage = "Stage 1";
      
      Serial.println("\n*** DAY " + String(currentDay) + " of 21 ***");
      Serial.println("Stage 1: Development Phase");
      Serial.println("Target Humidity: 50-55%");
    } else {
      // Stage 2: Days 19-21 (Lockdown period)
      targetHumidityMin = HUMIDITY_STAGE2_MIN;
      targetHumidityMax = HUMIDITY_STAGE2_MAX;
      currentStage = "Stage 2 (Lockdown)";
      
      Serial.println("\n*** DAY " + String(currentDay) + " of 21 ***");
      Serial.println("Stage 2: LOCKDOWN PERIOD");
      Serial.println("Target Humidity: 65-75%");
      Serial.println("⚠️  DO NOT OPEN INCUBATOR!");
    }
    
    Serial.println("Target Temperature: 37°C");
    Serial.println();
    
    // Send day update to Blynk
    Blynk.virtualWrite(VPIN_DAY_COUNTER, currentDay);
    String stageInfo = currentStage + ": " + String(targetHumidityMin, 0) + "-" + String(targetHumidityMax, 0) + "% RH";
    Blynk.virtualWrite(VPIN_STAGE_INFO, stageInfo);
    
    if (currentDay == 19) {
      Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "⚠️ LOCKDOWN Started - Day 19");
    }
  }
  
  // Check if incubation is complete
  if (calculatedDay > TOTAL_INCUBATION_DAYS) {
    Serial.println("\n🎉 INCUBATION COMPLETE! 🎉");
    Serial.println("21 days completed. Check for hatching!");
    Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "✓ Incubation Complete - Check eggs!");
    delay(60000); // Wait 1 minute before next check
  }
}

// ============== SENSOR READING ==============
void readSensorData() {
  // Read temperature and humidity
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  
  // Check if readings are valid
  if (isnan(temp) || isnan(humid)) {
    Serial.println("ERROR: Failed to read from DHT sensor!");
    Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "⚠️ Sensor Error");
    return;
  }
  
  // Update current values
  currentTemp = temp;
  currentHumidity = humid;
  
  // Display on Serial Monitor
  Serial.println("------ Sensor Reading ------");
  Serial.print("Day: ");
  Serial.print(currentDay);
  Serial.print(" | Stage: ");
  Serial.println(currentStage);
  
  Serial.print("Temperature: ");
  Serial.print(currentTemp, 1);
  Serial.print("°C  |  Target: ");
  Serial.print(TARGET_TEMP, 1);
  Serial.println("°C");
  
  Serial.print("Humidity: ");
  Serial.print(currentHumidity, 1);
  Serial.print("%  |  Target: ");
  Serial.print(targetHumidityMin, 0);
  Serial.print("-");
  Serial.print(targetHumidityMax, 0);
  Serial.println("%");
  Serial.println("----------------------------\n");
}

// ============== CONTROL SYSTEM ==============
void controlSystem() {
  // Temperature Control (Constant 37°C for all 21 days)
  if (currentTemp < (TARGET_TEMP - TEMP_TOLERANCE)) {
    // Too cold - Turn heater ON
    if (!heaterState) {
      digitalWrite(HEATER_PIN, LOW);  // LOW = ON for active-low relay
      heaterState = true;
      Serial.println(">>> HEATER: ON (Temperature too low)");
    }
  } 
  else if (currentTemp > (TARGET_TEMP + TEMP_TOLERANCE)) {
    // Too hot - Turn heater OFF
    if (heaterState) {
      digitalWrite(HEATER_PIN, HIGH);  // HIGH = OFF
      heaterState = false;
      Serial.println(">>> HEATER: OFF (Temperature too high)");
    }
  }
  
  // Humidity Control (Stage-based)
  float targetHumidityMid = (targetHumidityMin + targetHumidityMax) / 2.0;
  
  if (currentHumidity < (targetHumidityMin + HUMID_TOLERANCE)) {
    // Too dry - Turn humidifier ON
    if (!humidifierState) {
      digitalWrite(HUMIDIFIER_PIN, LOW);  // LOW = ON
      humidifierState = true;
      Serial.println(">>> HUMIDIFIER: ON (Humidity too low)");
    }
  } 
  else if (currentHumidity > (targetHumidityMax - HUMID_TOLERANCE)) {
    // Too humid - Turn humidifier OFF
    if (humidifierState) {
      digitalWrite(HUMIDIFIER_PIN, HIGH);  // HIGH = OFF
      humidifierState = false;
      Serial.println(">>> HUMIDIFIER: OFF (Humidity too high)");
    }
  }
  
  // Display current control status
  Serial.print("Control Status - Heater: ");
  Serial.print(heaterState ? "ON" : "OFF");
  Serial.print("  |  Humidifier: ");
  Serial.println(humidifierState ? "ON" : "OFF");
  Serial.println();
}

// ============== SEND DATA TO BLYNK ==============
void sendToBlynk() {
  // Send temperature and humidity to Blynk app
  Blynk.virtualWrite(VPIN_TEMP, currentTemp);
  Blynk.virtualWrite(VPIN_HUMIDITY, currentHumidity);
  
  // Send device status
  Blynk.virtualWrite(VPIN_HEATER_STATUS, heaterState ? "ON" : "OFF");
  Blynk.virtualWrite(VPIN_HUMIDIFIER_STATUS, humidifierState ? "ON" : "OFF");
  
  // Send day counter and stage info
  Blynk.virtualWrite(VPIN_DAY_COUNTER, currentDay);
  String stageInfo = currentStage + ": " + String(targetHumidityMin, 0) + "-" + String(targetHumidityMax, 0) + "% RH";
  Blynk.virtualWrite(VPIN_STAGE_INFO, stageInfo);
  
  // Send system status
  String status = "Day " + String(currentDay) + " | " + String(currentTemp, 1) + "°C | " + String(currentHumidity, 1) + "%";
  Blynk.virtualWrite(VPIN_SYSTEM_STATUS, status);
  
  Serial.println("✓ Data sent to Blynk app");
  Serial.println();
}

// ============== BLYNK BUTTON TO RESET TIMER ==============
BLYNK_WRITE(VPIN_RESET_TIMER) {
  int value = param.asInt();
  
  if (value == 1) {
    // Reset incubation timer
    incubationStartTime = millis();
    currentDay = 1;
    targetHumidityMin = HUMIDITY_STAGE1_MIN;
    targetHumidityMax = HUMIDITY_STAGE1_MAX;
    currentStage = "Stage 1";
    
    Serial.println("\n=================================");
    Serial.println("INCUBATION TIMER RESET");
    Serial.println("Starting Day 1 of 21");
    Serial.println("=================================\n");
    
    Blynk.virtualWrite(VPIN_DAY_COUNTER, 1);
    Blynk.virtualWrite(VPIN_STAGE_INFO, "Stage 1: 50-55% RH");
    Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "Timer Reset - Day 1 Started");
  }
}

// ============== BLYNK CONNECTED EVENT ==============
BLYNK_CONNECTED() {
  Serial.println("Blynk Connected!");
  
  // Send current status
  Blynk.virtualWrite(VPIN_DAY_COUNTER, currentDay);
  String stageInfo = currentStage + ": " + String(targetHumidityMin, 0) + "-" + String(targetHumidityMax, 0) + "% RH";
  Blynk.virtualWrite(VPIN_STAGE_INFO, stageInfo);
  
  // Sync all virtual pins
  Blynk.syncAll();
}

// ============== BLYNK DISCONNECTED EVENT ==============
BLYNK_DISCONNECTED() {
  Serial.println("Blynk Disconnected! Attempting to reconnect...");
}
