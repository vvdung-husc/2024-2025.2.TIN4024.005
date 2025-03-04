// (1) Khai báo thông tin Blynk trước khi include thư viện
#define BLYNK_TEMPLATE_ID "TMPL6VUsiWRYo"
#define BLYNK_TEMPLATE_NAME "ESP32 TRAFFIC DHT SENSOR"
#define BLYNK_AUTH_TOKEN "Y3PaCsXarmz6Atq_EnDC7BNx3VAorkXO"

#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// (2) WiFi credentials (sử dụng Wokwi-GUEST cho mô phỏng)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// (3) Định nghĩa chân kết nối
#define BUTTON_PIN 23   // Nút nhấn (btn1)
#define LED_PIN    21   // LED xanh (led1)
#define DHT_PIN    16   // DHT22 (dht1:SDA)
#define CLK_PIN    18   // TM1637 CLK (sevseg1:CLK)
#define DIO_PIN    19   // TM1637 DIO (sevseg1:DIO)
#define DHT_TYPE   DHT22

// (4) Khởi tạo đối tượng DHT & màn hình
DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK_PIN, DIO_PIN);

// (5) Biến toàn cục
bool displayOn = true;            // true: hiển thị uptime trên TM1637, false: tắt màn hình
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long sensorLastReadTime = 0;
const unsigned long sensorInterval = 2000; // Đọc cảm biến mỗi 2 giây

float temperature = 0.0;
float humidity = 0.0;

// Biến & khoảng thời gian cập nhật uptime
unsigned long lastUptimeSendTime = 0;
const unsigned long uptimeInterval = 1000; // Cập nhật uptime mỗi 1 giây

// (6) Callback Blynk: nút trên Virtual Pin V1 để bật/tắt hiển thị
BLYNK_WRITE(V1) {
  displayOn = param.asInt(); // 1 -> hiển thị, 0 -> tắt
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  display.setBrightness(0x0F);
  display.showNumberDec(0, true);

  dht.begin();

  // (7) Kết nối WiFi & Blynk
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected to Blynk");

  // Đồng bộ trạng thái ban đầu với Blynk
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
  Blynk.virtualWrite(V1, displayOn);
}

void loop() {
  Blynk.run();

  unsigned long currentMillis = millis();

  // (8) Xử lý nút nhấn vật lý (debounce) để bật/tắt hiển thị
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis;
  }
  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (lastButtonState == HIGH && reading == LOW) {
      // Toggle trạng thái hiển thị
      displayOn = !displayOn;
      Blynk.virtualWrite(V1, displayOn); // Đồng bộ sang Blynk
    }
  }
  lastButtonState = reading;

  // (9) Đồng bộ LED xanh với trạng thái hiển thị
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);

  // (10) Đọc dữ liệu DHT22 mỗi sensorInterval ms
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
      // Gửi dữ liệu cảm biến lên Blynk
      Blynk.virtualWrite(V2, temperature);
      Blynk.virtualWrite(V3, humidity);
    }
  }

  // (11) Uptime: hiển thị trên TM1637 & gửi lên Blynk (V0) mỗi 1 giây
  if (currentMillis - lastUptimeSendTime >= uptimeInterval) {
    lastUptimeSendTime = currentMillis;
    unsigned long uptimeSeconds = currentMillis / 1000;

    // Gửi uptime lên Blynk
    Blynk.virtualWrite(V0, uptimeSeconds);

    // Hiển thị trên TM1637 nếu displayOn == true
    if (displayOn) {
      display.showNumberDec(uptimeSeconds, true);
    } else {
      display.clear();
    }
  }

  delay(50);
}
