#include <Arduino.h>
#include "utils.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <U8g2lib.h>
// //Lê Quang Khải
// #define BLYNK_TEMPLATE_ID "TMPL6JA7z9_KD"
// #define BLYNK_TEMPLATE_NAME "ESP8266 Project Blynk"
// #define BLYNK_AUTH_TOKEN "XeOcIK_VvI8815fDjcW4iTYbsysNE30z"
//Lê Phước Quang
// #define BLYNK_TEMPLATE_ID "TMPL6wQKPQ6OH"
// #define BLYNK_TEMPLATE_NAME "ESP8286 Project Blynk"
// #define BLYNK_AUTH_TOKEN "AyPfhrFYJN8w_ECXOODxvJpFVSDu5dEe"

//Lê Nguyễn Thiện Bình
#define BLYNK_TEMPLATE_ID "TMPL6BB21OMBX"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project Blynk"
#define BLYNK_AUTH_TOKEN "kgdfoQHneDMkL5gIAHWlL33IkVKT7pT3"

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
void chopTatCaDen() {
    static ulong thoiGianTruoc = 0;
    static int denHienTai = 0;
    static const int danhSachDen[3] = {LED_XANH, LED_VANG, LED_DO};

    if (!IsReady(thoiGianTruoc, 1000)) return;
    int denTruoc = (denHienTai + 2) % 3;
    digitalWrite(danhSachDen[denTruoc], LOW);
    digitalWrite(danhSachDen[denHienTai], HIGH);
    denHienTai = (denHienTai + 1) % 3;
}

float doAm = 0.0;
float nhietDo = 0.0;

void capNhatDHT() {
    static ulong thoiGianTruoc = 0;
    if (!IsReady(thoiGianTruoc, 2000)) return;

    float doAmMoi = random(0, 10001) / 100.0;
    float nhietDoMoi = random(-4000, 8001) / 100.0;

    if (isnan(doAmMoi) || isnan(nhietDoMoi)) {
        Serial.println("Lỗi đọc cảm biến DHT!");
        return;
    }

    bool canVe = false;

    if (nhietDo != nhietDoMoi) {
        canVe = true;
        nhietDo = nhietDoMoi;
        Serial.printf("Nhiệt độ: %.2f °C\n", nhietDo);
    }

    if (doAm != doAmMoi) {
        canVe = true;
        doAm = doAmMoi;
        Serial.printf("Độ ẩm: %.2f%%\n", doAm);
    }

    if (canVe) {
        oled.clearBuffer();
        oled.setFont(u8g2_font_unifont_t_vietnamese2);

        String chuoiNhietDo = StringFormat("Nhiệt độ: %.2f °C", nhietDo);
        oled.drawUTF8(0, 14, chuoiNhietDo.c_str());

        String chuoiDoAm = StringFormat("Độ ẩm: %.2f%%", doAm);
        oled.drawUTF8(0, 42, chuoiDoAm.c_str());

        oled.sendBuffer();
    }

    Blynk.virtualWrite(V1, nhietDo);
    Blynk.virtualWrite(V2, doAm);
}