#include <Arduino.h>
// Blynk 
//Hoàng Thanh Nhã
#define BLYNK_TEMPLATE_ID "TMPL6uXaA1tg0"
#define BLYNK_TEMPLATE_NAME "Project"
#define BLYNK_AUTH_TOKEN "RuMutY_A_wRiTDm7BTcRJcWIpY89zDba"

//Ton Huyen Kim Khánh
//#define BLYNK_TEMPLATE_ID "TMPL6jaTBmBM0"
//#define BLYNK_TEMPLATE_NAME "ESP8266 Project"
//#define BLYNK_AUTH_TOKEN "FqNdtB2y7zmDDZLgTP0NQ4ypw_1QY34f"

//Nguyễn Khánh Linh
//#define BLYNK_TEMPLATE_ID "TMPL69kKlGRk4"
//#define BLYNK_TEMPLATE_NAME "ESP8266 Project"
//#define BLYNK_AUTH_TOKEN "e48658AtfppPhVuaCVu9H2adOQoW_0nK"


//Dương Duy Khanh
//#define BLYNK_TEMPLATE_ID "TMPL6M7rOjSH1"
//#define BLYNK_TEMPLATE_NAME "Telegram"
//#define BLYNK_AUTH_TOKEN "afs2DqGURHCGMrDwEBwheVSTrPBTUbPS"
#include "utils.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <UniversalTelegramBot.h>

// Thông tin WiFi
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

// Telegram Bot
// Hoàng Thanh Nhã 
#define BOT_TOKEN "7975958050:AAH9EdnSBVFB6R_9Qs8bTtwbISixWwHSot0" 
#define CHAT_ID "-4720454162" 

// Tôn Huyền Kim Khánh
//#define BOT_TOKEN "7405557746:AAGGWwmLWX3N5yjaeSVTItw3YNFs67hpox4"
//#define CHAT_ID "-4691157407"

//Nguyễn Khánh Linh
//#define BOT_TOKEN "7566364423:AAEdj6Us9k1aXl-thFjqfdLHtk7bcFcpfHU"
//#define CHAT_ID "-4663865281"

//Dương Duy Khanh
//#define BOTtoken "7953116116:AAHD0oG34KQMV7SvzHaQa_KI27qcm83-B8U" 
//#define GROUP_ID "-4751134824" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ===== KHAI BÁO CHÂN =====
#define gPIN 15
#define yPIN 2
#define rPIN 5
#define OLED_SDA 13
#define OLED_SCL 12

// ===== OLED =====
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// ===== BIẾN TOÀN CỤC =====
bool yellowBlinkMode = false;
bool trafficOn = true;
int currentLedIndex = 0;
unsigned long lastLedSwitchTime = 0;
const int ledPin[3] = {gPIN, yPIN, rPIN};
const int durations[3] = {5000, 7000, 2000}; // Xanh, Vàng, Đỏ

float temperature = 0.0;
float humidity = 0.0;

// ===== HÀM HỖ TRỢ =====
bool IsReady(unsigned long &lastTime, unsigned long interval)
{
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= interval)
  {
    lastTime = currentTime;
    return true;
  }
  return false;
}

String StringFormat(const char *format, ...)
{
  char buf[128];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  return String(buf);
}

bool WelcomeDisplayTimeout(uint msSleep = 5000)
{
  static unsigned long lastTimer = 0;
  static bool bDone = false;
  if (bDone)
    return true;
  if (!IsReady(lastTimer, msSleep))
    return false;
  bDone = true;
  return bDone;
}

// ===== SETUP =====
void setup()
{
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
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  client.setInsecure();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  if (Blynk.connected())
  {
    Serial.println("Connected to Blynk!");
  }
  else
  {
    Serial.println("Failed to connect to Blynk!");
  }
}

// ===== ĐÈN GIAO THÔNG =====
void ThreeLedBlink()
{
  static unsigned long lastTimer = 0;
  if (yellowBlinkMode || !trafficOn)
    return;
  if (!IsReady(lastTimer, durations[currentLedIndex]))
    return;

  int prevLed = (currentLedIndex + 2) % 3;
  digitalWrite(ledPin[prevLed], LOW);
  digitalWrite(ledPin[currentLedIndex], HIGH);
  lastLedSwitchTime = millis();
  currentLedIndex = (currentLedIndex + 1) % 3;
}

void yellowBlink()
{
  static unsigned long lastTimer = 0;
  static bool state = false;
  if (!yellowBlinkMode)
    return;
  if (!IsReady(lastTimer, 2000))
    return;

  state = !state;
  digitalWrite(gPIN, LOW);
  digitalWrite(rPIN, LOW);
  digitalWrite(yPIN, state);
}

// ===== GIẢ LẬP CẢM BIẾN =====
float generateRandomTemperature() { return random(-400, 800) / 10.0; }
float generateRandomHumidity() { return random(0, 1000) / 10.0; }

void updateRandomDHT()
{
  static unsigned long lastTimer = 0;
  if (!IsReady(lastTimer, 5000))
    return;

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

// ===== CẢNH BÁO SỨC KHỎE =====
void checkHealthConditions()
{
  static unsigned long lastAlertTime = 0;
  if (!IsReady(lastAlertTime, 300000)) // 5 phút
    return;

  String NT = "";
  String DA = "";

  if (temperature < 10)
    NT = "- Nhiệt độ " + String(temperature) + "°C - Nguy cơ hạ thân nhiệt.";
  else if (temperature < 15)
    NT = "- Nhiệt độ " + String(temperature) + "°C - Cảm giác lạnh.";
  else if (temperature >= 20 && temperature <= 30)
    NT = "- Nhiệt độ " + String(temperature) + "°C - Lý tưởng.";
  else if (temperature > 30 && temperature <= 35)
    NT = "- Nhiệt độ " + String(temperature) + "°C - Có thể mất nước.";
  else if (temperature > 35 && temperature <= 40)
    NT = "- Nhiệt độ " + String(temperature) + "°C - Nguy cơ sốc nhiệt.";
  else if (temperature > 40)
    NT = "- Nhiệt độ " + String(temperature) + "°C - Rất nguy hiểm!";

  if (humidity < 30)
    DA = "- Độ ẩm " + String(humidity) + "% - Không khí khô.";
  else if (humidity >= 40 && humidity <= 60)
    DA = "- Độ ẩm " + String(humidity) + "% - Lý tưởng.";
  else if (humidity > 70 && humidity <= 80)
    DA = "- Độ ẩm " + String(humidity) + "% - Tăng nguy cơ nấm mốc.";
  else if (humidity > 80)
    DA = "- Độ ẩm " + String(humidity) + "% - Oi bức, khó thở.";

  if (NT != "" && DA != "")
  {
    String canhBao = "⚠️ Cảnh báo:\n" + NT + "\n" + DA;
    bot.sendMessage(CHAT_ID, canhBao);
    Serial.println(canhBao);
  }
}

// ===== TELEGRAM =====
void handleTelegramMessages()
{
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages)
  {
    for (int i = 0; i < numNewMessages; i++)
    {
      String chat_id = bot.messages[i].chat_id;
      String text = bot.messages[i].text;

      if (chat_id != CHAT_ID)
        continue;

      if (text == "/traffic_off")
      {
        trafficOn = false;
        digitalWrite(gPIN, LOW);
        digitalWrite(yPIN, LOW);
        digitalWrite(rPIN, LOW);
        bot.sendMessage(CHAT_ID, "✅ Đèn giao thông đã tắt!");
      }
      else if (text == "/traffic_on")
      {
        trafficOn = true;
        bot.sendMessage(CHAT_ID, "✅ Đèn giao thông hoạt động trở lại!");
      }
      else if (text == "/status")
      {
        String status = StringFormat("🌡 Nhiệt độ: %.1f°C\n💧 Độ ẩm: %.1f%%", temperature, humidity);
        bot.sendMessage(CHAT_ID, status);
      }
      else
      {
        bot.sendMessage(CHAT_ID, "❌ Lệnh không hợp lệ! Dùng: /traffic_on /traffic_off /status");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ===== OLED =====
void updateOLED()
{
  static unsigned long lastTimer = 0;
  if (!IsReady(lastTimer, 1000))
    return;

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, StringFormat("Nhiệt: %.1f C", temperature).c_str());
  oled.drawUTF8(0, 28, StringFormat("Độ ẩm: %.1f %%", humidity).c_str());

  unsigned long uptime = millis() / 1000;
  int hours = uptime / 3600;
  int minutes = (uptime % 3600) / 60;
  int seconds = uptime % 60;
  oled.drawUTF8(0, 42, StringFormat("Thời gian: %dh %02dm %02ds", hours, minutes, seconds).c_str());

  if (!yellowBlinkMode && trafficOn)
  {
    unsigned long elapsed = millis() - lastLedSwitchTime;
    int remainingTime = (durations[currentLedIndex] - elapsed) / 1000;
    if (remainingTime < 0) remainingTime = 0;

    String ledStr = (ledPin[currentLedIndex] == gPIN) ? "Xanh" : (ledPin[currentLedIndex] == yPIN) ? "Vàng" : "Đỏ";
    oled.drawUTF8(0, 56, StringFormat("%s: %ds", ledStr.c_str(), remainingTime).c_str());
  }
  oled.sendBuffer();
}

// ===== UPTIME BLYNK =====
void updateUptime()
{
  static unsigned long lastTimer = 0;
  if (!IsReady(lastTimer, 1000))
    return;

  unsigned long uptime = millis() / 1000;
  Blynk.virtualWrite(V0, uptime);
  Serial.print("Uptime (seconds): ");
  Serial.println(uptime);
}

// ===== BLYNK TỪ ỨNG DỤNG =====
BLYNK_WRITE(V3)
{
  yellowBlinkMode = param.asInt();
  if (yellowBlinkMode)
    digitalWrite(yPIN, HIGH);
  else
    digitalWrite(yPIN, LOW);
}

// ===== LOOP =====
void loop()
{
  Blynk.run();
  if (!WelcomeDisplayTimeout()) return;
  ThreeLedBlink();
  yellowBlink();
  updateRandomDHT();
  updateOLED();
  updateUptime();
  checkHealthConditions();
  handleTelegramMessages();
}