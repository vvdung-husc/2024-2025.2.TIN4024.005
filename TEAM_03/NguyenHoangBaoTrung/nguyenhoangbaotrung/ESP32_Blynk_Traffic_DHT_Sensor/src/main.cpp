#include <Arduino.h>
#include <TM1637Display.h>


#define BLYNK_TEMPLATE_ID "TMPL6dXvmkr1O"
#define BLYNK_TEMPLATE_NAME "ESP32 Blynk Traffic DHT Sensor"
#define BLYNK_AUTH_TOKEN "TXqUgq7kTSIP9W667D72ycrzG47IMK_Z"




#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

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

    Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
    Serial.println(displayOn ? "Display ON" : "Display OFF");

    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);

    if (!displayOn) display.clear();
}

void updateCountdown() {
    if (millis() - previousMillis >= interval) {
        previousMillis = millis();
        countdown--;

        if (countdown < 0) {
            switch (lightState) {
                case GREEN: 
                    lightState = YELLOW; 
                    countdown = 3; 
                    break;
                case YELLOW: 
                    lightState = RED; 
                    countdown = 10; 
                    break;
                case RED: 
                    lightState = GREEN; 
                    countdown = 10; 
                    break;
            }
        }

        if (displayOn) {
            display.showNumberDec(countdown);
        } else {
            display.clear();
        }

        // In thời gian đếm ngược ra Serial Monitor
        Serial.print("⏳ Thời gian đếm ngược: "); 
        Serial.print(countdown); 
        Serial.println(" giây");
    }
}


int ldrThreshold = 1000;
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

        Serial.println("⚠️ Đèn vàng nhấp nháy (chế độ ban đêm)");
    } else {
        digitalWrite(LED_GREEN, lightState == GREEN);
        digitalWrite(LED_YELLOW, lightState == YELLOW);
        digitalWrite(LED_RED, lightState == RED);

        if (lightState == GREEN) {
            Serial.println("🟢 Đèn xanh đang sáng");
        } else if (lightState == YELLOW) {
            Serial.println("🟡 Đèn vàng đang sáng");
        } else if (lightState == RED) {
            Serial.println("🔴 Đèn đỏ đang sáng");
        }
    }
}


void uptimeBlynk() {
    static unsigned long lastTime = 0;
    if (!IsReady(lastTime, 1000)) return;
    unsigned long value = lastTime / 1000;
    Blynk.virtualWrite(V0, value);
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
}

BLYNK_WRITE(V1) {
    blueButtonON = param.asInt();
    displayOn = blueButtonON;
    Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
    Serial.println(displayOn ? "Blynk -> Display ON" : "Blynk -> Display OFF");
    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    if (!displayOn) display.clear();
}

BLYNK_WRITE(V4) {
    ldrThreshold = param.asInt();
    Serial.print("Updated LDR Threshold: ");
    Serial.println(ldrThreshold);
}

void setup() {
    setupPins();
    dht.begin();
    Serial.print("Connecting to "); Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("WiFi connected");
    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
    Blynk.virtualWrite(V4, ldrThreshold);
    Serial.println("== START ==>");
}

void loop() {
    Blynk.run();
    readLDR();
    handleButtonPress();
    updateCountdown();
    handleTrafficLights();
    uptimeBlynk();
    readDHTSensor();
    delay(1000);  // Thêm độ trễ 1 giây để dễ đọc giá trị
}