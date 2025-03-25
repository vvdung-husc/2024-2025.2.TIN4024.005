#include <Arduino.h>
#include "utils.h" 
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>

// Blynk Information (Le Van Minh Toan)
#define BLYNK_TEMPLATE_ID "TMPL6eDvyBqz2"
#define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
#define BLYNK_AUTH_TOKEN "RP-9PNRNu-xTRTCNClk-LGQNvj6r77_a"

// WiFi Information
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

// Telegram Information (Le Van Minh Toan)
#define BOT_TOKEN "7441033779:AAEOc1s6Gc_TVWQDAtlih7Zdoei2oiXSCD8"
#define GROUP_ID "-4621614489" 

// Pin Definitions
#define LED_GREEN 15 // D8
#define LED_YELLOW 2  // D4
#define LED_RED 5    // D1
#define DHT_PIN 16  // D0
#define DHT_TYPE DHT11
#define OLED_SDA 13 // D7
#define OLED_SCL 12 // D6

// Object Declarations
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Global Variables
float humidity = 0.0;
float temperature = 0.0;
bool buttonPressed = false;
bool trafficOn = true;
unsigned long startTime;

bool DisplayTimeInterval(uint waitTime = 5000) {
    static unsigned long previousTime = 0;
    static bool completed = false;
    if (completed) return true;
    if (!IsReady(previousTime, waitTime)) return false;
    completed = true;
    return completed;
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    dht.begin();
    Wire.begin(OLED_SDA, OLED_SCL);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    Serial.println(WiFi.status() == WL_CONNECTED ? "✅ WiFi connected successfully!" : "❌ WiFi connection failed!");

    Blynk.virtualWrite(V3, buttonPressed);
    oled.begin();
    oled.clearBuffer();

    oled.setFont(u8g2_font_unifont_t_vietnamese1);
    oled.drawUTF8(0, 14, "University of Science");
    oled.drawUTF8(0, 28, "Faculty of Information Technology");
    oled.drawUTF8(0, 42, "IoT System Programming");
    oled.sendBuffer();

    client.setInsecure();
    startTime = millis();
    randomSeed(analogRead(0));
}

void controlTrafficLights() {
    static unsigned long previousTime = 0;
    if (!trafficOn || buttonPressed) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);
        return;
    }

    if (!IsReady(previousTime, 1000)) return;
    
    if (temperature < 15) {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_GREEN, LOW);
    } else if (temperature >= 15 && temperature <= 30) {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_GREEN, HIGH);
    } else {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(LED_GREEN, LOW);
    }
}

void updateDHT() {
    static unsigned long previousTime = 0;
    if (!IsReady(previousTime, 2000)) return;

    float newHumidity = random(0, 10001) / 100.0;
    float newTemperature = random(-4000, 8001) / 100.0;

    if (isnan(newHumidity) || isnan(newTemperature)) {
        Serial.println("Random number generation error!");
        return;
    }

    bool shouldUpdateDisplay = false;
    if (temperature != newTemperature) {
        shouldUpdateDisplay = true;
        temperature = newTemperature;
        Serial.printf("Temperature: %.2f °C\n", temperature);
    }

    if (humidity != newHumidity) {
        shouldUpdateDisplay = true;
        humidity = newHumidity;
        Serial.printf("Humidity: %.2f%%\n", humidity);
    }

    if (shouldUpdateDisplay) {
        oled.clearBuffer();
        oled.setFont(u8g2_font_unifont_t_vietnamese2);
        String tempStr = StringFormat("Temperature: %.2f °C", temperature);
        oled.drawUTF8(0, 14, tempStr.c_str());
        String humidityStr = StringFormat("Humidity: %.2f%%", humidity);
        oled.drawUTF8(0, 42, humidityStr.c_str());
        oled.sendBuffer();
    }

    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);
}

void blinkYellowLight() {
    static bool yellowLightState = false;
    static unsigned long previousTime = 0;
    if (IsReady(previousTime, 500)) {
        yellowLightState = !yellowLightState;
        digitalWrite(LED_YELLOW, yellowLightState);
    }
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
}

void sendTimeToBlynk() {
    static unsigned long previousTime = 0;
    if (!IsReady(previousTime, 1000)) return;
    unsigned long elapsed = (millis() - startTime) / 1000;
    Blynk.virtualWrite(V0, elapsed);
}

BLYNK_WRITE(V3) {
    buttonPressed = param.asInt();
}

void loop() {
    Blynk.run();
    if (!DisplayTimeInterval()) return;
    controlTrafficLights();
    updateDHT();
    sendTimeToBlynk();
    if (buttonPressed) {
        blinkYellowLight();
    }
}


