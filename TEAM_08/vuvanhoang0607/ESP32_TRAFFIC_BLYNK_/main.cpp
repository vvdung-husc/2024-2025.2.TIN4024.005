// Vũ Văn HoàngHoàng
#define BLYNK_TEMPLATE_ID "TMPL6ELJxae3F"
#define BLYNK_TEMPLATE_NAME "ESP32 TRAFFIC BLYNK"
#define BLYNK_AUTH_TOKEN "50QqXnoc7wRD7xRySxgugt3HFRMeJRp0"

//Nguyễn Đại Tuấn
// #define BLYNK_TEMPLATE_ID "TMPL6HnJaaJvS"
// #define BLYNK_TEMPLATE_NAME "ESP32 LED TM1677"
// #define BLYNK_AUTH_TOKEN "PAcoQCPCPQi-BkPH7GO3-XoRT1pZWLh6"

//Lê Thị Thảo
// #define BLYNK_TEMPLATE_ID "TMPL65viDJjdJ"
// #define BLYNK_TEMPLATE_NAME "ESP 32 LED TM1637"
// #define BLYNK_AUTH_TOKEN "GC-jgJv6Me9vOzZKUTwRDJW1musgY0BG"

//Phạm Mạnh Hoàng
// #define BLYNK_TEMPLATE_ID "TMPL6Jn5gpp6U"
// #define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
// #define BLYNK_AUTH_TOKEN "uSfJFV-kyfHjmrzlESzvYmaKfjsTSGzw"

//Nguyễn Trọng Khôi Nguyên
// #define BLYNK_TEMPLATE_ID "TMPL6umxuTnGE"
// #define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
// #define BLYNK_AUTH_TOKEN "7MKvg2VpruxqGX8cBG4BjB9B3OR5ple0"

#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define BUTTON_PIN 23
#define LED_BLUE 21
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27
#define LDR_PIN 34
#define CLK 18
#define DIO 19
#define DHT_PIN 16
#define DHT_TYPE DHT22

TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

int buttonState = 0, lastButtonState = HIGH;
bool displayOn = true, blueButtonON = true;
int ldrValue = 0, countdown = 10;
unsigned long previousMillis = 0, blinkMillis = 0;
const long interval = 1000, blinkInterval = 500;
bool yellowBlinkState = false;
int ldrThreshold = 1000;

enum TrafficLightState { GREEN, YELLOW, RED };
TrafficLightState lightState = GREEN;

void setupPins() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    display.setBrightness(0x0F);
    display.showNumberDec(countdown);
    Serial.begin(115200);
}

void readLDR() {
    ldrValue = analogRead(LDR_PIN);
}

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
    if (millis() - ulTimer < millisecond) return false;
    ulTimer = millis();
    return true;
}

void handleButtonPress() {
    static unsigned long lastDebounceTime = 0;
    static int lastButtonState = HIGH;
    if (!IsReady(lastDebounceTime, 50)) return;
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == lastButtonState) return;
    lastButtonState = buttonState;
    if (buttonState == LOW) return;
    blueButtonON = !blueButtonON;
    displayOn = !displayOn;
    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
    if (!displayOn) display.clear();
}

void updateCountdown() {
    if (millis() - previousMillis >= interval) {
        previousMillis = millis();
        countdown--;
        if (countdown < 0) {
            lightState = (lightState == GREEN) ? YELLOW : (lightState == YELLOW) ? RED : GREEN;
            countdown = (lightState == YELLOW) ? 3 : 10;
        }
        if (displayOn) display.showNumberDec(countdown);
        else display.clear();
    }
}

void handleTrafficLights() {
    if (ldrValue < ldrThreshold) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, LOW);
        if (millis() - blinkMillis >= blinkInterval) {
            blinkMillis = millis();
            yellowBlinkState = !yellowBlinkState;
            digitalWrite(LED_YELLOW, yellowBlinkState);
        }
        if (displayOn) display.clear();
    } else {
        digitalWrite(LED_GREEN, lightState == GREEN);
        digitalWrite(LED_YELLOW, lightState == YELLOW);
        digitalWrite(LED_RED, lightState == RED);
    }
}

void uptimeBlynk() {
    static unsigned long lastTime = 0;
    if (!IsReady(lastTime, 1000)) return;
    Blynk.virtualWrite(V0, lastTime / 1000);
}

void readDHTSensor() {
    static unsigned long lastTime = 0;
    if (!IsReady(lastTime, 2000)) return;
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();
    if (!isnan(temp) && !isnan(humidity)) {
        Blynk.virtualWrite(V2, temp);
        Blynk.virtualWrite(V3, humidity);
    }
}

BLYNK_WRITE(V1) {
    blueButtonON = param.asInt();
    displayOn = blueButtonON;
    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    if (!displayOn) display.clear();
}

BLYNK_WRITE(V4) {
    ldrThreshold = param.asInt();
}

void setup() {
    setupPins();
    dht.begin();
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
    Blynk.virtualWrite(V4, ldrThreshold);
}

void loop() {
    Blynk.run();
    readLDR();
    handleButtonPress();
    updateCountdown();
    handleTrafficLights();
    uptimeBlynk();
    readDHTSensor();
}