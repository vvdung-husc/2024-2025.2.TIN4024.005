#include <Arduino.h>
#include <TM1637Display.h>


// // Lê Nguyễn Thiện Bình
// #define BLYNK_TEMPLATE_ID "TMPL6Z0bWWlkg"
// #define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
// #define BLYNK_AUTH_TOKEN "G6JDL9oJjra3YjGtbqN5JC_gwDQ1FIFN"

// // Mai Đức Đạt
// #define BLYNK_TEMPLATE_ID "TMPL6Thd77apQ"
// #define BLYNK_TEMPLATE_NAME "ESP32 TRAFFIC BLYNK"
// #define BLYNK_AUTH_TOKEN "9PW9tGbbmOVG3WJY7FcFoF302LeuGsWH" 

// // Lê Phước Quang
// #define BLYNK_TEMPLATE_ID "TMPL6FmNRjyo2"
// #define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
// #define BLYNK_AUTH_TOKEN "uztsql_P-9s17msAlM8Ln5FyvSIVK06c" 

//Lê Quang Khải
#define BLYNK_TEMPLATE_ID "TMPL6hBcvM0Ga"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
#define BLYNK_AUTH_TOKEN "QOKEkCt0KbhBmTKDR8AA3VtEVojKBZCV"
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


bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
    if (millis() - ulTimer < millisecond) return false;
    ulTimer = millis();
    return true;
}
void handleButtonPress() {
    static unsigned long lastDebounceTime = 0;
    static int lastButtonState = HIGH;
    
    if (!IsReady(lastDebounceTime, 50)) return;  // Chống dội phím (50ms)

    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == lastButtonState) return;  // Nếu không thay đổi thì thoát

    lastButtonState = buttonState;
    if (buttonState == LOW) return;  // Chỉ xử lý khi NHẢ NÚT (HIGH)

    // Khi nút được NHẢ ra -> Đảo trạng thái cả màn hình và LED xanh
    blueButtonON = !blueButtonON;
    displayOn = !displayOn;

    Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
    Serial.println(displayOn ? "Display ON" : "Display OFF");

    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);

    if (!displayOn) display.clear();  // Tắt màn hình khi cần
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
        if (displayOn) {  // Chỉ hiển thị khi displayOn == true
            display.showNumberDec(countdown);
        } else {
            display.clear();
        }
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
    displayOn = blueButtonON;  // Đồng bộ trạng thái màn hình với LED

    Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
    Serial.println(displayOn ? "Blynk -> Display ON" : "Blynk -> Display OFF");

    digitalWrite(LED_BLUE, blueButtonON ? HIGH : LOW);

    if (!displayOn) display.clear();  // Nếu tắt LED xanh, tắt luôn màn hình
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
    handleButtonPress();  // Gọi hàm gộp mới
    updateCountdown();
    handleTrafficLights();
    uptimeBlynk();
    readDHTSensor();
}