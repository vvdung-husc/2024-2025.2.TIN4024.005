#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>

#define BLYNK_TEMPLATE_ID "TMPL6UtUU_5WZ"
#define BLYNK_TEMPLATE_NAME "ESP8266Project"
#define BLYNK_AUTH_TOKEN "p_B7I5wU_xTCi3owpkhbeklgLbr7r-8R"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

#define gPIN 15  // Đèn xanh (D8/GPIO 15)
#define yPIN 2   // Đèn vàng (D4/GPIO 2)
#define rPIN 5   // Đèn đỏ (D1/GPIO 5)

#define dhtPIN 16     // Digital pin connected to the DHT sensor (D0/GPIO 16)
#define dhtTYPE DHT11 // DHT 11

#define OLED_SDA 13   // D7/GPIO 13
#define OLED_SCL 12   // D6/GPIO 12

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Khởi tạo cảm biến DHT
DHT dht(dhtPIN, dhtTYPE);

// Traffic light variables
int currentState = 0;           // 0: Xanh, 1: Vàng, 2: Đỏ
unsigned long previousMillis = 0;
unsigned long remainingTime = 0;
unsigned long greenTime = 15000;  // 15 giây
unsigned long yellowTime = 2000;  // 2 giây
unsigned long redTime = 5000;     // 5 giây
bool yellowBlinkMode = false;

float fHumidity = 0.0;
float fTemperature = 0.0;

// Hàm tạo giá trị ngẫu nhiên
float randomFloat(float min, float max) {
  return min + (float)random(0, 1000) / 1000.0 * (max - min);
}

// Định nghĩa hàm IsReady
bool IsReady(unsigned long &lastTimer, unsigned long interval) {
  unsigned long currentTime = millis();
  if (currentTime - lastTimer >= interval) {
    lastTimer = currentTime;
    return true;
  }
  return false;
}

bool WelcomeDisplayTimeout(unsigned long msSleep = 5000) {
  static unsigned long lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;
  return bDone;
}

void updateDisplay() {
  static unsigned long lastTimer = 0;
  if (!IsReady(lastTimer, 1000)) return;  // Cập nhật màn hình mỗi 1 giây

  if (!yellowBlinkMode) {
    int secondsLeft = remainingTime / 1000;

    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);
    String s;
    if (currentState == 0) s = "Den xanh: " + String(secondsLeft) + "s";
    else if (currentState == 1) s = "Den vang: " + String(secondsLeft) + "s";
    else s = "Den do: " + String(secondsLeft) + "s";
    oled.drawUTF8(0, 12, s.c_str());

    s = "Nhiet do: " + String(fTemperature, 1) + "C";
    oled.drawUTF8(0, 48, s.c_str());
    s = "Do am: " + String(fHumidity, 1) + "%";
    oled.drawUTF8(0, 60, s.c_str());
    oled.sendBuffer();

    // Gửi thời gian đếm ngược lên Blynk V0
    Blynk.virtualWrite(V0, secondsLeft);
  }
}

void TrafficLightMode() {
  if (yellowBlinkMode) return;

  unsigned long currentMillis = millis();
  unsigned long interval = (currentState == 0) ? greenTime : (currentState == 1) ? yellowTime : redTime;

  // Tính thời gian còn lại
  remainingTime = interval - (currentMillis - previousMillis);

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(rPIN, LOW);
    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);

    switch (currentState) {
      case 0:  // Xanh -> Vàng
        digitalWrite(yPIN, HIGH);
        currentState = 1;
        break;
      case 1:  // Vàng -> Đỏ
        digitalWrite(rPIN, HIGH);
        currentState = 2;
        break;
      case 2:  // Đỏ -> Xanh
        digitalWrite(gPIN, HIGH);
        currentState = 0;
        break;
    }
    remainingTime = interval;  // Reset thời gian còn lại khi chuyển trạng thái
    Serial.print("[TRAFFIC] ");
    Serial.println(currentState == 0 ? "Green" : currentState == 1 ? "Yellow" : "Red");
  }
}

void YellowBlinkMode() {
  static unsigned long lastTimer = 0;
  static bool yellowState = false;

  if (!yellowBlinkMode) return;

  if (!IsReady(lastTimer, 1000)) return;

  digitalWrite(rPIN, LOW);
  digitalWrite(gPIN, LOW);
  yellowState = !yellowState;
  digitalWrite(yPIN, yellowState ? HIGH : LOW);
}

void updateDHT() {
  static unsigned long lastTimer = 0;
  if (!IsReady(lastTimer, 15000)) return;  // Cập nhật mỗi 15 giây (15000ms)

  // Tạo giá trị ngẫu nhiên cho nhiệt độ và độ ẩm
  float t = randomFloat(-40.0, 80.0);
  float h = randomFloat(0.0, 100.0);

  if (fTemperature != t) {
    fTemperature = t;
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
    Blynk.virtualWrite(V1, t);
  }

  if (fHumidity != h) {
    fHumidity = h;
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    Blynk.virtualWrite(V2, h);
  }
}

BLYNK_WRITE(V3) {
  yellowBlinkMode = param.asInt();
  if (!yellowBlinkMode) {
    digitalWrite(rPIN, LOW);
    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);
    currentState = 0;  // Reset về trạng thái xanh khi tắt chế độ nhấp nháy
    previousMillis = millis();  // Reset thời gian
    digitalWrite(gPIN, HIGH);  // Bật đèn xanh ngay lập tức
    remainingTime = greenTime;  // Reset thời gian còn lại
  }
}

BLYNK_WRITE(V4) {
  greenTime = param.asInt() * 1000;
  if (greenTime < 1000) greenTime = 1000;
  if (greenTime > 30000) greenTime = 30000;
}

BLYNK_WRITE(V5) {
  yellowTime = param.asInt() * 1000;
  if (yellowTime < 1000) yellowTime = 1000;
  if (yellowTime > 10000) yellowTime = 10000;
}

BLYNK_WRITE(V6) {
  redTime = param.asInt() * 1000;
  if (redTime < 1000) redTime = 1000;
  if (redTime > 30000) redTime = 30000;
}

void setup() {
  Serial.begin(115200);

  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);
  digitalWrite(gPIN, HIGH);  // Bắt đầu với đèn xanh
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);

  dht.begin();

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.clearBuffer();

  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT");
  oled.sendBuffer();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  previousMillis = millis();  // Khởi tạo thời gian ban đầu
  remainingTime = greenTime;  // Khởi tạo thời gian còn lại

  randomSeed(analogRead(0));  // Khởi tạo seed cho randomFloat
}

void loop() {
  Blynk.run();

  if (!WelcomeDisplayTimeout()) return;

  updateDHT();
  TrafficLightMode();  // Điều khiển đèn giao thông
  YellowBlinkMode();   // Chế độ nhấp nháy vàng
  updateDisplay();     // Cập nhật màn hình và V0
}