#include <Arduino.h>
#include "utils.h" 
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>

// Thông tin Blynk (Ngô Văn Hiếu)
#define BLYNK_TEMPLATE_ID "TMPL6KLUX0g8k"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project Blynk"
#define BLYNK_AUTH_TOKEN "OPO0M5x-ooILRht8BKrYJafoq6OTBJYY"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>

// Thông tin WiFi
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

// Thông tin Telegram (Ngô Văn Hiếu)
#define BOT_TOKEN "8184771014:AAEoqRHKjOhevsrds3CD-F54lkpoY3IoW24"
#define GROUP_ID "-1002655884696" // Nhóm "ESP32-Iot"

// Định nghĩa chân
#define LED_XANH 15 // D8
#define LED_VANG 2  // D4
#define LED_DO 5    // D1
#define DHT_PIN 16  // D0
#define DHT_TYPE DHT11
#define OLED_SDA 13 // D7
#define OLED_SCL 12 // D6

// Khai báo đối tượng
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Biến toàn cục
float doAm = 0.0;
float nhietDo = 0.0;
bool nutNhan = false;
bool trafficOn = true;
unsigned long startTime;

bool KhoangThoiGianHienThi(uint tgCho = 5000) {
    static unsigned long thoiGianTruoc = 0;
    static bool hoanTat = false;
    if (hoanTat) return true;
    if (!IsReady(thoiGianTruoc, tgCho)) return false;
    hoanTat = true;
    return hoanTat;
}

