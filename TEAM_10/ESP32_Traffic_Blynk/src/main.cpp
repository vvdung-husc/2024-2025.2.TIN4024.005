#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

// NGUYEN VAN HOANG NAM
#define BLYNK_TEMPLATE_ID "TMPL6PR14jYu4"
#define BLYNK_TEMPLATE_NAME "Blynk Traffic DHT Sensor"
#define BLYNK_AUTH_TOKEN "HSryV8aWhysNSYDLOcUNrjPsUcTFXRvr"
// PHAN THANH TOAN
// #define BLYNK_TEMPLATE_ID "TMPL6NL5ny714"
// #define BLYNK_TEMPLATE_NAME "TrafficBlynk"
// #define BLYNK_AUTH_TOKEN "uENQDUAhyfsOW5K5pqkw1vxl2WiVek7R"
// LE VAN MINH TOAN
// #define BLYNK_TEMPLATE_ID "TMPL6eDvyBqz2"
// #define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
// #define BLYNK_AUTH_TOKEN "RP-9PNRNu-xTRTCNClk-LGQNvj6r77_a"


#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClient.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Pin Definitions
#define btnBLED 23
#define pinBLED 21
#define CLK 18
#define DIO 19
#define DHT_PIN 16
#define DHT_TYPE DHT22
#define LDR_PIN 34
#define RED_PIN 27
#define YELLOW_PIN 26
#define GREEN_PIN 25

// Variables
unsigned long currentMillis = 0;
bool blueButtonON = true;
TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);
int lightThreshold = 1000;
int lightLevel = 0;
bool isDark = false;

// Traffic light variables
int currentState = 0;
int lastState = -1;
unsigned long previousMillis = 0;
unsigned long greenTime = 5000;
unsigned long yellowTime = 3000;
unsigned long redTime = 5000;
unsigned long remainingTime = 0;

// Function prototypes
bool IsReady(unsigned long &ulTimer, uint32_t milisecond);
void updateBlueButton();
void sendSensorData();
void monitorLightLevel();
void runTrafficLights();
void blinkYellowLED();

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMillis - ulTimer < milisecond) return false;
  ulTimer = currentMillis;
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  dht.begin();
  display.setBrightness(0x0f);
  lastState = -1; // Khởi tạo lastState

  // Kiểm tra kết nối WiFi/Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  if (!Blynk.connected()) {
    Serial.println("Failed to connect to Blynk!");
  }

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V0, blueButtonON);

  // In giá trị ánh sáng ban đầu để điều chỉnh threshold
  lightLevel = analogRead(LDR_PIN);
  Serial.print("Initial light level: ");
  Serial.println(lightLevel);

  Serial.println("=== SYSTEM INITIALIZED ===");
}

void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  }
  currentMillis = millis();
  updateBlueButton();
  monitorLightLevel();
  sendSensorData();

  if (isDark) {
    blinkYellowLED();
  } else {
    runTrafficLights();
  }
}

void updateBlueButton() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 50)) return;

  int state = digitalRead(btnBLED);
  if (state == LOW) {
    blueButtonON = !blueButtonON;
    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V0, blueButtonON);
    if (!blueButtonON) display.clear();
  }
}

BLYNK_WRITE(V0) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (!blueButtonON) display.clear();
}

//DHT
void sendSensorData() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (!isnan(temp) && !isnan(hum)) {
    Blynk.virtualWrite(V5, temp); 
    Blynk.virtualWrite(V6, hum);  
  } else {
    Serial.println("Failed to read from DHT sensor!");
    Blynk.virtualWrite(V5, 0);
    Blynk.virtualWrite(V6, 0);
  }
}

void monitorLightLevel() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 500)) return;

  lightLevel = analogRead(LDR_PIN);
  isDark = lightLevel < lightThreshold;
}

// Điều chỉnh ngưỡng ánh sáng
BLYNK_WRITE(V4) {
  int adjustment = param.asInt();
  lightThreshold += adjustment;
  Blynk.virtualWrite(V11, lightThreshold);
}

BLYNK_WRITE(V1) {
  greenTime = param.asInt() * 1000;
}

BLYNK_WRITE(V2) {
  yellowTime = param.asInt() * 1000;
}

BLYNK_WRITE(V3) {
  redTime = param.asInt() * 1000;
}

void blinkYellowLED() {
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

void runTrafficLights() {
  unsigned long interval;
  switch (currentState) {
    case 0: interval = greenTime; break;
    case 1: interval = yellowTime; break;
    case 2: interval = redTime; break;
    default: interval = greenTime;
  }

  if (currentState != lastState) {
    previousMillis = currentMillis;
    lastState = currentState;
    digitalWrite(RED_PIN, currentState == 2);
    digitalWrite(YELLOW_PIN, currentState == 1);
    digitalWrite(GREEN_PIN, currentState == 0);
  }

  remainingTime = interval - (currentMillis - previousMillis);
  if (blueButtonON && !isDark && remainingTime <= interval) {
    int secondsLeft = remainingTime / 1000;
    display.showNumberDec(secondsLeft);
  }

  if (currentMillis - previousMillis >= interval) {
    currentState = (currentState + 1) % 3;
  }
}