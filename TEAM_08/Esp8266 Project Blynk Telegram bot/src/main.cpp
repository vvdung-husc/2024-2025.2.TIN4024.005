// (Lê Thị Thảo)
// #define BLYNK_TEMPLATE_ID "TMPL65O6sDW_i"
// #define BLYNK_TEMPLATE_NAME "ESP8266"
// #define BLYNK_AUTH_TOKEN "LFlWmSFmwNYBB0y5H6WYq5lf7EMZtGxl"

// #define BOT_TOKEN "7885026566:AAGa0Tjz0dJ4C6CSyo1_iMC8KleoFYlUczI"
// #define GROUP_ID "-4684647200"
// #define ADMIN_ID "5358198549"

//(Phạm Mạnh Hoàng)
#define BLYNK_TEMPLATE_ID "TMPL6lnJuhhnf"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "kP2tbphYsFPG_14H7FCDuSdJ6juTUnXY"

#define BOT_TOKEN "8052849841:AAH736dza0CdpWsMipOP-TWlwkA4qSWFJuo"
#define GROUP_ID "-4615063743"
#define ADMIN_ID "5551063619"

// #define BLYNK_TEMPLATE_ID "TMPL6U3DIjxBu"
// #define BLYNK_TEMPLATE_NAME "BlynkTelegram"
// #define BLYNK_AUTH_TOKEN "3idL0m2aHIXWgpE-6tzmT5OBgR7m7bRq"

// Token Telegram Bot
// #define BOT_TOKEN "7701372718:AAFMrL_zhJWiBivuHbdcO4vlyOK8hbigaQU"  
// #define CHAT_ID "-4758447997"

// (Vũ Văn Hoàng)
// #define BLYNK_TEMPLATE_ID "TMPL6EnQwI3ZN"
// #define BLYNK_TEMPLATE_NAME "Esp8266"
// #define BLYNK_AUTH_TOKEN "IJHJllpj-GsJZBpbgOsxoWjpGkIQhdS6"

// #define BOTtoken "7552149595:AAG_nU1B5MBSMpj8_fk-yUi_bXF-RCY9TIg"
// #define GROUP_ID "-4770066788"
// #define ADMIN_ID "5755242161"


//(Nguyễn Trọng Khôi Nguyên)
// #define BLYNK_TEMPLATE_ID "TMPL61Qq4UXhq"
// #define BLYNK_TEMPLATE_NAME "Esp8266 Project Blynk Telegram bot"
// #define BLYNK_AUTH_TOKEN "YhmHYSiC3dN4WPiUz5FRc9Yq4aBm_c7j"

// 🔹 Cấu hình Telegram
// #define BOT_TOKEN "7679532681:AAE1oPGDkkSNCUVlMcUEPsChgMrYWJdqhsw"
// #define GROUP_ID "-4688474179"
// #define ADMIN_ID "7173099827"


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>


WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// 🟢 Chân LED (đỏ, vàng, xanh)
#define gPIN 15
#define yPIN 2
#define rPIN 5

// 📟 Chân I2C OLED
#define OLED_SDA 13
#define OLED_SCL 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

unsigned long runTime = 0;
float fTemperature = 0.0;
float fHumidity = 0.0;
bool yellowBlinkMode = false;
bool yellowOnlyMode = false; // 🔥 Chế độ chỉ bật đèn vàng
bool trafficLightEnabled = true;

BlynkTimer timer;

// 📶 Nhận lệnh bật/tắt chế độ đèn vàng từ Blynk (V4)
BLYNK_WRITE(V1) {
  yellowOnlyMode = param.asInt(); // 1 = bật, 0 = tắt
}

// 📟 Cập nhật dữ liệu trên OLED
void updateOLED() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);
  oled.drawUTF8(0, 14, ("Nhiet do: " + String(fTemperature, 1) + "°C").c_str());
  oled.drawUTF8(0, 28, ("Do am: " + String(fHumidity, 1) + "%").c_str());
  oled.drawUTF8(0, 42, ("Thoi gian: " + String(runTime) + "s").c_str());
  oled.sendBuffer();
}

// 🚦 Điều khiển đèn giao thông
void TrafficLightControl() {
  if (!trafficLightEnabled) {
    digitalWrite(rPIN, LOW);
    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);
    return;
  }

  if (yellowOnlyMode) {
    digitalWrite(rPIN, LOW);
    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, HIGH); // Bật đèn vàng cố định
    return;
  }

  static unsigned long lastTimer = 0;
  static int state = 0;
  static const unsigned long durations[] = {2000, 3000, 1000}; // Đỏ 2s, Xanh 3s, Vàng 1s
  static const int ledPins[] = {rPIN, gPIN, yPIN};

  if (yellowBlinkMode) {
    if (millis() - lastTimer > 500) {
      lastTimer = millis();
      digitalWrite(yPIN, !digitalRead(yPIN));
    }
    digitalWrite(rPIN, LOW);
    digitalWrite(gPIN, LOW);
    return;
  }

  if (millis() - lastTimer > durations[state]) {
    lastTimer = millis();
    digitalWrite(ledPins[state], LOW);
    state = (state + 1) % 3;
    digitalWrite(ledPins[state], HIGH);
  }
}

// 🔢 Sinh dữ liệu nhiệt độ & độ ẩm ngẫu nhiên
float randomTemperature() {
  return random(200, 350) / 10.0;
}

float randomHumidity() {
  return random(300, 800) / 10.0;
}

// 🌡 Cập nhật nhiệt độ & độ ẩm
void updateSensorData() {
  static unsigned long lastTimer = 0;
  if (millis() - lastTimer < 2000) return;
  lastTimer = millis();

  fTemperature = randomTemperature();
  fHumidity = randomHumidity();

  Serial.print("Nhiet do: ");
  Serial.print(fTemperature);
  Serial.println("°C");
  Serial.print("Do am: ");
  Serial.print(fHumidity);
  Serial.println("%");
}

// 📟 Gửi dữ liệu lên Blynk
void sendToBlynk() {
  Blynk.virtualWrite(V0, runTime);
  Blynk.virtualWrite(V2, fTemperature); // 🌡 Gửi nhiệt độ lên Blynk
  Blynk.virtualWrite(V3, fHumidity);    // 💧 Gửi độ ẩm lên Blynk
}

// 📟 Gửi cảnh báo đến Telegram nếu nhiệt độ hoặc độ ẩm vượt ngưỡng
// 📟 Gửi cảnh báo đến Telegram
void sendAlertTelegram() {
  static unsigned long lastAlert = 0;
  if (millis() - lastAlert < 300000) return;
  lastAlert = millis();

  String message = "📡 Cập nhật thông tin:\n";
  message += "🌡 Nhiệt độ: " + String(fTemperature, 1) + "°C\n";
  message += "💧 Độ ẩm: " + String(fHumidity, 1) + "%\n\n";

  if (fTemperature < 10)
    message += "⚠️ Nguy cơ hạ thân nhiệt, tê cóng, giảm miễn dịch.\n";
  else if (fTemperature >= 10 && fTemperature < 15)
    message += "⚠️ Cảm giác lạnh, tăng nguy cơ mắc bệnh đường hô hấp.\n";
  else if (fTemperature >= 25 && fTemperature < 30)
    message += "✅ Nhiệt độ lý tưởng, ít ảnh hưởng đến sức khỏe.\n";
  else if (fTemperature >= 30 && fTemperature < 35)
    message += "⚠️ Cơ thể bắt đầu có dấu hiệu mất nước, mệt mỏi.\n";
  else if (fTemperature > 35 && fTemperature <= 40)
    message += "🚨 Nguy cơ sốc nhiệt, chuột rút, say nắng.\n";
  else if (fTemperature > 40)
    message += "🚨 Cực kỳ nguy hiểm! Có thể gây suy nội tạng, đột quỵ.\n";

  if (fHumidity < 30)
    message += "⚠️ Da khô, kích ứng mắt, tăng nguy cơ mắc bệnh về hô hấp.\n";
  else if (fHumidity >= 40 && fHumidity <= 60)
    message += "✅ Mức lý tưởng, ít ảnh hưởng đến sức khỏe.\n";
  else if (fHumidity > 70 && fHumidity <= 80)
    message += "⚠️ Tăng nguy cơ nấm mốc, vi khuẩn phát triển, bệnh đường hô hấp.\n";
  else if (fHumidity > 80)
    message += "🚨 Cảm giác oi bức, khó thở, cơ thể khó thoát mồ hôi, tăng nguy cơ sốc nhiệt.\n";

  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(GROUP_ID, message, "");
  }
}

// 📨 Xử lý lệnh từ Telegram
void handleNewMessages() {
  int messageCount = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < messageCount; i++) {
    String text = bot.messages[i].text;
    String sender_id = String(bot.messages[i].from_id);

    if (sender_id != ADMIN_ID) {
      bot.sendMessage(GROUP_ID, "⛔ Bạn không có quyền điều khiển bot!", "");
      return;
    }

    if (text == "/traffic_off") {
      trafficLightEnabled = false;
      bot.sendMessage(GROUP_ID, "🚦 Đèn giao thông đã tắt.", "");
    } else if (text == "/traffic_on") {
      trafficLightEnabled = true;
      bot.sendMessage(GROUP_ID, "🚦 Đèn giao thông hoạt động bình thường.", "");
    }
  }
}

// 🔢 Hiển thị thời gian chạy
void updateRunTime() {
  static unsigned long lastTimer = 0;
  if (millis() - lastTimer < 1000) return;
  lastTimer = millis();
  runTime++;
}

// 🏁 SETUP
void setup() {
  Serial.begin(115200);

  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);
  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  
  WiFi.begin("CNTT-MMT", "13572468");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  client.setInsecure();
  Blynk.begin(BLYNK_AUTH_TOKEN, "CNTT-MMT", "13572468");

  timer.setInterval(2000L, updateSensorData);
  timer.setInterval(300000L, sendAlertTelegram);
  timer.setInterval(5000L, handleNewMessages);
  timer.setInterval(2000L, sendToBlynk);
}

// 🔁 LOOP
void loop() {
  Blynk.run();
  timer.run();
  TrafficLightControl();
  updateRunTime();
  updateOLED();
}

