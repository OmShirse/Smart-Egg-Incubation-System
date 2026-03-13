#define BLYNK_TEMPLATE_ID "TMPL0000000"
#define BLYNK_TEMPLATE_NAME "Egg Incubator"
#define BLYNK_AUTH_TOKEN "Your_Auth_Token"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

/* ---------------- WiFi ---------------- */

char ssid[] = "Your_WiFi";
char pass[] = "Your_Password";

/* ---------------- Sensor ---------------- */

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

/* ---------------- Relay GPIO ---------------- */

#define HEATER_GPIO 14     // D5
#define HUMIDIFIER_GPIO 12 // D6

/* ---------------- ESP8266 GPIO REGISTERS ---------------- */

#define GPIO_OUT_W1TS 0x60000304
#define GPIO_OUT_W1TC 0x60000308
#define GPIO_ENABLE_W1TS 0x60000310

#define REG_WRITE(addr,val) (*((volatile uint32_t *)(addr)) = (val))

/* ---------------- System Targets ---------------- */

#define TARGET_TEMP 37.0
#define TEMP_TOLERANCE 0.5

#define HUM_STAGE1_MIN 50
#define HUM_STAGE1_MAX 55

#define HUM_STAGE2_MIN 65
#define HUM_STAGE2_MAX 75

/* ---------------- Timing ---------------- */

#define READ_INTERVAL 2000
#define BLYNK_INTERVAL 5000

/* ---------------- Incubation ---------------- */

#define STAGE1_DAYS 18
#define TOTAL_DAYS 21

#define HOUR_MS 3600000UL
#define DAY_HOURS 24

/* ---------------- Variables ---------------- */

float currentTemp = 0;
float currentHumidity = 0;

bool heaterState = false;
bool humidifierState = false;

unsigned long lastRead = 0;
unsigned long lastBlynk = 0;

unsigned long incubationStart = 0;
int currentDay = 1;

float targetHumMin = HUM_STAGE1_MIN;
float targetHumMax = HUM_STAGE1_MAX;

/* ---------------- Memory-Mapped Relay Control ---------------- */

void heaterON()
{
  REG_WRITE(GPIO_OUT_W1TC,(1<<HEATER_GPIO));
  heaterState=true;
}

void heaterOFF()
{
  REG_WRITE(GPIO_OUT_W1TS,(1<<HEATER_GPIO));
  heaterState=false;
}

void humidifierON()
{
  REG_WRITE(GPIO_OUT_W1TC,(1<<HUMIDIFIER_GPIO));
  humidifierState=true;
}

void humidifierOFF()
{
  REG_WRITE(GPIO_OUT_W1TS,(1<<HUMIDIFIER_GPIO));
  humidifierState=false;
}

/* ---------------- Setup ---------------- */

void setup()
{

  Serial.begin(115200);

  Serial.println("\nSmart Egg Incubator (Memory-Mapped GPIO)");

  dht.begin();

  /* Enable GPIO output using registers */

  REG_WRITE(GPIO_ENABLE_W1TS,(1<<HEATER_GPIO));
  REG_WRITE(GPIO_ENABLE_W1TS,(1<<HUMIDIFIER_GPIO));

  heaterOFF();
  humidifierOFF();

  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);

  incubationStart = millis();
}

/* ---------------- Loop ---------------- */

void loop()
{

  Blynk.run();

  updateDay();

  if(millis()-lastRead >= READ_INTERVAL)
  {
    lastRead = millis();

    readSensor();

    controlSystem();
  }

  if(millis()-lastBlynk >= BLYNK_INTERVAL)
  {
    lastBlynk = millis();

    sendBlynk();
  }

}

/* ---------------- Update Day ---------------- */

void updateDay()
{

  unsigned long elapsed = millis()-incubationStart;

  int dayCalc = (elapsed/HOUR_MS/DAY_HOURS)+1;

  if(dayCalc!=currentDay && dayCalc<=TOTAL_DAYS)
  {

    currentDay=dayCalc;

    if(currentDay<=STAGE1_DAYS)
    {
      targetHumMin=HUM_STAGE1_MIN;
      targetHumMax=HUM_STAGE1_MAX;
    }
    else
    {
      targetHumMin=HUM_STAGE2_MIN;
      targetHumMax=HUM_STAGE2_MAX;
    }

    Serial.print("Day Updated: ");
    Serial.println(currentDay);

  }

}

/* ---------------- Read Sensor ---------------- */

void readSensor()
{

  float t=dht.readTemperature();
  float h=dht.readHumidity();

  if(isnan(t)||isnan(h))
  {
    Serial.println("Sensor Error");
    return;
  }

  currentTemp=t;
  currentHumidity=h;

  Serial.print("Temp: ");
  Serial.print(t);

  Serial.print(" | Hum: ");
  Serial.println(h);

}

/* ---------------- Control System ---------------- */

void controlSystem()
{

  if(currentTemp < TARGET_TEMP - TEMP_TOLERANCE)
  {
    if(!heaterState) heaterON();
  }

  else if(currentTemp > TARGET_TEMP + TEMP_TOLERANCE)
  {
    if(heaterState) heaterOFF();
  }

  if(currentHumidity < targetHumMin)
  {
    if(!humidifierState) humidifierON();
  }

  else if(currentHumidity > targetHumMax)
  {
    if(humidifierState) humidifierOFF();
  }

}

/* ---------------- Send to Blynk ---------------- */

void sendBlynk()
{

  Blynk.virtualWrite(V0,currentTemp);
  Blynk.virtualWrite(V1,currentHumidity);
  Blynk.virtualWrite(V5,currentDay);

}
