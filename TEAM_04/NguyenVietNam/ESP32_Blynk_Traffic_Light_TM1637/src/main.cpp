#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

#define BLYNK_TEMPLATE_ID "TMPL6mmprv4iy"
#define BLYNK_TEMPLATE_NAME "ESP32 Led M1637"
#define BLYNK_AUTH_TOKEN "Qeuo43PKaDCnRg_bDAPLfE88TSTdtm6E"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED 23
#define pinBLED 21
#define CLK 18
#define DIO 19
#define DHT_PIN 16
#define DHT_TYPE DHT22

ulong currentMiliseconds = 0;
bool blueButtonON = true;

TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void sendSensorData();

void setup()
{
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(0x0f);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  Serial.println("== START ==>");
}

void loop()
{
  Blynk.run();
  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  sendSensorData();
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
    display.clear();
  }
}

void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  if (blueButtonON)
  {
    Serial.print("Displaying uptime: ");
    Serial.println(value);
    display.showNumberDec(value);
  }
}

void sendSensorData()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000))
    return;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);

  if (!blueButtonON)
  {
    display.showNumberDec(temp * 10);
  }

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(hum);
  Serial.println(" %");
}

BLYNK_WRITE(V1)
{
  blueButtonON = param.asInt();
  if (blueButtonON)
  {
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    uptimeBlynk();
  }
  else
  {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    display.clear();
  }
}