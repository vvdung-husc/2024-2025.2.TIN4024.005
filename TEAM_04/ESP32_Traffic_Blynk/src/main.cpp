#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#define BLYNK_TEMPLATE_ID "TMPL6YeCq4kLS"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
#define BLYNK_AUTH_TOKEN "yHBEJfZ9T7gfaPf_iNwT6ccPgvYH_WGT"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Pin definitions
char ssid[] = "Wokwi-GUEST";
char pass[] = "";
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
int lightThreshold = 1000;  // Ngưỡng: <1000 là tối, >=1000 là sáng (dựa trên bảng)
int lightLevel = 0;
int lastLightLevel = -1;
bool isDark = false;

// Traffic light variables
int currentState = 0;  // 0 = green, 1 = yellow, 2 = red
int lastState = -1;
unsigned long previousMillis = 0;
// Thời gian thực tế (ms)
unsigned long greenTime = 5000;  // 30 giây
unsigned long yellowTime = 3000;  // 3 giây
unsigned long redTime = 5000;    // 40 giây
unsigned long remainingTime = 0;

// Function prototypes
bool IsReady(unsigned long &ulTimer, uint32_t milisecond);
void updateBlueButton();
void sendSensorData();
void monitorLightLevel();
void runTrafficLights();
void blinkYellowLED();

void setup()
{
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  
  dht.begin();
  display.setBrightness(0x0f);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  Blynk.virtualWrite(V4, lightThreshold); // Gửi ngưỡng mặc định
  
  Serial.println("=== SYSTEM INITIALIZED ===");
}

void loop()
{
  Blynk.run();
  currentMillis = millis();
  updateBlueButton();
  sendSensorData();
  monitorLightLevel();
  
  if (isDark) {
    blinkYellowLED();
    display.clear();
  } else {
    runTrafficLights();
  }
}

bool IsReady(unsigned long &ulTimer, uint32_t milisecond)
{
  if (currentMillis - ulTimer < milisecond)
    return false;
  ulTimer = currentMillis;
  return true;
}

void updateBlueButton()
{
  static unsigned long lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;
  
  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  
  Serial.println(blueButtonON ? "[STATUS] Blue LED: ON" : "[STATUS] Blue LED: OFF");
  if (!blueButtonON) display.clear();
}

void sendSensorData()
{
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;
  
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  lightLevel = analogRead(LDR_PIN);
  
  if (isnan(temp) || isnan(hum)) {
    Serial.println("[ERROR] Failed to read DHT sensor");
    return;
  }
  
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V5, lightLevel);
  
  // Chỉ in khi thay đổi đáng kể hoặc lần đầu
  if (abs(lightLevel - lastLightLevel) > 50 || lastLightLevel == -1) {
    Serial.printf("[SENSOR] Temp: %.1f°C | Humidity: %.1f%% | Light: %d (Threshold: %d)\n", 
                  temp, hum, lightLevel, lightThreshold);
    lastLightLevel = lightLevel;
  }
}

void monitorLightLevel()
{
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 500)) return;
  
  bool wasDark = isDark;
  isDark = lightLevel < lightThreshold;
  
  // Phân loại điều kiện sáng/tối dựa trên bảng
  if (wasDark != isDark) {
    if (isDark) {
      if (lightLevel <= 853) Serial.println("[ENV] Twilight or darker detected");
      else if (lightLevel <= 985) Serial.println("[ENV] Deep twilight detected");
      else if (lightLevel <= 1016) Serial.println("[ENV] Full moon detected");
    } else {
      if (lightLevel >= 170 && lightLevel < 281) Serial.println("[ENV] Overcast day detected");
      else if (lightLevel >= 281 && lightLevel < 511) Serial.println("[ENV] Office lighting detected");
      else if (lightLevel >= 511 && lightLevel < 633) Serial.println("[ENV] Stairway lighting detected");
      else if (lightLevel >= 633 && lightLevel < 853) Serial.println("[ENV] Computer monitor lighting detected");
      else if (lightLevel >= 853) Serial.println("[ENV] Bright daylight or sunlight detected");
    }
    Blynk.virtualWrite(V6, isDark ? "Night" : "Day");
  }
}

void blinkYellowLED()
{
  static unsigned long lastBlink = 0;
  static bool state = false;
  
  if (currentMillis - lastBlink >= 500) {
    state = !state;
    digitalWrite(YELLOW_PIN, state);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    lastBlink = currentMillis;
    
    static bool firstBlink = true;
    if (firstBlink && state) {
      Serial.println("[TRAFFIC] Yellow blinking started");
      firstBlink = false;
    }
    if (!isDark) firstBlink = true;
  }
}

void runTrafficLights()
{
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
    
    Serial.printf("[TRAFFIC] %s light ON\n", 
      currentState == 0 ? "Green" : 
      currentState == 1 ? "Yellow" : "Red");
  }
  
  remainingTime = interval - (currentMillis - previousMillis);
  if (blueButtonON && remainingTime <= interval) {
    int secondsLeft = remainingTime / 1000;
    display.showNumberDec(secondsLeft);
    
    static int lastSeconds = -1;
    if (secondsLeft <= 5 && secondsLeft != lastSeconds && secondsLeft > 0) {
      Serial.printf("[TRAFFIC] %d seconds remaining\n", secondsLeft);
      lastSeconds = secondsLeft;
    }
  }
  
  if (currentMillis - previousMillis >= interval) {
    currentState = (currentState + 1) % 3;
  }
}

// Blynk handlers
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON);
  if (!blueButtonON) display.clear();
  Serial.println(blueButtonON ? "[BLYNK] Blue LED ON" : "[BLYNK] Blue LED OFF");
}

BLYNK_WRITE(V4) {
  // Giữ ngưỡng cố định dựa trên bảng, không cho thay đổi
  Serial.println("[BLYNK] Light threshold is fixed at 1000 (based on data table)");
}

BLYNK_WRITE(V7) {
  greenTime = param.asInt() * 1000;
  Serial.printf("[BLYNK] Green time updated to: %d s\n", greenTime / 1000);
}

BLYNK_WRITE(V8) {
  yellowTime = param.asInt() * 1000;
  Serial.printf("[BLYNK] Yellow time updated to: %d s\n", yellowTime / 1000);
}

BLYNK_WRITE(V9) {
  redTime = param.asInt() * 1000;
  Serial.printf("[BLYNK] Red time updated to: %d s\n", redTime / 1000);
}