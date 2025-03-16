#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Thông tin WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Token Telegram Bot
#define BOT_TOKEN "7701372718:AAFMrL_zhJWiBivuHbdcO4vlyOK8hbigaQU"  
#define CHAT_ID "-4758447997"  // ID người nhận tin nhắn (cá nhân)

// Cấu hình Telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Cảm biến chuyển động PIR
const int motionSensor = 27;
bool motionDetected = false;
unsigned long lastMsgTime = 0;
const unsigned long delayTime = 5000; // Gửi tin nhắn tối đa mỗi 5 giây

// Định dạng chuỗi
String StringFormat(const char* fmt, ...) {
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return s;
}

// Xử lý khi phát hiện chuyển động
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Kết nối WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Chứng thực Telegram API

  int wifiTimeout = 20; // Thời gian chờ WiFi tối đa (10 giây)
  while (WiFi.status() != WL_CONNECTED && wifiTimeout > 0) {
    Serial.print(".");
    delay(500);
    wifiTimeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    bot.sendMessage(CHAT_ID, "🔹 IoT Developer started up 🚀", "");
  } else {
    Serial.println("\n⚠️ WiFi connection failed!");
  }
}

void loop() {
  if (motionDetected && millis() - lastMsgTime > delayTime) {
    lastMsgTime = millis();
    Serial.println("Motion detected! Sending alert to Telegram...");
    
    String msg = StringFormat("⚠️ Cảnh báo! Phát hiện chuyển động vào lúc %lu ms!", lastMsgTime);
    bot.sendMessage(CHAT_ID, msg, "");

    motionDetected = false;
  }
}
