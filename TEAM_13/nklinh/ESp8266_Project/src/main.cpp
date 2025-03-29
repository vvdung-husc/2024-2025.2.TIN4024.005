#include <Arduino.h>
#define BLYNK_TEMPLATE_ID "TMPL6uXaA1tg0"
#define BLYNK_TEMPLATE_NAME "Project"
#define BLYNK_AUTH_TOKEN "RuMutY_A_wRiTDm7BTcRJcWIpY89zDba"
#include "utils.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define OLED_SDA 13
#define OLED_SCL 12

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

bool yellowMode = false; // Biến kiểm tra chế độ đèn vàng

bool WelcomeDisplayTimeout(uint msSleep = 5000) {
    static ulong lastTimer = 0;
    static bool bDone = false;
    if (bDone) return true;
    if (!IsReady(lastTimer, msSleep)) return false;
    bDone = true;
    return bDone;
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, pass);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

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
    oled.drawUTF8(0, 42, "Hoàng Thanh Nhã");
    oled.sendBuffer();
}

void ThreeLedBlink() {
    if (yellowMode) return; // Nếu đang bật chế độ đèn vàng thì không chạy

    static ulong lastTimer = 0;
    static int currentLed = 0;
    static const int ledPin[3] = {gPIN, yPIN, rPIN};
    if (!IsReady(lastTimer, 1000)) return;
    int prevLed = (currentLed + 2) % 3;
    digitalWrite(ledPin[prevLed], LOW);
    digitalWrite(ledPin[currentLed], HIGH);
    currentLed = (currentLed + 1) % 3;
}

float fHumidity = 0.0;
float fTemperature = 0.0;

void updateDHT() {
    static ulong lastTimer = 0;
    if (!IsReady(lastTimer, 2000)) return;
    float h = random(0, 101) + random(0, 100) / 100.0;
    float t = random(-40, 101) + random(0, 100) / 100.0;
    bool bDraw = false;
    if (fTemperature != t) {
        bDraw = true;
        fTemperature = t;
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" *C");
        Blynk.virtualWrite(V1, t);
    }
    if (fHumidity != h) {
        bDraw = true;
        fHumidity = h;
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.println(" %");
        Blynk.virtualWrite(V2, h);
    }
    if (bDraw) {
        oled.clearBuffer();
        oled.setFont(u8g2_font_unifont_t_vietnamese2);
        String s = StringFormat("Nhiet do: %.2f °C", t);
        oled.drawUTF8(0, 14, s.c_str());
        s = StringFormat("Do am: %.2f %%", h);
        oled.drawUTF8(0, 42, s.c_str());
        oled.sendBuffer();
    }
}

void DrawCounter() {
    static uint counter = 0;
    static ulong lastTimer = 0;
    if (!IsReady(lastTimer, 2000)) return;
    Blynk.virtualWrite(V0, counter);
    counter++;
}

BLYNK_WRITE(V3) {
    int value = param.asInt();
    yellowMode = (value == 1); // Nếu nhận giá trị 1 từ Blynk, bật chế độ đèn vàng
    if (yellowMode) {
        // Bật chỉ đèn vàng
        digitalWrite(gPIN, LOW);
        digitalWrite(yPIN, HIGH);
        digitalWrite(rPIN, LOW);
    } else {
        // Quay lại chế độ đèn giao thông bình thường
        digitalWrite(yPIN, LOW);
    }
    Serial.print("Yellow Mode: ");
    Serial.println(yellowMode ? "ON" : "OFF");
}

void loop() {
    Blynk.run();
    if (!WelcomeDisplayTimeout()) return;
    if (!yellowMode) {
        ThreeLedBlink();
    }
    updateDHT();
    DrawCounter();
}
