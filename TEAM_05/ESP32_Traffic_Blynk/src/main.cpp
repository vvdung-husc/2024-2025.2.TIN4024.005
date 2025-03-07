#include <Arduino.h>
#include <TM1637Display.h>


// Lê Nguyễn Thiện Bình
#define BLYNK_TEMPLATE_ID "TMPL6Z0bWWlkg"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "G6JDL9oJjra3YjGtbqN5JC_gwDQ1FIFN"
//
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
// WiFi Credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// GPIO Pin Definitions
#define BUTTON_PIN 23
#define LED_BLUE 21
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27
#define LDR_PIN 34 // Đổi từ 13 thành 34
#define CLK 18
#define DIO 19
#define DHT_PIN 16
#define DHT_TYPE DHT22

// Initialize Components
TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

// Global Variables
int buttonState = 0, lastButtonState = HIGH;
bool displayOn = true;
int ldrValue = 0, countdown = 10;
unsigned long previousMillis = 0, blinkMillis = 0, lastDebounceTime = 0;
const long interval = 1000, blinkInterval = 500, debounceDelay = 50;
bool yellowBlinkState = false, blueButtonON = true;

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
    Serial.print("LUX Value: ");
    Serial.println(ldrValue);
}

void checkButtonPress() {
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
        displayOn = !displayOn;
        if (!displayOn) display.clear();
        lastDebounceTime = millis();
    }
    lastButtonState = buttonState;
}

void updateCountdown() {
    if (millis() - previousMillis >= interval) {
        previousMillis = millis();
        countdown--;
        if (countdown < 0) {
            switch (lightState) {
                case GREEN: lightState = YELLOW; countdown = 3; break;
                case YELLOW: lightState = RED; countdown = 10; break;
                case RED: lightState = GREEN; countdown = 10; break;
            }
        }
        if (displayOn) display.showNumberDec(countdown);
    }
}

void handleTrafficLights() {
    if (ldrValue < 1000) {
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

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
    if (millis() - ulTimer < millisecond) return false;
    ulTimer = millis();
    return true;
}

void updateBlueButton() {
    static unsigned long lastTime = 0;
    static int lastValue = HIGH;
    if (!IsReady(lastTime, 50)) return;
    int v = digitalRead(BUTTON_PIN);
    if (v == lastValue) return;
    lastValue = v;
    if (v == LOW) return;
    
    blueButtonON = !blueButtonON;
    Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
    if (!blueButtonON) display.clear();
}

void uptimeBlynk() {
    static unsigned long lastTime = 0;
    if (!IsReady(lastTime, 1000)) return;
    unsigned long value = lastTime / 1000;
    Blynk.virtualWrite(V0, value);
    // if (blueButtonON) display.showNumberDec(value);
}

void readDHTSensor() {
    static unsigned long lastTime = 0;
    if (!IsReady(lastTime, 2000)) return;
    
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();
    if (isnan(temp) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    
    Serial.print("Temperature: "); Serial.print(temp);
    Serial.print("°C, Humidity: "); Serial.print(humidity);
    Serial.println("%");
    
    Blynk.virtualWrite(V2, temp);
    Blynk.virtualWrite(V3, humidity);
    
    // if (blueButtonON) display.showNumberDec(temp * 10, false);
}

BLYNK_WRITE(V1) {
    blueButtonON = param.asInt();
    Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    if (!blueButtonON) display.clear();
}

void setup() {
    setupPins();
    dht.begin();
    Serial.print("Connecting to "); Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("WiFi connected");
    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
    Serial.println("== START ==>");
}

void loop() {
    Blynk.run();
    readLDR();
    checkButtonPress();
    updateCountdown();
    handleTrafficLights();
    updateBlueButton();
    uptimeBlynk();
    readDHTSensor();
}
