#ifndef ESP32
#error "This code is designed to run on an ESP32 board."
#endif

#define BLYNK_TEMPLATE_ID "TMPL6yTjMiow8"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "B3du7AXEeav_KcitkbrjYKqZf63Yii-R"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// **Cấu hình WiFi**
char ssid[] = "Cho Babe";   // Thay bằng tên WiFi của bạn
char pass[] = "hoichivan";  // Thay bằng mật khẩu WiFi của bạn

// **Khai báo chân kết nối**
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define CLK_PIN 18
#define DIO_PIN 19
#define DHT_TYPE DHT22
#define LED_CHANNEL 0  // Kênh PWM (chọn từ 0-15)

DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK_PIN, DIO_PIN);

// **Biến toàn cục**
bool ledState = false;  // Trạng thái LED
int brightness = 255;   // Độ sáng LED
bool displayOn = true;  // Trạng thái màn hình
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long sensorLastReadTime = 0;
const unsigned long sensorInterval = 2000;

float temperature = 0.0;
float humidity = 0.0;

unsigned long lastUptimeSendTime = 0;
const unsigned long uptimeInterval = 1000;

// **Xử lý Blynk Switch (V0) - Bật/Tắt LED**
BLYNK_WRITE(V0) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
  Serial.print("Switch V0: ");
  Serial.println(ledState ? "BẬT" : "TẮT");
}

// **Xử lý Blynk Slider (V1) - Điều chỉnh độ sáng**
BLYNK_WRITE(V1) {
  brightness = param.asInt();
  ledcWrite(LED_CHANNEL, brightness);  // Sử dụng kênh PWM 0
  Serial.print("Slider V1 - Độ sáng LED: ");
  Serial.println(brightness);
}

// **Xử lý Blynk Switch (V2) - Bật/Tắt màn hình TM1637**
BLYNK_WRITE(V2) {
  displayOn = param.asInt();
  Serial.print("Display V2: ");
  Serial.println(displayOn ? "BẬT" : "TẮT");
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);

  // Cấu hình PWM
  ledcSetup(LED_CHANNEL, 5000, 8);
  ledcAttachPin(LED_PIN, LED_CHANNEL);
  ledcWrite(LED_CHANNEL, brightness);

  display.setBrightness(7);
  display.showNumberDec(0, true);
  dht.begin();

  Serial.print("Kết nối WiFi: ");
  Serial.println(ssid);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("✅ Kết nối thành công với Blynk!");

  // Gửi trạng thái ban đầu lên Blynk
  Blynk.virtualWrite(V0, ledState);
  Blynk.virtualWrite(V1, brightness);
  Blynk.virtualWrite(V2, displayOn);
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

  // Xử lý debounce nút nhấn
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && lastButtonState == HIGH) {
      displayOn = !displayOn;
      Blynk.virtualWrite(V2, displayOn);
      Serial.print("Nút nhấn, Display V2: ");
      Serial.println(displayOn ? "BẬT" : "TẮT");
    }
  }

  lastButtonState = reading;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);

  // **Đọc dữ liệu từ cảm biến DHT22**
  if (currentMillis - sensorLastReadTime >= sensorInterval) {
    sensorLastReadTime = currentMillis;
    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();

    if (!isnan(newTemperature)) temperature = newTemperature;
    if (!isnan(newHumidity)) humidity = newHumidity;

    Serial.print("🌡 Nhiệt độ: ");
    Serial.print(temperature);
    Serial.print(" °C, 💧 Độ ẩm: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Gửi dữ liệu lên Blynk
    Blynk.virtualWrite(V3, temperature);  // Gửi nhiệt độ lên V3
    Blynk.virtualWrite(V4, humidity);     // Gửi độ ẩm lên V4
  }

  // **Gửi thời gian hoạt động lên Blynk**
  if (currentMillis - lastUptimeSendTime >= uptimeInterval) {
    lastUptimeSendTime = currentMillis;
    unsigned long uptimeSeconds = currentMillis / 1000;

    Blynk.virtualWrite(V5, uptimeSeconds); // Gửi uptime lên V5

    if (displayOn) {
      displayTime(uptimeSeconds);
    } else {
      display.clear();
    }
  }
}
