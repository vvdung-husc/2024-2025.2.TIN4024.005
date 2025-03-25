#define BLYNK_TEMPLATE_ID "TMPL6lnJuhhnf"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "kP2tbphYsFPG_14H7FCDuSdJ6juTUnXY"

#include <Arduino.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>

#define WIFI_SSID "CNTT-MMT"
#define WIFI_PASS "13572468"

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define OLED_SDA 13
#define OLED_SCL 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

bool yellowBlinkMode = false;
ulong lastTimerDHT = 0;
ulong lastTimerLed = 0;
ulong lastTimerUptime = 0;
ulong startTime = millis(); // Thời gian bắt đầu chạy thiết bị

bool WelcomeDisplayTimeout(uint msSleep = 5000) {
  static ulong lastTimer = millis();
  static bool bDone = false;
  if (bDone) return true;
  if (millis() - lastTimer < msSleep) return false;
  bDone = true;
  return bDone;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Đang kết nối Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Đã kết nối Wi-Fi!");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  Serial.println("✅ Đã kết nối Blynk!");

  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.clearBuffer();
  
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");  
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT");  
  oled.sendBuffer();
}

void ThreeLedBlink() {
  static int currentLed = 0;
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (millis() - lastTimerLed < 1000) return;
  lastTimerLed = millis();
  
  if (yellowBlinkMode) {
    digitalWrite(gPIN, LOW);
    digitalWrite(rPIN, LOW);
    digitalWrite(yPIN, !digitalRead(yPIN)); // Nhấp nháy đèn vàng
    return;
  }

  int prevLed = (currentLed + 2) % 3;
  digitalWrite(ledPin[prevLed], LOW);
  digitalWrite(ledPin[currentLed], HIGH);
  currentLed = (currentLed + 1) % 3;
}

BLYNK_WRITE(V1) {
  int value = param.asInt();
  yellowBlinkMode = (value == 1); // Bật/Tắt nháy đèn vàng
}

float randomTemperature() {
  return random(-400, 800) / 10.0;
}

float randomHumidity() {
  return random(0, 1000) / 10.0;
}

String getUptime() {
  ulong seconds = (millis() - startTime) / 1000;
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  int sec = seconds % 60;
  return String(hours) + "h " + String(minutes) + "m " + String(sec) + "s";
}

float fHumidity = 0.0;
float fTemperature = 0.0;

void updateDHT() {
  if (millis() - lastTimerDHT < 2000) return;
  lastTimerDHT = millis();

  float h = randomHumidity();
  float t = randomTemperature();
  bool bDraw = false;

  if (fTemperature != t) {
    bDraw = true;
    fTemperature = t;
    Serial.print("🌡️ Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
    Blynk.virtualWrite(V2, t);
  }

  if (fHumidity != h) {
    bDraw = true;
    fHumidity = h;
    Serial.print("💧 Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    Blynk.virtualWrite(V3, h);
  }

  if (bDraw) {
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);
    
    String sTemp = "Nhiet do: " + String(t, 2) + " °C";
    oled.drawUTF8(0, 14, sTemp.c_str());
    
    String sHumid = "Do am: " + String(h, 2) + " %";
    oled.drawUTF8(0, 28, sHumid.c_str());

    String uptime = "T.gian: " + getUptime();
    oled.drawUTF8(0, 42, uptime.c_str());

    oled.sendBuffer();
  }
}

void updateUptime() {
  if (millis() - lastTimerUptime < 1000) return; // Cập nhật mỗi giây
  lastTimerUptime = millis();

  String uptime = getUptime();
  Serial.print("⏳ Uptime: ");
  Serial.println(uptime);
  
  Blynk.virtualWrite(V4, uptime); // Gửi thời gian hoạt động lên Blynk
}

void loop() {
  Blynk.run();
  if (!WelcomeDisplayTimeout()) return;
  ThreeLedBlink();
  updateDHT();
  updateUptime(); // Cập nhật thời gian hoạt động
}
