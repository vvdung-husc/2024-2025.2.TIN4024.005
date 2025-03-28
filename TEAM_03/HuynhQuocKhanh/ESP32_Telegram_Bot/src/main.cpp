#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi Credentials
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// Telegram Bot Token
#define BOTtoken "7344244327:AAFVUIe-F0wKX9GrKY6zm8QNChoB514L0bs"
#define GROUP_ID "-1002539091533" // Chat ID của nhóm hoặc cá nhân

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27;
volatile bool motionDetected = false;
uint motionCount = 0;

// Hàm định dạng chuỗi giống printf
String StringFormat(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  char buf[128];
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  return String(buf);
}

// Xử lý khi phát hiện chuyển động
void IRAM_ATTR detectsMovement()
{
  motionDetected = true;
}

void connectWiFi()
{
  Serial.print("🔄 Đang kết nối WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setInsecure(); // Bỏ qua kiểm tra chứng chỉ SSL

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30)
  {
    Serial.print(".");
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\n✅ WiFi đã kết nối!");
    bot.sendMessage(GROUP_ID, "✅ IoT Developer started up!");
  }
  else
  {
    Serial.println("\n❌ Không thể kết nối WiFi!");
  }
}

void reconnectWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("⚠️ Mất WiFi! Đang thử kết nối lại...");
    WiFi.disconnect();
    WiFi.reconnect();
    delay(5000);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  connectWiFi();
}

void loop()
{
  reconnectWiFi(); // Kiểm tra lại WiFi

  if (motionDetected)
  {
    motionDetected = false; // Đặt lại cờ

    motionCount++;
    Serial.print(motionCount);
    Serial.println(". 🚨 Phát hiện chuyển động => Gửi tin Telegram");

    String msg = StringFormat("🚨 Phát hiện chuyển động! (Lần: %u)", motionCount);
    
    bool success = false;
    for (int i = 0; i < 3; i++) // Thử lại 3 lần nếu gửi thất bại
    {
      success = bot.sendMessage(GROUP_ID, msg.c_str(), "");
      if (success)
        break;
      Serial.println("⚠️ Gửi tin thất bại! Đang thử lại...");
      delay(2000);
    }

    if (success)
      Serial.println("✅ Gửi thành công!");
    else
      Serial.println("❌ Gửi thất bại sau 3 lần thử!");
  }
}
