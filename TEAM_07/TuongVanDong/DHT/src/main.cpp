#include <Arduino.h>
#include <TM1637Display.h>

/* Fill in information from Blynk Device Info here */

#define BLYNK_TEMPLATE_ID "TMPL6Y2NHV9JM"
#define BLYNK_TEMPLATE_NAME "DHT"
#define BLYNK_AUTH_TOKEN "FC9PmoVb8uYezAJ5UeVKZqyjRa5Syx64"
// Phải để trước khai báo sử dụng thư viện Blynk

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";
char pass[] = "";            

// GPIO Pin Definitions
#define BTN_BLED  23
#define PIN_BLED  21
#define CLK 18
#define DIO 19
#define DHT_PIN 16
#define DHT_TYPE DHT22

// Global Variables
ulong currentMilliseconds = 0;
bool blueButtonON = true;

// Initialize Components
TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

// Function Declarations
bool IsReady(ulong &ulTimer, uint32_t millisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHTSensor();

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BLED, OUTPUT);
    pinMode(BTN_BLED, INPUT_PULLUP);
    display.setBrightness(0x0f);
    dht.begin();
    
    // Connect to WiFi & Blynk
    Serial.print("Connecting to "); Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("WiFi connected");
    
    digitalWrite(PIN_BLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
    Serial.println("== START ==>");
}

void loop() {
    Blynk.run();
    currentMilliseconds = millis();
    uptimeBlynk();
    updateBlueButton();
    readDHTSensor();
}

bool IsReady(ulong &ulTimer, uint32_t millisecond) {
    if (currentMilliseconds - ulTimer < millisecond) return false;
    ulTimer = currentMilliseconds;
    return true;
}

void updateBlueButton() {
    static ulong lastTime = 0;
    static int lastValue = HIGH;
    if (!IsReady(lastTime, 50)) return;
    int v = digitalRead(BTN_BLED);
    if (v == lastValue) return;
    lastValue = v;
    if (v == LOW) return;
    
    blueButtonON = !blueButtonON;
    Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
    digitalWrite(PIN_BLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
    if (!blueButtonON) display.clear();
}

void uptimeBlynk() {
    static ulong lastTime = 0;
    if (!IsReady(lastTime, 1000)) return;
    ulong value = lastTime / 1000;
    Blynk.virtualWrite(V0, value);
    if (blueButtonON) display.showNumberDec(value);
}

void readDHTSensor() {
    static ulong lastTime = 0;
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
    
    if (blueButtonON) display.showNumberDec(temp * 10, false);
}

BLYNK_WRITE(V1) {
    blueButtonON = param.asInt();
    Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
    digitalWrite(PIN_BLED, blueButtonON ? HIGH : LOW);
    if (!blueButtonON) display.clear();
}