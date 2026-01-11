/*************************************************************
 * Smart Egg Incubation System
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
 * - Add Datastreams: V0 (Temperature), V1 (Humidity)
 * - Add Gauge widgets connected to V0 and V1
 * 
 * Target: 35°C Temperature, 85% Humidity
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
// Get this from Blynk IoT Console
char auth[] = "Your_Blynk_Auth_Token";

// DHT11 Sensor Configuration
#define DHTPIN D4              // DHT11 connected to GPIO D4
#define DHTTYPE DHT11          // DHT sensor type
DHT dht(DHTPIN, DHTTYPE);

// Relay Pin Configuration
#define HEATER_PIN D5          // Heater relay connected to GPIO D5
#define HUMIDIFIER_PIN D6      // Humidifier relay connected to GPIO D6

// Target Values
#define TARGET_TEMP 35.0       // Target temperature in Celsius
#define TARGET_HUMIDITY 85.0   // Target humidity in percentage

// Control Thresholds (Hysteresis to prevent rapid switching)
#define TEMP_TOLERANCE 0.5     // Temperature tolerance (±0.5°C)
#define HUMID_TOLERANCE 2.0    // Humidity tolerance (±2%)

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

// Virtual Pins for Blynk
#define VPIN_TEMP V0
#define VPIN_HUMIDITY V1
#define VPIN_HEATER_STATUS V2
#define VPIN_HUMIDIFIER_STATUS V3
#define VPIN_SYSTEM_STATUS V4

// ============== SETUP FUNCTION ==============
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n=================================");
  Serial.println("Smart Egg Incubation System");
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
  Serial.println("\n=================================\n");
  
  // Send initial status to Blynk
  Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "System Started");
  
  delay(2000); // Wait for sensor to stabilize
}

// ============== MAIN LOOP ==============
void loop() {
  // Run Blynk
  Blynk.run();
  
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

// ============== SENSOR READING ==============
void readSensorData() {
  // Read temperature and humidity
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  
  // Check if readings are valid
  if (isnan(temp) || isnan(humid)) {
    Serial.println("ERROR: Failed to read from DHT sensor!");
    Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "Sensor Error");
    return;
  }
  
  // Update current values
  currentTemp = temp;
  currentHumidity = humid;
  
  // Display on Serial Monitor
  Serial.println("------ Sensor Reading ------");
  Serial.print("Temperature: ");
  Serial.print(currentTemp);
  Serial.print("°C  |  Target: ");
  Serial.print(TARGET_TEMP);
  Serial.println("°C");
  
  Serial.print("Humidity: ");
  Serial.print(currentHumidity);
  Serial.print("%  |  Target: ");
  Serial.print(TARGET_HUMIDITY);
  Serial.println("%");
  Serial.println("----------------------------\n");
}

// ============== CONTROL SYSTEM ==============
void controlSystem() {
  // Temperature Control
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
  
  // Humidity Control
  if (currentHumidity < (TARGET_HUMIDITY - HUMID_TOLERANCE)) {
    // Too dry - Turn humidifier ON
    if (!humidifierState) {
      digitalWrite(HUMIDIFIER_PIN, LOW);  // LOW = ON
      humidifierState = true;
      Serial.println(">>> HUMIDIFIER: ON (Humidity too low)");
    }
  } 
  else if (currentHumidity > (TARGET_HUMIDITY + HUMID_TOLERANCE)) {
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
  
  // Send system status
  String status = "Temp: " + String(currentTemp, 1) + "°C | Humid: " + String(currentHumidity, 1) + "%";
  Blynk.virtualWrite(VPIN_SYSTEM_STATUS, status);
  
  Serial.println("✓ Data sent to Blynk app");
  Serial.println();
}

// ============== BLYNK CONNECTED EVENT ==============
BLYNK_CONNECTED() {
  Serial.println("Blynk Connected!");
  Blynk.virtualWrite(VPIN_SYSTEM_STATUS, "Online");
  
  // Sync all virtual pins
  Blynk.syncAll();
}

// ============== BLYNK DISCONNECTED EVENT ==============
BLYNK_DISCONNECTED() {
  Serial.println("Blynk Disconnected! Attempting to reconnect...");
}
