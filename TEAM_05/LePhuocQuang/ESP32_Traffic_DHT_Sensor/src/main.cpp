#define BLYNK_TEMPLATE_ID "TMPL6EVpg-k0f"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic DHT Sensor"
#define BLYNK_AUTH_TOKEN "6psMCUWPjGH45jTxvWt_W_iDzUthRxXR"

#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Định nghĩa chân kết nối
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define CLK_PIN 18
#define DIO_PIN 19

#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK_PIN, DIO_PIN);

// Biến toàn cục
bool displayOn = true;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long sensorLastReadTime = 0;
const unsigned long sensorInterval = 2000;

float temperature = 0.0;
float humidity = 0.0;

unsigned long lastUptimeSendTime = 0;
const unsigned long uptimeInterval = 1000;

// Callback Blynk
BLYNK_WRITE(V1) {
  displayOn = param.asInt();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  display.setBrightness(7);
  display.showNumberDec(0, true);
  
  dht.begin();
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected to Blynk");
  
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
  Blynk.virtualWrite(V1, displayOn);
}

void displayTime(unsigned long seconds) {
  int minutes = seconds / 60;
  int secs = seconds % 60;
  int timeToShow = (minutes * 100) + secs; // Hiển thị MM:SS
  display.showNumberDecEx(timeToShow, 0x40, true);
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();
  
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && lastButtonState == HIGH) {
      displayOn = !displayOn;
      Blynk.virtualWrite(V1, displayOn);
      Serial.print("Nút nhấn, displayOn: ");
      Serial.println(displayOn);
    }
  }
  lastButtonState = reading;
  
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
  
  if (currentMillis - sensorLastReadTime >= sensorInterval) {
    sensorLastReadTime = currentMillis;
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Lỗi đọc dữ liệu từ DHT22!");
    } else {
      Serial.print("Nhiệt độ: ");
      Serial.print(temperature);
      Serial.print(" °C, Độ ẩm: ");
      Serial.print(humidity);
      Serial.println(" %");
      
      Blynk.virtualWrite(V2, temperature);
      Blynk.virtualWrite(V3, humidity);
    }
  }
  
  if (currentMillis - lastUptimeSendTime >= uptimeInterval) {
    lastUptimeSendTime = currentMillis;
    unsigned long uptimeSeconds = currentMillis / 1000;
    Blynk.virtualWrite(V0, uptimeSeconds);
    
    if (displayOn) {
      displayTime(uptimeSeconds);
    } else {
      display.clear();
    }
  }
}