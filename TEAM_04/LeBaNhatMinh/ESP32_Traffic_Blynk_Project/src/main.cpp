#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

#define BLYNK_TEMPLATE_ID "TMPL6yean1Ms4"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
#define BLYNK_AUTH_TOKEN "LlapeQu1wAa9Xq5m6Mt1BiYAi_EUzieI"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Định nghĩa chân
#define btnBLED 23
#define pinBLED 21
#define pinRed 27
#define pinYellow 26
#define pinGreen 25
#define CLK 18
#define DIO 19
#define DHT_PIN 16
#define DHT_TYPE DHT22
#define LDR_PIN 12

// Biến toàn cục
ulong currentMiliseconds = 0;
bool blueButtonON = false;
int trafficState = 0;
int countdown = 30;
const int darkThreshold = 1000; // Ngưỡng ánh sáng (0-4095)
bool isDark = false;            // Trạng thái trời tối

// Khởi tạo đối tượng
TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void updateTrafficLights();
void sendSensorData();

void setup()
{
  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(pinRed, OUTPUT);
  pinMode(pinYellow, OUTPUT);
  pinMode(pinGreen, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  dht.begin();
  display.setBrightness(0x0f);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  digitalWrite(pinRed, HIGH);
  digitalWrite(pinYellow, LOW);
  digitalWrite(pinGreen, LOW);

  Serial.println("== START ==>");
}

void loop()
{
  Blynk.run();
  currentMiliseconds = millis();
  updateBlueButton();
  updateTrafficLights();
  sendSensorData();
}

void blinkYellowLed()
{
  static uint32_t lastBlinkTime = 0;
  static bool ledState = LOW;

  if (millis() - lastBlinkTime >= 1000)
  { // Nhấp nháy mỗi 1000ms
    lastBlinkTime = millis();
    ledState = !ledState;
    digitalWrite(pinYellow, ledState);
  }
}

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton()
{
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50))
    return;
  int v = digitalRead(btnBLED);
  if (v == lastValue)
    return;
  lastValue = v;
  if (v == LOW)
    return;

  if (!blueButtonON)
  {
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  }
  else
  {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
  }
}

void updateTrafficLights()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return;

  if (!isDark) // Nếu trời sáng (LDR <= 1000), tắt đèn giao thông
  {
    digitalWrite(pinRed, LOW);
    digitalWrite(pinYellow, LOW);
    digitalWrite(pinGreen, LOW);
    display.clear();
    return;
  }

  countdown--;
  display.showNumberDec(countdown);

  if (countdown <= 0)
  {
    trafficState = (trafficState + 1) % 3;
    switch (trafficState)
    {
    case 0:
      digitalWrite(pinRed, HIGH);
      digitalWrite(pinYellow, LOW);
      digitalWrite(pinGreen, LOW);
      countdown = 30;
      Serial.println("Traffic: RED");
      break;
    case 1:
      digitalWrite(pinRed, LOW);
      digitalWrite(pinYellow, HIGH);
      digitalWrite(pinGreen, LOW);
      countdown = 5;
      Serial.println("Traffic: YELLOW");
      break;
    case 2:
      digitalWrite(pinRed, LOW);
      digitalWrite(pinYellow, LOW);
      digitalWrite(pinGreen, HIGH);
      countdown = 20;
      Serial.println("Traffic: GREEN");
      break;
    }
    Blynk.virtualWrite(V4, trafficState);
  }
}

void sendSensorData()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000))
    return;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int ldrValue = analogRead(LDR_PIN);

  if (isnan(temp) || isnan(hum))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Kiểm tra ngưỡng ánh sáng
  isDark = (ldrValue > darkThreshold);
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V5, ldrValue);
  Blynk.virtualWrite(V6, isDark ? 1 : 0); // Gửi trạng thái trời tối lên Blynk

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(hum);
  Serial.print(" %, LDR: ");
  Serial.print(ldrValue);
  Serial.print(" (Dark: ");
  Serial.print(isDark ? "Yes" : "No");
  Serial.println(")");
}

BLYNK_WRITE(V1)
{
  blueButtonON = param.asInt();
  if (blueButtonON)
  {
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  }
  else
  {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);
  }
}