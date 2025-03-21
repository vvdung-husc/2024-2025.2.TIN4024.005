#include <Arduino.h>
#include "utils.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <U8g2lib.h>
//Lê Quang Khải
#define BLYNK_TEMPLATE_ID "TMPL6JA7z9_KD"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project Blynk"
#define BLYNK_AUTH_TOKEN "XeOcIK_VvI8815fDjcW4iTYbsysNE30z"
//Lê Phước Quang
#define BLYNK_TEMPLATE_ID "TMPL6wQKPQ6OH"
#define BLYNK_TEMPLATE_NAME "ESP8286 Project Blynk"
#define BLYNK_AUTH_TOKEN "AyPfhrFYJN8w_ECXOODxvJpFVSDu5dEe"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>

#define LED_XANH 15
#define LED_VANG 2
#define LED_DO 5

#define DHT_PIN 16
#define DHT_TYPE DHT11

#define OLED_SDA 13
#define OLED_SCL 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

DHT dht(DHT_PIN, DHT_TYPE);

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

bool nutNhan = false;

bool KhoangThoiGianHienThi(uint tgCho = 5000) {
    static ulong thoiGianTruoc = 0;
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
    randomSeed(analogRead(0));
}
