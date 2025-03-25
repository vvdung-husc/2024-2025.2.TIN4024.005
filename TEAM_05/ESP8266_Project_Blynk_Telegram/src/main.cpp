#include <Arduino.h>
#include "utils.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>

#define BLYNK_TEMPLATE_ID "TMPL6BB21OMBX"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project Blynk"
#define BLYNK_AUTH_TOKEN "kgdfoQHneDMkL5gIAHWlL33IkVKT7pT3"

#define LED_XANH 15
#define LED_VANG 2
#define LED_DO 5

#define DHT_PIN 16
#define DHT_TYPE DHT11

#define OLED_SDA 13
#define OLED_SCL 12

#define BOTtoken "7889894611:AAEd-D67_v_MZ6uTQLoVSpcFq2doQDkTPro"
#define GROUP_ID "-1002525074425"

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";
bool nutNhan = false;
bool trafficOn = true;
float doAm = 0.0;
float nhietDo = 0.0;

void guiCanhBaoTelegram() {
    static ulong thoiGianTruoc = 0;
    if (!IsReady(thoiGianTruoc, 300000)) return;  // 5 phút
    
    String canhBao = "\u26A0\uFE0F Cảnh báo: ";
    bool gui = false;

    if (nhietDo < 10) {
        canhBao += "Nhiệt độ quá thấp! \n";
        gui = true;
    } else if (nhietDo > 35) {
        canhBao += "Nhiệt độ quá cao! \n";
        gui = true;
    }

    if (doAm < 30 || doAm > 70) {
        canhBao += "Độ ẩm không an toàn! \n";
        gui = true;
    }

    if (gui) bot.sendMessage(GROUP_ID, canhBao, "Markdown");
}

void docTinNhanTelegram() {
    int tinNhanMoi = bot.getUpdates(bot.last_message_received + 1);
    for (int i = 0; i < tinNhanMoi; i++) {
        String tinNhan = bot.messages[i].text;
        if (tinNhan == "/traffic_off") {
            trafficOn = false;
            digitalWrite(LED_XANH, LOW);
            digitalWrite(LED_VANG, LOW);
            digitalWrite(LED_DO, LOW);
            bot.sendMessage(GROUP_ID, "\u26D4 Đèn giao thông đã tắt", "Markdown");
        } else if (tinNhan == "/traffic_on") {
            trafficOn = true;
            bot.sendMessage(GROUP_ID, "\u2705 Đèn giao thông hoạt động trở lại", "Markdown");
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_XANH, OUTPUT);
    pinMode(LED_VANG, OUTPUT);
    pinMode(LED_DO, OUTPUT);

    dht.begin();
    Wire.begin(OLED_SDA, OLED_SCL);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    client.setInsecure();
    bot.sendMessage(GROUP_ID, "\u2728 Hệ thống IoT đã khởi động!", "Markdown");
}

void capNhatDHT() {
    static ulong thoiGianTruoc = 0;
    if (!IsReady(thoiGianTruoc, 2000)) return;

    nhietDo = dht.readTemperature();
    doAm = dht.readHumidity();

    if (!isnan(nhietDo) && !isnan(doAm)) {
        Serial.printf("Nhiệt độ: %.2f °C, Độ ẩm: %.2f%%\n", nhietDo, doAm);
        Blynk.virtualWrite(V1, nhietDo);
        Blynk.virtualWrite(V2, doAm);
    }
}

void chopTatCaDen() {
    if (!trafficOn) return;
    static ulong thoiGianTruoc = 0;
    static int denHienTai = 0;
    static const int danhSachDen[3] = {LED_XANH, LED_VANG, LED_DO};

    if (!IsReady(thoiGianTruoc, 1000)) return;
    int denTruoc = (denHienTai + 2) % 3;
    digitalWrite(danhSachDen[denTruoc], LOW);
    digitalWrite(danhSachDen[denHienTai], HIGH);
    denHienTai = (denHienTai + 1) % 3;
}

void loop() {
    capNhatDHT();
    guiCanhBaoTelegram();
    docTinNhanTelegram();
    chopTatCaDen();
    Blynk.run();
}
