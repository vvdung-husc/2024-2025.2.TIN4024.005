#include <Arduino.h>
#include "utils.h" // Lấy từ TEAM_00/vvdung
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>

// Thông tin Blynk (Mai Đức Đạt)
#define BLYNK_TEMPLATE_ID "TMPL6MmuiU_Zh"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project Blynk"
#define BLYNK_AUTH_TOKEN "9rsoZ9K9ybKhcSz3_bFesZD7c7MQMDJ8"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>

// Thông tin WiFi
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

// Thông tin Telegram
#define BOT_TOKEN "8089088162:AAE3MUHKx6fh7-02USstnWi61UZ72rWH4kw"
#define GROUP_ID "-4644210824" // Nhóm "ESP32-Iot"

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

void capNhatDHT() {
    static unsigned long thoiGianTruoc = 0;
    if (!IsReady(thoiGianTruoc, 2000)) return;

    float doAmMoi = random(0, 10001) / 100.0;     // 0.0 đến 100.0
    float nhietDoMoi = random(-4000, 8001) / 100.0; // -40.0 đến 80.0

    if (isnan(doAmMoi) || isnan(nhietDoMoi)) {
        Serial.println("Lỗi sinh số ngẫu nhiên!");
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

void chopDenVang() {
    static bool trangThaiDenVang = false;
    static unsigned long thoiGianTruoc = 0;
    if (IsReady(thoiGianTruoc, 500)) {
        trangThaiDenVang = !trangThaiDenVang;
        digitalWrite(LED_VANG, trangThaiDenVang);
    }
    digitalWrite(LED_XANH, LOW);
    digitalWrite(LED_DO, LOW);
}

void guiThoiGianLenBlynk() {
    static unsigned long thoiGianTruoc = 0;
    if (!IsReady(thoiGianTruoc, 1000)) return;
    unsigned long giaTri = (millis() - startTime) / 1000;
    Blynk.virtualWrite(V0, giaTri);
}

void sendTelegramAlert() {
    static unsigned long thoiGianTruoc = 0;
    if (!IsReady(thoiGianTruoc, 300000)) return; // 5 phút

    String message = "";
    if (nhietDo < 10) message += "⚠️ Nguy cơ hạ thân nhiệt!\n";
    else if (nhietDo > 35) message += "⚠️ Nguy cơ sốc nhiệt!\n";
    else if (nhietDo > 40) message += "⚠️ Cực kỳ nguy hiểm!\n";

    if (doAm < 30) message += "⚠️ Độ ẩm thấp, nguy cơ bệnh hô hấp!\n";
    else if (doAm > 70) message += "⚠️ Độ ẩm cao, nguy cơ nấm mốc!\n";
    else if (doAm > 80) message += "⚠️ Nguy cơ sốc nhiệt do độ ẩm!\n";

    if (message != "") {
        message = "Cảnh báo:\n" + message + 
                 "Nhiệt độ: " + String(nhietDo) + "°C\n" +
                 "Độ ẩm: " + String(doAm) + "%";
        bot.sendMessage(GROUP_ID, message, "");
    }
}

void handleTelegram() {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;

        if (text == "/traffic_off") {
            trafficOn = false;
            nutNhan = false;
            bot.sendMessage(chat_id, "Đèn giao thông đã tắt", "");
        }
        else if (text == "/traffic_on") {
            trafficOn = true;
            bot.sendMessage(chat_id, "Đèn giao thông đã bật", "");
        }
    }
}

BLYNK_WRITE(V3) {
    nutNhan = param.asInt();
}

void loop() {
    Blynk.run();
    if (!KhoangThoiGianHienThi()) return;
    chopTatCaDen();
    capNhatDHT();
    guiThoiGianLenBlynk();
    sendTelegramAlert();
    handleTelegram();
    if (nutNhan) {
        chopDenVang();
    }
}