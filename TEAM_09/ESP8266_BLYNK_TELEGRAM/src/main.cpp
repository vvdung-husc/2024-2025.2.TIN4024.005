//Hoàng Văn Long
//#define BLYNK_TEMPLATE_ID "TMPL6G-_1Se-p"
//#define BLYNK_TEMPLATE_NAME "ESP8266"
//#define BLYNK_AUTH_TOKEN "J1RzJuboKtIHo0ZCcegBupFpHC00XLDC"
//Phan Văn Nhật Duy
#define BLYNK_TEMPLATE_ID "TMPL6xbr4RanW"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "vT1mc0MfWPD5B4w4qADVg7o8IXLde32d"


#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define OLED_SDA 13
#define OLED_SCL 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
bool yellowBlinkMode = false;
bool trafficEnabled = true;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";
//Phan Văn Nhật Duy
const char* botToken ="8044446031:AAGghQhARa6eiLzS9WLTyvBXoxVFlcY5oEc";
const char* chatID = "-1002619419433";
//Hoàng Văn Long
//const char* botToken ="7179261234:AAERMDKB_0mSa4SBp6tymQeOiYb6ebDHVJg";
//const char* chatID = "-1002559266898";
//Nguyễn Nhật Thi
//const char* botToken "7141650163:AAHVzmOzZk_oaqqfMp1e2o3QE1nm9ZdC7tA" ;
//const char* chatID "-4656911197";
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

void sendTelegramAlert(String message) {
  bot.sendMessage(chatID, message, "Markdown");
}

void setup() {
  Serial.begin(115200);
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  Blynk.begin(auth, ssid, pass);
  client.setInsecure();

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT-NHOM09");
  oled.sendBuffer();
}

BLYNK_WRITE(V1) {
  yellowBlinkMode = param.asInt();
}

void ThreeLedBlink() {
  static ulong lastTimer = 0;
  static int currentLed = 0;
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (millis() - lastTimer < 1000 || !trafficEnabled) return;
  lastTimer = millis();

  if (yellowBlinkMode) {
    digitalWrite(gPIN, LOW);
    digitalWrite(rPIN, LOW);
    digitalWrite(yPIN, !digitalRead(yPIN));
  } else {
    digitalWrite(ledPin[(currentLed + 2) % 3], LOW);
    digitalWrite(ledPin[currentLed], HIGH);
    currentLed = (currentLed + 1) % 3;
  }
}

void updateDHT() {
  static ulong lastTimer = 0;
  static ulong lastAlert = 0;
  if (millis() - lastTimer < 2000) return;
  lastTimer = millis();

  float t = random(-400, 800) / 10.0;
  float h = random(0, 1000) / 10.0;

  Serial.printf("Temperature: %.2f °C, Humidity: %.2f %%\n", t, h);
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);
  oled.setCursor(0, 20);
  oled.printf("Nhiet do: %.2f °C", t);
  oled.setCursor(0, 40);
  oled.printf("Do am: %.2f %%", h);
  oled.sendBuffer();
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);

  if (millis() - lastAlert > 300000) {
    if (t < 10 || t > 35 || h < 30 || h > 80) {
      String alertMsg = "🚨 Cảnh báo! Nhiệt độ: " + String(t) + "°C, Độ ẩm: " + String(h) + "% vượt ngưỡng an toàn!";
      sendTelegramAlert(alertMsg);
      lastAlert = millis();
    }
  }
}

void checkTelegram() {
  int messageCount = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < messageCount; i++) {
    String text = bot.messages[i].text;
    if (text == "/traffic_off") {
      trafficEnabled = false;
      digitalWrite(gPIN, LOW);
      digitalWrite(yPIN, LOW);
      digitalWrite(rPIN, LOW);
      sendTelegramAlert("🚦 Đèn giao thông đã tắt!");
    } else if (text == "/traffic_on") {
      trafficEnabled = true;
      sendTelegramAlert("🚦 Đèn giao thông hoạt động trở lại!");
    }
  }
}

void loop() {
  Blynk.run();
  ThreeLedBlink();
  updateDHT();
  checkTelegram();
}
