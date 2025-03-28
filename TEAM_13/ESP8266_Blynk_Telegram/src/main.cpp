#include <Arduino.h>
// Hoàng Thanh Nhã
#define BLYNK_TEMPLATE_ID "TMPL6uXaA1tg0"
#define BLYNK_TEMPLATE_NAME "Project"
#define BLYNK_AUTH_TOKEN "RuMutY_A_wRiTDm7BTcRJcWIpY89zDba"
#include "utils.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define OLED_SDA 13
#define OLED_SCL 12

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

// Telegram
const char* telegramBotToken = "7975958050:AAH9EdnSBVFB6R_9Qs8bTtwbISixWwHSot0";
const char* chatID = "-4720454162";

// Ngưỡng nhiệt độ & độ ẩm nguy hiểm
#define TEMP_MIN 10
#define TEMP_MAX 35
#define HUM_MIN 30
#define HUM_MAX 80

unsigned long lastAlertTime = 0; // Lưu thời gian lần gửi cảnh báo gần nhất
const unsigned long alertInterval = 5 * 60 * 1000; // 5 phút (ms)

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
bool yellowMode = false;

void sendTelegramMessage(String message) {
    WiFiClientSecure client;
    client.setInsecure(); 
    HTTPClient https;
    String url = "https://api.telegram.org/bot" + String(telegramBotToken) + "/sendMessage?chat_id=" + String(chatID) + "&text=" + message;
    https.begin(client, url);
    int httpCode = https.GET();
    https.end();

    if (httpCode > 0) {
        Serial.println("✅ Cảnh báo đã gửi đến Telegram!");
    } else {
        Serial.println("❌ Gửi cảnh báo thất bại!");
    }
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
    if (yellowMode) return;

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
    bool alertRequired = false; 

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

    // Kiểm tra nếu giá trị vượt ngưỡng nguy hiểm
    if (t < TEMP_MIN || t > TEMP_MAX || h < HUM_MIN || h > HUM_MAX) {
        alertRequired = true;
    }

    // Chỉ gửi cảnh báo mỗi 5 phút
    if (alertRequired && (millis() - lastAlertTime >= alertInterval)) {
        String message = "⚠️ CẢNH BÁO! Điều kiện môi trường không an toàn:\n";
        message += "🌡️ Nhiệt độ: " + String(t) + "°C\n";
        message += "💧 Độ ẩm: " + String(h) + "%";
        sendTelegramMessage(message);
        lastAlertTime = millis();
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
    yellowMode = (value == 1); 
    if (yellowMode) {
        digitalWrite(gPIN, LOW);
        digitalWrite(yPIN, HIGH);
        digitalWrite(rPIN, LOW);
    } else {
        digitalWrite(yPIN, LOW);
    }
    Serial.print("Yellow Mode: ");
    Serial.println(yellowMode ? "ON" : "OFF");
}

void loop() {
    Blynk.run();
    if (!yellowMode) {
        ThreeLedBlink();
    }
    updateDHT();
    DrawCounter();
}
