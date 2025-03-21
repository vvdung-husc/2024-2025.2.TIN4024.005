#define BLYNK_TEMPLATE_ID "TMPL61N9RxiUa"
#define BLYNK_TEMPLATE_NAME "ESP2TrafficBlynk"
#define BLYNK_AUTH_TOKEN "bOrtqnRWEYh74xb-ZXqVs4mVfhFkB4w9"

#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Định nghĩa chân
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define DHT_TYPE DHT22
#define CLK_PIN 18
#define DIO_PIN 19
#define LIGHT_SENSOR_PIN 34
#define RED_PIN 27
#define YELLOW_PIN 26
#define GREEN_PIN 25

// Cảm biến & hiển thị
DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK_PIN, DIO_PIN);

// Biến toàn cục
bool ledState = false;
int lightThreshold = 1000;
bool isDark = false;

// Đèn giao thông
int currentState = 0;
unsigned long previousMillis = 0;
const unsigned long greenTime = 5000;
const unsigned long yellowTime = 3000;
const unsigned long redTime = 5000;

// 🔹 KHAI BÁO TRƯỚC CÁC HÀM (Prototype)
void sendSensorData(unsigned long currentMillis);
void monitorLightLevel(unsigned long currentMillis);
void blinkYellowLED(unsigned long currentMillis);
void runTrafficLights(unsigned long currentMillis);

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(RED_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(LIGHT_SENSOR_PIN, INPUT);

    display.setBrightness(7);
    dht.begin();

    // Kết nối Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("Connected to Blynk");
}

void loop() {
    Blynk.run();
    unsigned long currentMillis = millis();

    sendSensorData(currentMillis);
    monitorLightLevel(currentMillis);

    if (isDark) {
        blinkYellowLED(currentMillis);
    } else {
        runTrafficLights(currentMillis);
    }
}

void sendSensorData(unsigned long currentMillis) {
    static unsigned long lastTime = 0;
    if (currentMillis - lastTime < 2000) return;
    lastTime = currentMillis;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int lightLevel = analogRead(LIGHT_SENSOR_PIN);

    if (!isnan(temp) && !isnan(hum)) {
        Blynk.virtualWrite(V0, temp);
        Blynk.virtualWrite(V2, hum);
        Blynk.virtualWrite(V4, lightLevel);
        Serial.printf("Temp: %.1f C, Humidity: %.1f%%, Light: %d\n", temp, hum, lightLevel);
    }
}

void monitorLightLevel(unsigned long currentMillis) {
    static unsigned long lastCheck = 0;
    if (currentMillis - lastCheck < 500) return;
    lastCheck = currentMillis;

    int lightLevel = analogRead(LIGHT_SENSOR_PIN);
    isDark = (lightLevel < lightThreshold);
}

void blinkYellowLED(unsigned long currentMillis) {
    static unsigned long lastBlink = 0;
    static bool state = false;

    if (currentMillis - lastBlink >= 500) {
        state = !state;
        digitalWrite(YELLOW_PIN, state);
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);
        lastBlink = currentMillis;
    }
}

void runTrafficLights(unsigned long currentMillis) {
    unsigned long interval;
    switch (currentState) {
        case 0: interval = greenTime; break;
        case 1: interval = yellowTime; break;
        case 2: interval = redTime; break;
    }

    if (currentMillis - previousMillis >= interval) {
        currentState = (currentState + 1) % 3;
        previousMillis = currentMillis;
    }

    digitalWrite(GREEN_PIN, currentState == 0);
    digitalWrite(YELLOW_PIN, currentState == 1);
    digitalWrite(RED_PIN, currentState == 2);

    display.showNumberDec((interval - (currentMillis - previousMillis)) / 1000);
}

BLYNK_WRITE(V1) {
    ledState = param.asInt();
    digitalWrite(LED_PIN, ledState);
}

BLYNK_WRITE(V4) {
    lightThreshold = param.asInt();
}