#define BLYNK_TEMPLATE_ID "TMPL6SUYdvhZm"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "k0AxHgiznQN6zQy3q34ClNTNRRNtyOTf"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define DHTPIN 16
#define DHTTYPE 22
DHT dht(DHTPIN, DHTTYPE);

#define TM1637_CLK 18
#define TM1637_DIO 19
TM1637Display display(TM1637_CLK, TM1637_DIO);

#define BUTTON_PIN 23
#define LED_PIN 21

bool isCounting = true;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 10;
bool buttonPressed = false;

unsigned long sensorLastReadTime = 0;
const unsigned long sensorInterval = 2000;

unsigned long lastUptimeSendTime = 0;
const unsigned long uptimeInterval = 1000;

float temperature = 0.0;
float humidity = 0.0;

void displayTime(unsigned long seconds) {
  int minutes = seconds / 60;
  int secs = seconds % 60;
  int timeToShow = (minutes * 100) + secs;
  display.showNumberDecEx(timeToShow, 0x40, true);
}

BLYNK_WRITE(V0) {
  if (!buttonPressed) {
    isCounting = param.asInt();
    digitalWrite(LED_PIN, isCounting ? HIGH : LOW);
    if (!isCounting) {
      display.clear();
    } else {
      unsigned long uptimeSeconds = millis() / 1000;
      displayTime(uptimeSeconds);
    }
  }
}



void handleButton() {
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW) {
      buttonPressed = true;
      isCounting = !isCounting;
      Blynk.virtualWrite(V1, isCounting);
      digitalWrite(LED_PIN, isCounting ? HIGH : LOW);
      if (isCounting) {
        unsigned long uptimeSeconds = millis() / 1000;
        displayTime(uptimeSeconds);
      } else {
        display.clear();
      }
      buttonPressed = false;
    }
  }
  lastButtonState = reading;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  dht.begin();
  display.setBrightness(0x0f);
  display.showNumberDec(0);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.virtualWrite(V1, isCounting);
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();
  
  handleButton();
  
  if (currentMillis - sensorLastReadTime >= sensorInterval) {
    sensorLastReadTime = currentMillis;
    if (isCounting) {
      temperature = dht.readTemperature();
      humidity = dht.readHumidity();
      if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
      } else {
        Blynk.virtualWrite(V3, temperature);
        Blynk.virtualWrite(V2, humidity);
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C, Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
      }
    }
  }
  
  if (currentMillis - lastUptimeSendTime >= uptimeInterval) {
    lastUptimeSendTime = currentMillis;
    unsigned long uptimeSeconds = currentMillis / 1000;
    Blynk.virtualWrite(V1, uptimeSeconds);
    if (isCounting) {
      displayTime(uptimeSeconds);
    } else {
      display.clear();
    }
  }
}