#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL69kKlGRk4"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project"
#define BLYNK_AUTH_TOKEN "e48658AtfppPhVuaCVu9H2adOQoW_0nK"

#include "utils.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Thông tin WiFi
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

// Telegram Bot
#define BOT_TOKEN "7566364423:AAEdj6Us9k1aXl-thFjqfdLHtk7bcFcpfHU"
#define CHAT_ID "-4663865281"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

#define gPIN 15  // Đèn xanh
#define yPIN 2   // Đèn vàng
#define rPIN 5   // Đèn đỏ
#define OLED_SDA 13
#define OLED_SCL 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

bool yellowBlinkMode = false;
bool trafficOn = true;

int currentLedIndex = 0;
unsigned long lastLedSwitchTime = 0;
const int ledPin[3] = {gPIN, yPIN, rPIN};
const int durations[3] = {5000, 7000, 2000}; // Xanh 5s, Vàng 7s, Đỏ 2s

// Biến toàn cục để lưu nhiệt độ và độ ẩm
float temperature = 0.0;
float humidity = 0.0;

bool WelcomeDisplayTimeout(uint msSleep = 5000) {
  static unsigned long lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;
  return bDone;
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

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

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  client.setInsecure();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  if (Blynk.connected()) {
    Serial.println("Connected to Blynk!");
  } else {
    Serial.println("Failed to connect to Blynk!");
  }
}

void ThreeLedBlink() {
  static unsigned long lastTimer = 0;
  if (yellowBlinkMode || !trafficOn) return;
  if (!IsReady(lastTimer, durations[currentLedIndex])) return;

  int prevLed = (currentLedIndex + 2) % 3;
  digitalWrite(ledPin[prevLed], LOW);
  digitalWrite(ledPin[currentLedIndex], HIGH);
  lastLedSwitchTime = millis();
  currentLedIndex = (currentLedIndex + 1) % 3;
}

void yellowBlink() {
  static unsigned long lastTimer = 0;
  static bool state = false;
  if (!yellowBlinkMode) return;
  if (!IsReady(lastTimer, 2000)) return;

  state = !state;
  digitalWrite(gPIN, LOW);
  digitalWrite(rPIN, LOW);
  digitalWrite(yPIN, state);
}

float generateRandomTemperature() {
  return random(-400, 800) / 10.0;
}

float generateRandomHumidity() {
  return random(0, 1000) / 10.0;
}

void updateRandomDHT() {
  static unsigned long lastTimer = 0;
  if (!IsReady(lastTimer, 5000)) return; // 5 giây = 5,000 ms

  temperature = generateRandomTemperature();
  humidity = generateRandomHumidity();

  Serial.print("Random Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("Random Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
}

void checkHealthConditions() {
  static unsigned long lastAlertTime = 0;
  if (!IsReady(lastAlertTime, 300000)) return; // 5 phút = 300,000 ms

  String NT = "";
  String DA = "";
  // Nhiệt độ
  if (temperature < 10) NT = "Nhiệt độ < 10°C - Nguy cơ hạ thân nhiệt!";
  else if (10 <= temperature <= 15) NT = "Cảm giác lạnh, tăng nguy cơ mắc bệnh đường hô hấp.";
  else if (20 <= temperature <= 30) NT = " Nhiệt độ lý tưởng, ít ảnh hưởng đến sức khỏe.";
  else if (30 < temperature <= 35) NT = "Cơ thể bắt đầu có dấu hiệu mất nước, mệt mỏi.";
  else if (temperature > 35) NT = "Nguy cơ sốc nhiệt, chuột rút, say nắng.";
  else if (temperature > 40) NT = "Cực kỳ nguy hiểm, có thể gây suy nội tạng, đột quỵ nhiệt.";
  // Độ ẩm
  if (humidity < 30) DA = "Da khô, kích ứng mắt, tăng nguy cơ mắc bệnh về hô hấp (viêm họng, khô mũi).";
  else if (40 <= humidity <= 60) DA = "Mức lý tưởng, ít ảnh hưởng đến sức khỏe.";
  else if (humidity > 70) DA = "Tăng nguy cơ nấm mốc, vi khuẩn phát triển, gây bệnh về đường hô hấp.";
  else if (humidity > 80) DA = "Cảm giác oi bức, khó thở, cơ thể khó thoát mồ hôi, tăng nguy cơ sốc nhiệt.";

  if (NT != "" && DA != "") {
    String canhBao = "Cảnh báo: " + NT + "\n" + DA;
    bot.sendMessage(CHAT_ID, canhBao);
    Serial.println(canhBao); // hiển thị ra telegram
  }
}

void handleTelegramMessages() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = bot.messages[i].chat_id;
      String text = bot.messages[i].text;

      if (chat_id != CHAT_ID) continue;

      if (text == "/traffic_off") {
        trafficOn = false;
        digitalWrite(gPIN, LOW);
        digitalWrite(yPIN, LOW);
        digitalWrite(rPIN, LOW);
        bot.sendMessage(CHAT_ID, "Đèn giao thông đã tắt!");
      } else if (text == "/traffic_on") {
        trafficOn = true;
        bot.sendMessage(CHAT_ID, "Đèn giao thông hoạt động trở lại!");
      } else {
        bot.sendMessage(CHAT_ID, "Lệnh không hợp lệ! Dùng: /traffic_on hoặc /traffic_off");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void updateOLED() {
    static unsigned long lastTimer = 0;
    if (!IsReady(lastTimer, 1000)) return; // Cập nhật OLED mỗi giây
  
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese1);
    String tempStr = StringFormat("Nhiet: %.1f C", temperature);
    String humStr = StringFormat("Do am: %.1f %%", humidity);
  
    unsigned long uptime = millis() / 1000;
    int hours = uptime / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;
    String uptimeStr = StringFormat("Up: %dh %02dm %02ds", hours, minutes, seconds);
  
    oled.drawUTF8(0, 14, tempStr.c_str());
    oled.drawUTF8(0, 28, humStr.c_str());
    oled.drawUTF8(0, 42, uptimeStr.c_str());
  
    if (!yellowBlinkMode && trafficOn) {
      unsigned long elapsed = millis() - lastLedSwitchTime;
      int remainingTime = (durations[currentLedIndex] - elapsed) / 1000;
      if (remainingTime < 0) remainingTime = 0;
  
      String ledStr;
      if (ledPin[currentLedIndex] == gPIN) ledStr = "Xanh";
      else if (ledPin[currentLedIndex] == yPIN) ledStr = "Vang";
      else ledStr = "Do";
  
      String countdownStr = StringFormat("%s: %ds", ledStr.c_str(), remainingTime);
      oled.drawUTF8(0, 56, countdownStr.c_str());
    }
  
    oled.sendBuffer();
  }
  

void updateUptime() {
    static unsigned long lastTimer = 0;
    unsigned long currentMillis = millis();
  
    if (currentMillis - lastTimer >= 1000) { // Kiểm tra mỗi giây
      lastTimer = currentMillis;
  
      unsigned long uptime = currentMillis / 1000; // Tính thời gian hoạt động (giây)
      Blynk.virtualWrite(V0, uptime); // Gửi giá trị lên Blynk
  
      Serial.print("Uptime (seconds) sent to Blynk: ");
      Serial.println(uptime);
    }
  }
  

BLYNK_WRITE(V3) {
  yellowBlinkMode = param.asInt();
  if (!yellowBlinkMode) digitalWrite(yPIN, LOW);
}

void loop() {
  Blynk.run();
  if (!WelcomeDisplayTimeout()) return;
  ThreeLedBlink();
  yellowBlink();
  updateRandomDHT();
  updateOLED(); // Cập nhật OLED mỗi giây
  updateUptime();
  checkHealthConditions();
  handleTelegramMessages();
}