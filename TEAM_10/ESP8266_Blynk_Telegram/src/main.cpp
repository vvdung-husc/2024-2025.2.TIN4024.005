#include <Arduino.h>
#include "utils.h"
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>

// Lê Văn Minh Toàn config
// #define BLYNK_TEMPLATE_ID "TMPL6J5vyJ-Sx"
// #define BLYNK_TEMPLATE_NAME "8266Project"
// #define BLYNK_AUTH_TOKEN "vh8uS_cSevS3c-lhJ3SA_hpkaHIhjDTY"
// #define BLYNK_AUTH_TOKEN "KvjwSKBaTdKY_5HIcugiNpTHfJ8cU86X"
// #define BOT_TOKEN "7472915762:AAG6Pol5VV1yNYh-NfU295aDKScjwkut2CQ"
// #define TELEGRAM_GROUP_ID "-4603839699" 

// Phan Thanh Toàn
// #define BLYNK_TEMPLATE_ID "TMPL6UtUU_5WZ"
// #define BLYNK_TEMPLATE_NAME "ESP8266Project"
// #define BLYNK_AUTH_TOKEN "p_B7I5wU_xTCi3owpkhbeklgLbr7r-8R"
// #define BOT_TOKEN "7499637616:AAGVz985ubbTrkQlWYRrJMTNC70be_QVP7k"
// #define TELEGRAM_GROUP_ID "-4698442702"

// Lê Trọng Tùng Lâm
// #define BLYNK_TEMPLATE_ID "TMPL61OYx_d4N"
// #define BLYNK_TEMPLATE_NAME "ESP8266blynkbottelegram"
// #define BLYNK_AUTH_TOKEN "6nbBw_6ai4_UZ60oPZYrB5NBoSzTOqvn"
// #define BOT_TOKEN "7813666133:AAGlXB9w4ZkrJ6jP5h7VYiQgzcX36xf4bBM"
// #define TELEGRAM_GROUP_ID "-4698442702"

// Nguyễn Văn Hoàng Nam
#define BLYNK_TEMPLATE_ID "TMPL6aruMTIqM"
#define BLYNK_TEMPLATE_NAME "Esp8266 Project Blynk Telegram bot"
#define BLYNK_AUTH_TOKEN "aJmjzlwxzsxlSF6AIEl-9So-gM1H6rw_"
#define BOT_TOKEN "7499637616:AAGVz985ubbTrkQlWYRrJMTNC70be_QVP7k"
#define TELEGRAM_GROUP_ID "-4698442702"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define LED_GREEN 15
#define LED_YELLOW 2  
#define LED_RED 5 

#define SENSOR_PIN 16
#define SENSOR_TYPE DHT11 

#define OLED_SDA_PIN 13
#define OLED_SCL_PIN 12   

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

DHT dht(SENSOR_PIN, SENSOR_TYPE);
// char wifiSSID[] = "Workwi-guest";
// char wifiPASS[] = "";

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

// WiFiClientSecure secureClient;
// UniversalTelegramBot telegramBot(BOT_TOKEN, secureClient);

WiFiClientSecure client;
UniversalTelegramBot telegramBot(BOT_TOKEN, client);

// Hàm kiểm tra thời gian hiển thị màn hình chào mừng
bool DisplayWelcomeTimeout(unsigned long timeoutMS = 5000) 
{
  static unsigned long lastTime = 0;

  static bool done = false;
  if (done) 
    return true;
  if (!IsReady(lastTime, timeoutMS)) 
    return false;
  done = true;
  return done;
}

int currentState = 0;           // Trạng thái hiện tại: 0 - Xanh, 1 - Vàng, 2 - Đỏ
unsigned long previousMillis = 0; // Thời gian trước đó (dùng để tính thời gian)
unsigned long remainingTime = 0;  // Thời gian còn lại của trạng thái hiện tại
unsigned long greenTime = 15000;  
unsigned long yellowTime = 2000;  
unsigned long redTime = 5000;     
bool BlinkYellow = false;     // Chế độ nhấp nháy đèn vàng
bool trafficOff = false;          // Cờ để kiểm soát trạng thái đèn giao thông
bool trafficOffByTelegram = false; // Cờ để kiểm tra xem đèn bị tắt bởi Telegram hay không

float randomFloat(float min, float max) {
  return min + (float)random(0, 1000) / 1000.0 * (max - min);
}

bool IsReady(unsigned long &lastTime, unsigned long interval) {
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= interval) {
    lastTime = currentTime;
    return true;
  }
  return false;
}

void setup() 
{
  Serial.begin(115200);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  dht.begin();

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  client.setInsecure();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("❌ WiFi connection failed!");
  }
  else
  {
    Serial.println("✅ WiFi connected!");
  }
  Blynk.virtualWrite(V3, BlinkYellow);
  oled.begin();
  oled.clearBuffer();

  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT");

  oled.sendBuffer();
  client.setInsecure(); // Bỏ qua kiểm tra chứng chỉ SSL cho ESP8266

  

  // Kết nối với Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  previousMillis = millis();  // Khởi tạo thời gian ban đầu
  remainingTime = greenTime;  // Khởi tạo thời gian còn lại

  randomSeed(analogRead(0));
  telegramBot.sendMessage(TELEGRAM_GROUP_ID, "ESP8266 đã khởi động!", "");
}

void CycleTrafficLights() 
{
  if (BlinkYellow || trafficOff) return; // Không chạy nếu ở chế độ nhấp nháy hoặc đèn bị tắt

  unsigned long currentLight = millis();
  unsigned long interval = (currentState == 0) ? greenTime : (currentState == 1) ? yellowTime : redTime;

  // Tính thời gian còn lại
  remainingTime = interval - (currentLight - previousMillis);

  if (currentLight - previousMillis >= interval) {
    previousMillis = currentLight;
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);

    switch (currentState) {
      case 0:  // Xanh -> Vàng
        digitalWrite(LED_YELLOW, HIGH);
        currentState = 1;
        break;
      case 1:  // Vàng -> Đỏ
        digitalWrite(LED_RED, HIGH);
        currentState = 2;
        break;
      case 2:  // Đỏ -> Xanh
        digitalWrite(LED_GREEN, HIGH);
        currentState = 0;
        break;
    }
    remainingTime = interval;  // Đặt lại thời gian còn lại khi chuyển trạng thái
    Serial.print("[TRAFFIC] ");
    Serial.println(currentState == 0 ? "Xanh" : currentState == 1 ? "Vang" : "Do");
  }
}

float humidityVal = 0.0;
float temperatureVal = 0.0;

void ReadSensorData() 
{
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 10000)) 
    return;

  float humidity = randomFloat(0.0, 100.0);
  float temperature = randomFloat(-40.0, 80.0);
  


  bool shouldUpdate = false;
  String message = "Cập nhật dữ liệu:\n";

  if (temperatureVal != temperature) 
  {
    shouldUpdate = true;
    temperatureVal = temperature;
    Serial.print("Nhiet do: ");
    Serial.print(temperature);
    Serial.println(" *C");
    Blynk.virtualWrite(V1, temperature);
    message += "🔥 Nhiệt độ: " + String(temperatureVal, 1) + "°C\n";
  }

  if (humidityVal != humidity) 
  {
    shouldUpdate = true;
    humidityVal = humidity;
    Serial.print("Do am: ");
    Serial.print(humidity);
    Serial.println(" %\t");
    Blynk.virtualWrite(V2, humidityVal);
    message += "💦 Độ ẩm: " + String(humidityVal, 1) + "%\n";
  }
  if (shouldUpdate) 
  {
    if (temperatureVal < 10) {
      message += "🔥 CẢNH BÁO: Nhiệt độ quá thấp (" + String(temperatureVal) + "°C)! Nguy cơ hạ thân nhiệt, tê cóng, giảm miễn dịch.\n";
    } else if (temperatureVal >= 10 && temperatureVal <= 15) {
      message += "🔥 CẢNH BÁO: Nhiệt độ thấp (" + String(temperatureVal) + "°C)! Cảm giác lạnh, tăng nguy cơ mắc bệnh đường hô hấp.\n";
    } else if (temperatureVal > 25 && temperatureVal <= 30) {
      message += "🔥 Nhiệt độ bình thường (" + String(temperatureVal) + "°C)! Nhiệt độ lý tưởng, ít ảnh hưởng đến sức khỏe.\n";
    } else if (temperatureVal > 30 && temperatureVal <= 35) {
      message += "🔥 CẢNH BÁO: Nhiệt độ cao (" + String(temperatureVal) + "°C)! Cơ thể có dấu hiệu mất nước, mệt mỏi.\n";
    } else if (temperatureVal > 35 && temperatureVal <= 40) {
      message += "🔥 CẢNH BÁO: Nhiệt độ rất cao (" + String(temperatureVal) + "°C)! Nguy cơ sốc nhiệt, chuột rút, say nắng.\n";
    } else if (temperatureVal > 40) {
      message += "🔥 CẢNH BÁO: Nhiệt độ cực kỳ nguy hiểm (" + String(temperatureVal) + "°C)! Nguy cơ suy nội tạng, đột quỵ nhiệt.\n";
    }
    if (humidityVal < 30) {
      message += "💦 CẢNH BÁO: Độ ẩm quá thấp (" + String(humidityVal) + "%)! Da khô, kích ứng mắt, tăng nguy cơ mắc bệnh về hô hấp.\n";
    } else if (humidityVal >= 40 && humidityVal <= 60) {
      message += "💦Độ ẩm bình thường (" + String(humidityVal) + "%)! Mức lý tưởng, ít ảnh hưởng đến sức khỏe.\n";
    } else if (humidityVal > 70 && humidityVal <= 80) {
      message += "💦 CẢNH BÁO: Độ ẩm cao (" + String(humidityVal) + "%)! Tăng nguy cơ nấm mốc, vi khuẩn phát triển, gây bệnh đường hô hấp.\n";
    } else if (humidityVal > 80) {
      message += "💦 CẢNH BÁO: Độ ẩm rất cao (" + String(humidityVal) + "%)! Oi bức, khó thở, tăng nguy cơ sốc nhiệt.\n";
    }

    telegramBot.sendMessage(TELEGRAM_GROUP_ID, message, "");
  }
}
void BlinkYellowMode() 
{ 
  static bool yellowBlink = false;
  static unsigned long lastBlinkTime = 0;
  if (!BlinkYellow || trafficOff) 
    return;
  if (!IsReady(lastBlinkTime, 1000)) 
    return;
  yellowBlink = !yellowBlink;
  digitalWrite(LED_YELLOW, yellowBlink ? HIGH : LOW);

  
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
}

void UpdateUptimeBlynk() 
{
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 1000)) 
    return;
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);
  String s;

  // Hiển thị trạng thái đèn giao thông
  if (trafficOff) {
    s = "Den tat(Tele)";
    s = "Den vang"; // Hiển thị thông báo nếu ở chế độ nhấp nháy
  } else {
    int elapsedTime = remainingTime / 1000; // Tính thời gian còn lại (giây)
    if (currentState == 0) s = "Den xanh: " + String(elapsedTime) + "s";
    else if (currentState == 1) s = "Den vang: " + String(elapsedTime) + "s";
    else s = "Den do: " + String(elapsedTime) + "s";
    // Gửi thời gian đếm ngược lên Blynk V0
    Blynk.virtualWrite(V0, elapsedTime);
  }
  oled.drawUTF8(0, 12, s.c_str());

  // Hiển thị nhiệt độ và độ ẩm
  s = "Nhiet do: " + String(temperatureVal, 1) + "C";
  oled.drawUTF8(0, 48, s.c_str());
  s = "Do am: " + String(humidityVal, 1) + "%";
  oled.drawUTF8(0, 60, s.c_str());
  oled.sendBuffer();
}

BLYNK_WRITE(V3) 
{
  BlinkYellow = param.asInt();
  if (!BlinkYellow) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    currentState = 0;  // Đặt lại trạng thái về đèn xanh khi tắt chế độ nhấp nháy
    previousMillis = millis();  // Đặt lại thời gian
    digitalWrite(LED_GREEN, HIGH);  // Bật đèn xanh ngay lập tức
    remainingTime = greenTime;  // Đặt lại thời gian còn lại
    trafficOff = false; // Bật lại đèn giao thông nếu đang bị tắt
    trafficOffByTelegram = false; // Xóa đánh dấu tắt bởi Telegram
  }
}

void ProcessTelegramCommands() 
{
  int msgCount = telegramBot.getUpdates(telegramBot.last_message_received + 1);
  if (msgCount == 0)
  return;
  while (msgCount) 
  {
    for (int i = 0; i < msgCount; i++) 
    {
      String command = telegramBot.messages[i].text;
      Serial.println("Received from Telegram: " + command);
      String chat_id = String(telegramBot.messages[i].chat_id);
      

      if (command == "/traffic_off") {
        trafficOff = true;
        trafficOffByTelegram = true;
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);
        telegramBot.sendMessage(chat_id, "Đèn giao thông đã được tắt.", "");
      } else if (command == "/traffic_on") {
        trafficOff = false;
        trafficOffByTelegram = false; // Xóa đánh dấu tắt bởi Telegram
        currentState = 0; // Đặt lại trạng thái về đèn xanh
        previousMillis = millis();
        digitalWrite(LED_GREEN, HIGH);
        remainingTime = greenTime;
        telegramBot.sendMessage(chat_id, "Đèn giao thông đã được bật.", "");
      } else {
        telegramBot.sendMessage(chat_id, "Dùng lệnh /traffic_off để tắt đèn hoặc /traffic_on để bật đèn.", "");
      }
    }
    msgCount = telegramBot.getUpdates(telegramBot.last_message_received + 1);
  }
}


// Hàm xử lý khi nhận giá trị từ Blynk V4 (thời gian đèn xanh)
BLYNK_WRITE(V4) {
  greenTime = param.asInt() * 1000;
  if (greenTime < 1000) greenTime = 1000;
  if (greenTime > 30000) greenTime = 30000;
}

// Hàm xử lý khi nhận giá trị từ Blynk V5 (thời gian đèn vàng)
BLYNK_WRITE(V5) {
  yellowTime = param.asInt() * 1000;
  if (yellowTime < 1000) yellowTime = 1000;
  if (yellowTime > 10000) yellowTime = 10000;
}

// Hàm xử lý khi nhận giá trị từ Blynk V6 (thời gian đèn đỏ)
BLYNK_WRITE(V6) {
  redTime = param.asInt() * 1000;
  if (redTime < 1000) redTime = 1000;
  if (redTime > 30000) redTime = 30000;
}

void loop() 
{
  Blynk.run();
  if (!DisplayWelcomeTimeout())
    return;

  CycleTrafficLights(); 
  ReadSensorData();
  UpdateUptimeBlynk();
  BlinkYellowMode(); 
  ProcessTelegramCommands();
}