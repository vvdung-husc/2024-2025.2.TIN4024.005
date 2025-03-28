#include <Arduino.h>
#include "utils.h"
#include <Wire.h>
#include <U8g2lib.h>

#define BLYNK_TEMPLATE_ID "TMPL6X5vRad-t"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project"
#define BLYNK_AUTH_TOKEN "hLyv3208-9U8Sq_m42iamNNKL_pDC0ts"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

#define gPIN 15
#define yPIN 2
#define rPIN 5
#define OLED_SDA 13
#define OLED_SCL 12

// Telegram Bot Setup
#define BOT_TOKEN "7377317535:AAGSbtdkcMVeQ_VANB43-kj4CCetAzvZoKw"
#define CHAT_ID "-4727293043"
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

bool blinkMode = false;
bool trafficEnabled = true;
ulong startTime;
ulong lastAlertTime = 0;

BLYNK_WRITE(V0) {
  blinkMode = param.asInt();
}

bool WelcomeDisplayTimeout(uint msSleep = 5000){
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;    
  return bDone;
}

void setup() {
  Serial.begin(115200);
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
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  client.setInsecure();
  Serial.println(WiFi.status() == WL_CONNECTED ? "✅ WiFi kết nối thành công!" : "❌ Kết nối WiFi thất bại!");
  
  startTime = millis();
}

void sendAlert(int temp, int humidity) {
  if (millis() - lastAlertTime < 300000) return;
  lastAlertTime = millis();
  
  String message = "⚠️ Cảnh báo môi trường ⚠️\n";
  message += "Nhiệt độ: " + String(temp) + "°C\n";
  message += "Độ ẩm: " + String(humidity) + "%\n";

  if (temp < 10) {
    message += "🚨 Nguy cơ hạ thân nhiệt, tê cóng, giảm miễn dịch.\n";
  } else if (temp >= 10 && temp < 15) {
    message += "🌡️ Cảm giác lạnh, tăng nguy cơ mắc bệnh đường hô hấp.\n";
  } else if (temp >= 30 && temp < 35) {
    message += "💦 Cơ thể bắt đầu có dấu hiệu mất nước, mệt mỏi.\n";
  } else if (temp >= 35 && temp < 40) {
    message += "🔥 Nguy cơ sốc nhiệt, chuột rút, say nắng.\n";
  } else if (temp >= 40) {
    message += "🚑 Cực kỳ nguy hiểm! Có thể gây suy nội tạng, đột quỵ nhiệt.\n";
  }

  if (humidity < 30) {
    message += "🌬️ Da khô, kích ứng mắt, tăng nguy cơ mắc bệnh về hô hấp.\n";
  } else if (humidity > 70) {
    message += "🌫️ Tăng nguy cơ nấm mốc, vi khuẩn phát triển, gây bệnh về đường hô hấp.\n";
  } else if (humidity > 80) {
    message += "💦 Cảm giác oi bức, khó thở, tăng nguy cơ sốc nhiệt.\n";
  }
  
  bot.sendMessage(CHAT_ID, message, "Markdown");
}

void updateTemperature(){
  static ulong lastTimer = 0;  
  if (!IsReady(lastTimer, 2000)) return;

  int t = random(-40, 81);
  int h = random(0, 101);

  char tempBuffer[20];
  char humidityBuffer[20];
  sprintf(tempBuffer, "Nhiệt độ: %d°C", t);
  sprintf(humidityBuffer, "Độ ẩm: %d%%", h);
  
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 28, tempBuffer);
  oled.drawUTF8(0, 42, humidityBuffer);
  oled.sendBuffer();

  Serial.println(tempBuffer);
  Serial.println(humidityBuffer);
  
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);
  
  if (t < 10 || t > 35 || h < 30 || h > 80) {
    sendAlert(t, h);
  }
}

void updateTrafficLights(){
  static ulong lastTimer = 0;
  static int state = 0;
  if (!IsReady(lastTimer, 1000)) return;

  if (!trafficEnabled) {
    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);
    digitalWrite(rPIN, LOW);
    return;
  }

  if (blinkMode) {
    digitalWrite(gPIN, LOW);
    digitalWrite(rPIN, LOW);
    digitalWrite(yPIN, !digitalRead(yPIN));
  } else {
    digitalWrite(yPIN, LOW);
    digitalWrite(gPIN, state == 0);
    digitalWrite(rPIN, state == 1);
    digitalWrite(yPIN, state == 2);
    state = (state + 1) % 3;
  }
}

void updateRuntime(){
  static ulong lastTimer = 0;
  if (!IsReady(lastTimer, 1000)) return;
  
  ulong elapsedTime = (millis() - startTime) / 1024;
  Blynk.virtualWrite(V1, elapsedTime);
}

void handleTelegram(){
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    if (text == "/traffic_off") {
      trafficEnabled = false;
      bot.sendMessage(CHAT_ID, "🚦 Đèn giao thông đã tắt", "Markdown");
    } else if (text == "/traffic_on") {
      trafficEnabled = true;
      bot.sendMessage(CHAT_ID, "🚦 Đèn giao thông hoạt động trở lại", "Markdown");
    }
  }
}

void loop() {
  Blynk.run();
  if (!WelcomeDisplayTimeout()) return;
  updateTemperature();
  updateTrafficLights();
  updateRuntime();
  handleTelegram();
}


