//Trần Kiêm Hiếu
#define BLYNK_TEMPLATE_ID "TMPL6SUYdvhZm"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "k0AxHgiznQN6zQy3q34ClNTNRRNtyOTf"

//Ngô Viết Hy
// #define BLYNK_TEMPLATE_ID "TMPL6LzI2uXDN"
// #define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
// #define BLYNK_AUTH_TOKEN "RatdE4oZnHcz2sy4tOWfnSHyWAV8loZp"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Cảm biến DHT22
#define DHTPIN 16
#define DHTTYPE 22
DHT dht(DHTPIN, DHTTYPE);

// TM1637 Display
#define TM1637_CLK 18
#define TM1637_DIO 19
TM1637Display display(TM1637_CLK, TM1637_DIO);

// Định nghĩa chân
#define BUTTON_PIN 23
#define LED_PIN 21
#define LED_BLUE 21
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27
#define LDR_PIN 34

// Trạng thái
bool displayOn = true;
bool buttonPressed = false;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 10;

unsigned long sensorLastReadTime = 0;
const unsigned long sensorInterval = 2000;

unsigned long lastUptimeSendTime = 0;
const unsigned long uptimeInterval = 1000;

unsigned long uptimeCounter = 0;

float temperature = 0.0;
float humidity = 0.0;
float ldrLux = 0.0; // Biến lưu giá trị LDR (Lux)
float ldrThreshold = 50.0; // Ngưỡng ánh sáng từ Blynk (mặc định 50 Lux)

// Đèn giao thông
enum TrafficState { GREEN, RED, YELLOW };
TrafficState currentState = GREEN;
int countdown = 10;
unsigned long lastMillis = 0;
unsigned long lastBlinkMillis = 0;
const long blinkDelay = 500;
bool yellowBlink = false;

void displayCountdown(int seconds) {
  if (displayOn) {
    display.showNumberDecEx(seconds, 0x40, true);
  } else {
    display.clear();
  }
}

// Điều khiển từ Blynk (Bật/Tắt LED và Display)
BLYNK_WRITE(V0) {
  displayOn = param.asInt();
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
  if (!displayOn) {
    display.clear();
  } else {
    displayCountdown(countdown);
  }
}

// Nhận ngưỡng ánh sáng từ Blynk qua V4
BLYNK_WRITE(V4) {
    ldrThreshold = param.asFloat(); // Lấy giá trị ngưỡng từ Blynk
    Serial.print("Received LDR Threshold from Blynk V4: ");
    Serial.println(ldrThreshold);
}

// Xử lý nút nhấn
void handleButton() {
    int reading = digitalRead(BUTTON_PIN);
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading == LOW) {
            displayOn = !displayOn;
            Blynk.virtualWrite(V0, displayOn);
            digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
            if (!displayOn) {
                display.clear();
            } else {
                displayCountdown(countdown);
            }
        }
    }
    lastButtonState = reading;
}

// Điều khiển đèn giao thông dựa vào LDR
void handleTrafficLight() {
  unsigned long currentMillis = millis();
  int ldrValue = analogRead(LDR_PIN);
  ldrLux = map(ldrValue, 0, 4095, 0, 1000); // Chuyển đổi giá trị LDR thành Lux

  Serial.print("LDR Lux: ");
  Serial.println(ldrLux);

  if (ldrLux < ldrThreshold) { // Khi môi trường tối (so với ngưỡng từ V4)
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);

    if (currentMillis - lastBlinkMillis >= blinkDelay) {
      lastBlinkMillis = currentMillis;
      yellowBlink = !yellowBlink;
      digitalWrite(LED_YELLOW, yellowBlink);
    }

    display.clear();
    return;
  }

  if (currentMillis - lastMillis >= 1000) {
    lastMillis = currentMillis;
    countdown--;

    if (countdown < 0) {
      switch (currentState) {
        case GREEN:
          currentState = RED;
          countdown = 10;
          break;
        case RED:
          currentState = YELLOW;
          countdown = 3;
          break;
        case YELLOW:
          currentState = GREEN;
          countdown = 10;
          break;
      }
    }
    displayCountdown(countdown);
  }

  // Điều khiển đèn theo trạng thái
  digitalWrite(LED_GREEN, currentState == GREEN);
  digitalWrite(LED_YELLOW, currentState == YELLOW);
  digitalWrite(LED_RED, currentState == RED);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  digitalWrite(LED_PIN, HIGH);
  dht.begin();
  display.setBrightness(0x0f);
  display.showNumberDec(0);
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected to Blynk");
  
  Blynk.virtualWrite(V0, displayOn);
  Blynk.virtualWrite(V4, ldrThreshold); // Đồng bộ giá trị ngưỡng ban đầu lên Blynk
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();
  
  handleButton();
  handleTrafficLight();

  // Gửi dữ liệu cảm biến lên Blynk
  if (currentMillis - sensorLastReadTime >= sensorInterval) {
    sensorLastReadTime = currentMillis;

    // Đọc và gửi nhiệt độ, độ ẩm
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    if (!isnan(temperature) && !isnan(humidity)) {
      Blynk.virtualWrite(V3, temperature);
      Blynk.virtualWrite(V2, humidity);
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
    } else {
      Serial.println("Failed to read from DHT sensor!");
    }

    
  }

  // Gửi thời gian hoạt động
  if (currentMillis - lastUptimeSendTime >= uptimeInterval) {
    lastUptimeSendTime = currentMillis;
    uptimeCounter++;
    Blynk.virtualWrite(V1, uptimeCounter);
  }
}