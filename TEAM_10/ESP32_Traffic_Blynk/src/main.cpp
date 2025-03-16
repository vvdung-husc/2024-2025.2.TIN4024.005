#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

// NGUYEN VAN HOANG NAM
#define BLYNK_TEMPLATE_ID "TMPL6PR14jYu4"
#define BLYNK_TEMPLATE_NAME "Blynk Traffic DHT Sensor"
#define BLYNK_AUTH_TOKEN "HSryV8aWhysNSYDLOcUNrjPsUcTFXRvr"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Pin definitions
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
int currentState = 0; // 0 = green, 1 = yellow, 2 = red
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

  Serial.println("=== SYSTEM INITIALIZED ===");
}

void loop()
{
  Blynk.run();
  currentMillis = millis();
  updateBlueButton();
  monitorLightLevel(); // Di chuyển lên trước để cập nhật lightLevel
  sendSensorData();

  if (isDark)
    blinkYellowLED();
  else
    runTrafficLights();
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
  if (!IsReady(lastTime, 50))
    return;

  int v = digitalRead(btnBLED);
  if (v == lastValue)
    return;
  lastValue = v;
  if (v == LOW)
    return;

  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  Serial.print("[LED] Blue: ");
  Serial.println(blueButtonON ? "ON" : "OFF");
  if (!blueButtonON)
    display.clear();
}

void sendSensorData()
{
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 2000))
    return;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum))
  {
    Serial.println("[ERROR] Failed to read DHT sensor");
    return;
  }

  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V5, lightLevel);

  Serial.print("[SENSOR] Temp: ");
  Serial.print(temp, 1);
  Serial.print("°C | Humidity: ");
  Serial.print(hum, 1);
  Serial.print("% | Light: ");
  Serial.print(isDark ? "Dark" : "Light");
  Serial.print(" (Level: ");
  Serial.print(lightLevel);
  Serial.print(" | Threshold: ");
  Serial.print(lightThreshold);
  Serial.println(")");
}

void monitorLightLevel()
{
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 500))
    return;

  lightLevel = analogRead(LDR_PIN);
  bool wasDark = isDark;
  isDark = lightLevel < lightThreshold;

  if (wasDark != isDark)
  {
    Blynk.virtualWrite(V6, isDark ? "Dark" : "Light");
  }
}

void blinkYellowLED()
{
  static unsigned long lastBlink = 0;
  static bool state = false;
  static bool logged = false;

  if (currentMillis - lastBlink >= 500)
  {
    state = !state;
    digitalWrite(YELLOW_PIN, state);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    lastBlink = currentMillis;

    if (!logged)
    {
      Serial.println("[TRAFFIC] Yellow: Blinking");
      logged = true;
    }
  }
  if (!isDark)
    logged = false;
}

void runTrafficLights()
{
  unsigned long interval;
  switch (currentState)
  {
  case 0:
    interval = greenTime;
    break;
  case 1:
    interval = yellowTime;
    break;
  case 2:
    interval = redTime;
    break;
  default:
    interval = greenTime;
  }

  if (currentState != lastState)
  {
    previousMillis = currentMillis;
    lastState = currentState;

    digitalWrite(RED_PIN, currentState == 2);
    digitalWrite(YELLOW_PIN, currentState == 1);
    digitalWrite(GREEN_PIN, currentState == 0);

    Serial.print("[TRAFFIC] ");
    Serial.println(currentState == 0 ? "Green" : currentState == 1 ? "Yellow"
                                                                   : "Red");
  }

  remainingTime = interval - (currentMillis - previousMillis);
  if (blueButtonON && remainingTime <= interval)
  {
    int secondsLeft = remainingTime / 1000;
    display.showNumberDec(secondsLeft);
  }

  if (currentMillis - previousMillis >= interval)
  {
    currentState = (currentState + 1) % 3;
  }
}

// Blynk handlers
BLYNK_WRITE(V1)
{
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON);
  if (!blueButtonON)
    display.clear();
  Serial.print("[BLYNK] Blue: ");
  Serial.println(blueButtonON ? "ON" : "OFF");
}

BLYNK_WRITE(V7)
{
  greenTime = param.asInt() * 1000;
  Serial.print("[BLYNK] Green time: ");
  Serial.print(greenTime / 1000);
  Serial.println(" s");
}

BLYNK_WRITE(V8)
{
  yellowTime = param.asInt() * 1000;
  Serial.print("[BLYNK] Yellow time: ");
  Serial.print(yellowTime / 1000);
  Serial.println(" s");
}

BLYNK_WRITE(V9)
{
  redTime = param.asInt() * 1000;
  Serial.print("[BLYNK] Red time: ");
  Serial.print(redTime / 1000);
  Serial.println(" s");
}