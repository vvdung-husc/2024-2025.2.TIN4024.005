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

// Thông tin Blynk (Lê Phước Quang)
// #define BLYNK_TEMPLATE_ID "TMPL6wQKPQ6OH"
// #define BLYNK_TEMPLATE_NAME "ESP8286 Project Blynk"
// #define BLYNK_AUTH_TOKEN "AyPfhrFYJN8w_ECXOODxvJpFVSDu5dEe"
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
// Thông tin Telegram (Lê Phước Quang)
// #define BOTtoken "7575921200:AAGyLJE132J4mUuTmqhb1P5budnX_11SPpQ"  // your Bot Token (Get from Botfather)
// #define GROUP_ID "-1002356539994" //là một số âm
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
void setup() {
    Serial.begin(115200);
    pinMode(LED_XANH, OUTPUT);
    pinMode(LED_VANG, OUTPUT);
    pinMode(LED_DO, OUTPUT);

    digitalWrite(LED_XANH, LOW);
    digitalWrite(LED_VANG, LOW);
    digitalWrite(LED_DO, LOW);

    dht.begin();
    Wire.begin(OLED_SDA, OLED_SCL);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    Serial.println(WiFi.status() == WL_CONNECTED ? "✅ WiFi kết nối thành công!" : "❌ Kết nối WiFi thất bại!");

    Blynk.virtualWrite(V3, nutNhan);
    oled.begin();
    oled.clearBuffer();

    oled.setFont(u8g2_font_unifont_t_vietnamese1);
    oled.drawUTF8(0, 14, "Trường Đại học Khoa học");
    oled.drawUTF8(0, 28, "Khoa Công Nghệ Thông Tin");
    oled.drawUTF8(0, 42, "Lập trình hệ thống IoT");
    oled.sendBuffer();

    client.setInsecure();
    startTime = millis();
    randomSeed(analogRead(0));
}

void chopTatCaDen() {
    static unsigned long thoiGianTruoc = 0;
    if (!trafficOn || nutNhan) {
        digitalWrite(LED_XANH, LOW);
        digitalWrite(LED_VANG, LOW);
        digitalWrite(LED_DO, LOW);
        return;
    }

    if (!IsReady(thoiGianTruoc, 1000)) return;
    
    if (nhietDo < 15) {
        digitalWrite(LED_DO, HIGH);
        digitalWrite(LED_VANG, LOW);
        digitalWrite(LED_XANH, LOW);
    } else if (nhietDo >= 15 && nhietDo <= 30) {
        digitalWrite(LED_DO, LOW);
        digitalWrite(LED_VANG, LOW);
        digitalWrite(LED_XANH, HIGH);
    } else {
        digitalWrite(LED_DO, LOW);
        digitalWrite(LED_VANG, HIGH);
        digitalWrite(LED_XANH, LOW);
    }
}
