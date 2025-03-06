#define BLYNK_TEMPLATE_ID "TMPL6736SgR1f"
#define BLYNK_TEMPLATE_NAME "ESP32 Blynk Traffic DHT Sensor"
#define BLYNK_AUTH_TOKEN "LHhqFG8yUvq9tYWoAGYLTYTbpQaisajA"

#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi Credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Pin Definitions
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define CLK_PIN 18
#define DIO_PIN 19

#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK_PIN, DIO_PIN);

// Global Variables
bool isDisplayOn = true;
int lastButtonState = HIGH;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 50;

unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 2000;

float temperature = 0.0;
float humidity = 0.0;

unsigned long lastUptimeUpdate = 0;
const unsigned long uptimeInterval = 1000;

// Blynk Callback
BLYNK_WRITE(V1) {
    isDisplayOn = param.asInt();
    digitalWrite(LED_PIN, isDisplayOn ? HIGH : LOW);
}

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    
    display.setBrightness(7);
    display.showNumberDec(0, true);
    
    dht.begin();
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("Connected to Blynk");
    
    Blynk.virtualWrite(V1, isDisplayOn);
}

void updateDisplay(unsigned long seconds) {
    int minutes = seconds / 60;
    int secs = seconds % 60;
    int timeDisplay = (minutes * 100) + secs;
    display.showNumberDecEx(timeDisplay, 0x40, true);
}

void loop() {
    Blynk.run();
    unsigned long currentMillis = millis();
    
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState != lastButtonState) {
        lastButtonPress = currentMillis;
    }

    if ((currentMillis - lastButtonPress) > debounceDelay) {
        if (buttonState == LOW && lastButtonState == HIGH) {
            isDisplayOn = !isDisplayOn;
            Blynk.virtualWrite(V1, isDisplayOn);
            Serial.printf("Button Pressed, Display: %s\n", isDisplayOn ? "ON" : "OFF");
        }
    }
    lastButtonState = buttonState;
    
    digitalWrite(LED_PIN, isDisplayOn ? HIGH : LOW);
    
    if (currentMillis - lastSensorRead >= sensorInterval) {
        lastSensorRead = currentMillis;
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        
        if (!isnan(temperature) && !isnan(humidity)) {
            Serial.printf("Temp: %.2f°C, Humidity: %.2f%%\n", temperature, humidity);
            Blynk.virtualWrite(V2, temperature);
            Blynk.virtualWrite(V3, humidity);
        } else {
            Serial.println("Failed to read from DHT22!");
        }
    }
    
    if (currentMillis - lastUptimeUpdate >= uptimeInterval) {
        lastUptimeUpdate = currentMillis;
        unsigned long uptime = currentMillis / 1000;
        Blynk.virtualWrite(V0, uptime);
        
        if (isDisplayOn) {
            updateDisplay(uptime);
        } else {
            display.clear();
        }
    }
}
