#define BLYNK_TEMPLATE_ID "TMPL6EnQwI3ZN"
#define BLYNK_TEMPLATE_NAME "Esp8266"
#define BLYNK_AUTH_TOKEN "IJHJllpj-GsJZBpbgOsxoWjpGkIQhdS6"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// 🔌 Thông tin Telegram Bot
#define BOTtoken "7552149595:AAG_nU1B5MBSMpj8_fk-yUi_bXF-RCY9TIg"
#define GROUP_ID "-4770066788"
#define id_admin "5755242161" // ID Telegram của bạn




WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

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
void sendTelegramAlert() {
  String message = "";

  if (fTemperature < 10) {
    message += "🔥 Cảnh báo: Nhiệt độ quá thấp! Nguy cơ hạ thân nhiệt, tê cóng.\n";
  } else if (fTemperature >= 10 && fTemperature < 15) {
    message += "⚠️ Cảnh báo: Trời lạnh, tăng nguy cơ mắc bệnh đường hô hấp.\n";
  } else if (fTemperature >= 30 && fTemperature < 35) {
    message += "🥵 Cảnh báo: Cơ thể có dấu hiệu mất nước, mệt mỏi.\n";
  } else if (fTemperature >= 35 && fTemperature < 40) {
    message += "🚨 Cảnh báo: Nguy cơ sốc nhiệt, chuột rút, say nắng!\n";
  } else if (fTemperature >= 40) {
    message += "🛑 Cảnh báo: Nhiệt độ cực kỳ nguy hiểm! Nguy cơ suy nội tạng, đột quỵ!\n";
  }

  if (fHumidity < 30) {
    message += "💨 Độ ẩm thấp! Da khô, kích ứng mắt, tăng nguy cơ bệnh hô hấp.\n";
  } else if (fHumidity > 70 && fHumidity <= 80) {
    message += "☁️ Độ ẩm cao! Tăng nguy cơ nấm mốc, vi khuẩn phát triển.\n";
  } else if (fHumidity > 80) {
    message += "💦 Độ ẩm quá cao! Oi bức, khó thở, tăng nguy cơ sốc nhiệt.\n";
  }

  if (message.length() > 0) {
    message = "⚠️ CẢNH BÁO MÔI TRƯỜNG ⚠️\n" + message;
    bot.sendMessage(GROUP_ID, message, "");
  }
}

// 📨 Xử lý lệnh từ Telegram
void handleTelegramCommands() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String sender_id = bot.messages[i].chat_id; // ID của người gửi

    Serial.println("Tin nhan nhan duoc: " + text);
    Serial.println("ID nguoi gui: [" + sender_id + "]"); // In ra để kiểm tra

    String admin_id = "5755242161"; // Đặt đúng ID của bạn (loại bỏ số dư)
    sender_id.trim();  // Xóa khoảng trắng thừa
    if (sender_id != admin_id) {
      bot.sendMessage(sender_id, "⛔ Bạn không có quyền điều khiển hệ thống!", "");
      continue;
    }

    if (text == "/traffic_off") {
      trafficLightEnabled = false;
      bot.sendMessage(GROUP_ID, "🚦 Đèn giao thông đã tắt!", "");
    } else if (text == "/traffic_on") {
      trafficLightEnabled = true;
      bot.sendMessage(GROUP_ID, "🚦 Đèn giao thông đã bật!", "");
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
  timer.setInterval(300000L, sendTelegramAlert);
  timer.setInterval(5000L, handleTelegramCommands);
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
