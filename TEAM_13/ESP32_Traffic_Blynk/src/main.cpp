#include <Arduino.h>
#include <TM1637Display.h>
// Dương Duy Khanh
//#define BLYNK_TEMPLATE_ID "TMPL6rW4m1S4J"
//#define BLYNK_TEMPLATE_NAME "khanh"
//#define BLYNK_AUTH_TOKEN "ejtyvb4NArA0Ek1L4KVNv5pZDDcf81PB"
// Hoàng Thanh Nhã
// #define BLYNK_TEMPLATE_ID "TMPL6WdT9pprT"
// #define BLYNK_TEMPLATE_NAME "TrafficBlynk"
// #define BLYNK_AUTH_TOKEN   "Eg73I3k1TN8KpG9DKqM8dqpWl1ShcgpB"

// Nguyễn Khánh Linh
#define BLYNK_TEMPLATE_ID "TMPL6CbYUwJFN"
#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK LED"
#define BLYNK_AUTH_TOKEN "C6_0VoXx3puIy5vz3fCEsGo1dBWE7oXF"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
// WiFi Credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// GPIO Pins
#define CLK         18
#define DIO         19
#define DHT_PIN     16
#define DHT_TYPE    DHT22  
#define LDR_PIN     34
#define RED_LED     27
#define YELLOW_LED  26
#define GREEN_LED   25
#define BUTTON_PIN  23
#define BLUE_LED    21  

// Traffic light durations (ms)
ulong trafficDurations[] = {10000, 10000, 3000}; // Red 10s, Green 10s, Yellow 3s

// Global variables
ulong lastChange = 0;
int   trafficState = 0;
bool  isDark = false;
int   ldrThreshold = 2000;
bool  isDisplayOn = true;
long  timeLeftInState = 0;

TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

// System Uptime
unsigned long startTime = 0;

// Function Declarations
void updateTrafficLights(unsigned long);
void updateTemperatureHumidity(unsigned long);
void updateLightThreshold(unsigned long);
void checkButton(unsigned long);
void checkWiFi(unsigned long);
void updateUptime(unsigned long);

/* --------------------------------------------------
   Setup
   -------------------------------------------------- */
void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BLUE_LED, OUTPUT);

  display.setBrightness(0x0f);
  display.clear();
  dht.begin();
  startTime = millis();

  Serial.printf("🔗 Connecting to WiFi: %s\r\n", ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi Connected!");
  } else {
    Serial.println("❌ WiFi Connection Failed!");
  }

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(BLUE_LED, isDisplayOn ? HIGH : LOW);
}

/* --------------------------------------------------
   Loop
   -------------------------------------------------- */
void loop() {
  unsigned long currentMillis = millis();

  Blynk.run();
  checkButton(currentMillis);
  updateLightThreshold(currentMillis);
  updateTrafficLights(currentMillis);
  updateTemperatureHumidity(currentMillis);
  updateUptime(currentMillis);
  checkWiFi(currentMillis);
}

/* --------------------------------------------------
   Traffic Light System
   -------------------------------------------------- */
void updateTrafficLights(unsigned long currentMillis) {
  static int lastRemainingTime = -1;

  if (isDark) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    display.clear();
    return;
  }

  if (currentMillis - lastChange >= trafficDurations[trafficState]) {
    lastChange = currentMillis;
    trafficState = (trafficState + 1) % 3;
    digitalWrite(RED_LED, trafficState == 0);
    digitalWrite(GREEN_LED, trafficState == 1);
    digitalWrite(YELLOW_LED, trafficState == 2);
  }

  int remainingTime = (trafficDurations[trafficState] - (currentMillis - lastChange)) / 1000;
  if (isDisplayOn && remainingTime != lastRemainingTime) {
    display.showNumberDec(remainingTime);
    lastRemainingTime = remainingTime;
  }
}

/* --------------------------------------------------
   Light Sensor (LDR) Handling
   -------------------------------------------------- */
void updateLightThreshold(unsigned long currentMillis) {
  static ulong lastUpdate = 0;
  if (currentMillis - lastUpdate < 500) return;
  lastUpdate = currentMillis;

  int lightValue = analogRead(LDR_PIN);
  Blynk.virtualWrite(V4, lightValue);

  if (lightValue < ldrThreshold) {
    if (!isDark) {
      long leftover = trafficDurations[trafficState] - (currentMillis - lastChange);
      timeLeftInState = leftover > 0 ? leftover : 1000;
      isDark = true;
      Serial.println("🌙 Night Mode -> Yellow Light!");
    }
  } else {
    if (isDark) {
      isDark = false;
      lastChange = currentMillis - (trafficDurations[trafficState] - timeLeftInState);
      Serial.println("☀️ Day Mode -> Traffic Light Resumed!");
    }
  }
}

/* --------------------------------------------------
   Temperature & Humidity Sensor (DHT22)
   -------------------------------------------------- */
void updateTemperatureHumidity(unsigned long currentMillis) {
  static float lastTemp = -100, lastHum = -100;
  static ulong lastUpdate = 0;
  if (currentMillis - lastUpdate < 1000) return;
  lastUpdate = currentMillis;

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    if (temp != lastTemp || hum != lastHum) {
      Serial.printf("📡 Temp: %.2f°C | Hum: %.2f%%\r\n", temp, hum);
      lastTemp = temp;
      lastHum = hum;
    }
    Blynk.virtualWrite(V2, temp);
    Blynk.virtualWrite(V3, hum);
  } else {
    Serial.println("⚠️ Failed to read DHT22!");
  }
}

/* --------------------------------------------------
   Button Handling (Toggle Display)
   -------------------------------------------------- */
void checkButton(unsigned long currentMillis) {
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;

  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    if (currentMillis - lastDebounceTime > 50) {
      isDisplayOn = !isDisplayOn;
      Serial.printf("🎛 Display: %s\r\n", isDisplayOn ? "ON" : "OFF");
      digitalWrite(BLUE_LED, isDisplayOn ? HIGH : LOW);
      Blynk.virtualWrite(V1, isDisplayOn);
      if (!isDisplayOn) display.clear();
      lastDebounceTime = currentMillis;
    }
  }
  lastButtonState = buttonState;
}

/* --------------------------------------------------
   WiFi Reconnection Handling
   -------------------------------------------------- */
void checkWiFi(unsigned long currentMillis) {
  static unsigned long lastReconnectAttempt = 0;

  if (WiFi.status() != WL_CONNECTED) {
    if (currentMillis - lastReconnectAttempt >= 5000) {
      Serial.println("🚨 Lost WiFi, Reconnecting...");
      WiFi.begin(ssid, pass);
      lastReconnectAttempt = currentMillis;
    }
  }
}

/* --------------------------------------------------
   System Uptime
   -------------------------------------------------- */
void updateUptime(unsigned long currentMillis) {
  unsigned long uptimeSec = (currentMillis - startTime) / 1000;
  Blynk.virtualWrite(V0, uptimeSec);
}