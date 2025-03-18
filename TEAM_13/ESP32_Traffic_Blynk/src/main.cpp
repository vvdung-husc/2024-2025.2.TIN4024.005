#include <Arduino.h>
#include <TM1637Display.h>
// Blynk Credentials
#define BLYNK_TEMPLATE_ID "TMPL6WdT9pprT"
#define BLYNK_TEMPLATE_NAME "TrafficBlynk"
#define BLYNK_AUTH_TOKEN "Eg73I3k1TN8KpG9DKqM8dqpWl1ShcgpB" 

// Dương Duy Khanh
//#define BLYNK_TEMPLATE_ID "TMPL6rW4m1S4J"
//#define BLYNK_TEMPLATE_NAME "khanh"
//#define BLYNK_AUTH_TOKEN "ejtyvb4NArA0Ek1L4KVNv5pZDDcf81PB"

//Tôn Huyền Kim Khánh
// #define BLYNK_TEMPLATE_ID "TMPL6Mtg-cw9S"
// #define BLYNK_TEMPLATE_NAME "ESP32 LED"
// #define BLYNK_AUTH_TOKEN "jNyrijhCFQGHv4WNa5VCQxQpFPxd_l8B"

// Nguyễn Khánh Linh
// #define BLYNK_TEMPLATE_ID "TMPL6CbYUwJFN"
// #define BLYNK_TEMPLATE_NAME "ESP32 BLYNK LED"
// #define BLYNK_AUTH_TOKEN "C6_0VoXx3puIy5vz3fCEsGo1dBWE7oXF"

//Trần Thị Quỳnh Anh
//#define BLYNK_TEMPLATE_ID "TMPL69Wf7a53k"
//#define BLYNK_TEMPLATE_NAME "Traffic Bynk"
//#define BLYNK_AUTH_TOKEN "Et6D7IURjemqWdyKJxYQmnZQU7Zoa4X5"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
// WiFi Credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// GPIO Definitions
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

// Traffic Light Timings
ulong trafficDurations[] = {10000, 10000, 3000}; // Red: 10s, Green: 10s, Yellow: 3s

// Global Variables
ulong lastChange = 0; 
int trafficState = 0; 
bool isDark = false;
int ldrThreshold = 2000; 
bool isDisplayOn = true;
unsigned long startTime = 0;
long timeLeftInState = 0;  // Time remaining in current state

TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

/* --------------------------------------------------
   Update Traffic Lights
   -------------------------------------------------- */
void updateTrafficLights() {
  ulong currentMillis = millis();

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
  Serial.printf("⏳ Countdown: %d | isDisplayOn: %d | TrafficState: %d\r\n", remainingTime, isDisplayOn, trafficState);
  if (isDisplayOn) {
    display.showNumberDec(remainingTime);
  } else {
    display.clear();
  }
}

/* --------------------------------------------------
   Update Temperature & Humidity
   -------------------------------------------------- */
void updateTemperatureHumidity() {
  static ulong lastUpdate = 0;
  if (millis() - lastUpdate < 2000) return;
  lastUpdate = millis();

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  
  if (!isnan(temp) && !isnan(hum)) {
    Blynk.virtualWrite(V2, temp);
    Blynk.virtualWrite(V3, hum);
    Serial.printf("📡 Temp: %.2f°C | Hum: %.2f%%\r\n", temp, hum);
  } else {
    Serial.println("⚠️ Error reading DHT22!");
  }
}

/* --------------------------------------------------
   Update Light Threshold (LDR)
   -------------------------------------------------- */
void updateLightThreshold() {
  static ulong lastUpdate = 0;
  if (millis() - lastUpdate < 500) return;
  lastUpdate = millis();

  int lightValue = analogRead(LDR_PIN);
  Blynk.virtualWrite(V4, lightValue);
  Serial.printf("💡 Light Level: %d\r\n", lightValue);

  if (lightValue < ldrThreshold) {
    if (!isDark) {
        timeLeftInState = trafficDurations[trafficState] - (millis() - lastChange);
        if (timeLeftInState < 1000) timeLeftInState = 1000;

        isDark = true;
        Serial.println("🌙 It's dark -> Turning on yellow light!");

        isDisplayOn = false;
        display.clear();
        digitalWrite(BLUE_LED, LOW);
        Serial.printf("🔵 BLUE_LED (after LDR check): %d\n", digitalRead(BLUE_LED));

        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);
        Blynk.virtualWrite(V1, isDisplayOn);
    }
  } else {
    if (isDark) {
        isDark = false;
        lastChange = millis() - (trafficDurations[trafficState] - timeLeftInState);
        Serial.println("☀️ It's bright -> Resuming traffic lights!");

        isDisplayOn = true;
        digitalWrite(BLUE_LED, HIGH);
        Blynk.virtualWrite(V1, isDisplayOn);
    }
  }
}

/* --------------------------------------------------
   Button Check
   -------------------------------------------------- */
void checkButton() {
    static int lastButtonState = HIGH;
    static bool buttonPressed = false;
    int buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW && lastButtonState == HIGH) {
        buttonPressed = true;
    } else if (buttonState == HIGH && lastButtonState == LOW && buttonPressed) {
        isDisplayOn = !isDisplayOn;
        buttonPressed = false;

        Serial.printf("🎛 Display: %s\r\n", isDisplayOn ? "ON" : "OFF");

        if (!isDark) {
            digitalWrite(BLUE_LED, isDisplayOn ? HIGH : LOW);
        } else {
            digitalWrite(BLUE_LED, LOW);
        }

        if (!isDisplayOn) display.clear();

        Blynk.virtualWrite(V1, isDisplayOn);
    }

    lastButtonState = buttonState;
}

/* --------------------------------------------------
   Blynk Virtual Pin V1: Toggle Display
   -------------------------------------------------- */
BLYNK_WRITE(V1) {
    isDisplayOn = param.asInt();
    Serial.printf("🎛 Blynk Control - Display: %s\r\n", isDisplayOn ? "ON" : "OFF");

    if (!isDark) {
        digitalWrite(BLUE_LED, isDisplayOn ? HIGH : LOW);
    } else {
        digitalWrite(BLUE_LED, LOW);
    }

    if (isDisplayOn) {
        updateTrafficLights();
    } else {
        display.clear();
    }
}

/* --------------------------------------------------
   Update Uptime
   -------------------------------------------------- */
void updateUptime() {
  unsigned long uptimeSec = (millis() - startTime) / 1000;
  Blynk.virtualWrite(V0, uptimeSec);
}

/* --------------------------------------------------
   Setup
   -------------------------------------------------- */
void setup() {
  Serial.begin(115200);
  
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
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

  digitalWrite(BLUE_LED, isDisplayOn ? HIGH : LOW);
}

/* --------------------------------------------------
   Loop
   -------------------------------------------------- */
void loop() {
  Blynk.run();
  checkButton();
  updateLightThreshold();
  updateTrafficLights();
  updateTemperatureHumidity();
  updateUptime();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("🚨 Lost WiFi connection, reconnecting...");
    WiFi.begin(ssid, pass);
  }
}
