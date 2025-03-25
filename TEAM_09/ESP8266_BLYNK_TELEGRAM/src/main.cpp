
#define BLYNK_TEMPLATE_ID "TMPL6G-_1Se-p"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "J1RzJuboKtIHo0ZCcegBupFpHC00XLDC"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>



#define gPIN 15
#define yPIN 2
#define rPIN 5
#define OLED_SDA 13
#define OLED_SCL 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
bool yellowBlinkMode = false;
bool trafficActive = true;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

#define TELEGRAM_BOT_TOKEN "7179261234:AAERMDKB_0mSa4SBp6tymQeOiYb6ebDHVJg"
#define CHAT_ID "-1002559266898"

WiFiClientSecure client;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, client);
unsigned long lastTelegramAlert = 0;

void setup() {
    Serial.begin(115200);
    pinMode(gPIN, OUTPUT);
    pinMode(yPIN, OUTPUT);
    pinMode(rPIN, OUTPUT);
    Wire.begin(OLED_SDA, OLED_SCL);
    oled.begin();

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    client.setInsecure();

    Blynk.begin(auth, ssid, pass);
    oledDisplayMessage("Truong ĐHKH\nKhoa CNTT\nLập trình IoT-NHOM09");
}

void oledDisplayMessage(String message) {
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese1);
    int y = 14;
    int start = 0;
    int end = message.indexOf("\n");
    while (end != -1) {
        oled.drawUTF8(0, y, message.substring(start, end).c_str());
        y += 14;
        start = end + 1;
        end = message.indexOf("\n", start);
    }
    if (start < message.length()) {
        oled.drawUTF8(0, y, message.substring(start).c_str());
    }
    oled.sendBuffer();
}

BLYNK_WRITE(V1) {
    yellowBlinkMode = param.asInt();
}

void ThreeLedBlink() {
    static ulong lastTimer = 0;
    static int currentLed = 0;
    static const int ledPin[3] = {gPIN, yPIN, rPIN};

    if (!trafficActive) {
        digitalWrite(gPIN, LOW);
        digitalWrite(yPIN, LOW);
        digitalWrite(rPIN, LOW);
        return;
    }

    if (millis() - lastTimer < 1000) return;
    lastTimer = millis();

    if (yellowBlinkMode) {
        digitalWrite(gPIN, LOW);
        digitalWrite(rPIN, LOW);
        digitalWrite(yPIN, !digitalRead(yPIN));
    } else {
        digitalWrite(ledPin[(currentLed + 2) % 3], LOW);
        digitalWrite(ledPin[currentLed], HIGH);
        currentLed = (currentLed + 1) % 3;
    }
}

void updateDHT() {
    static ulong lastTimer = 0;
    if (millis() - lastTimer < 2000) return;
    lastTimer = millis();

    float t = random(-400, 800) / 10.0;
    float h = random(0, 1000) / 10.0;

    Serial.printf("Temperature: %.2f \xC2\xB0C, Humidity: %.2f %%\n", t, h);

    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);
    oled.setCursor(0, 20);
    oled.printf("Nhiet do: %.2f \xC2\xB0C", t);
    oled.setCursor(0, 40);
    oled.printf("Do am: %.2f %%", h);
    oled.sendBuffer();

    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);
    sendAlertIfNeeded(t, h);
}

void sendAlertIfNeeded(float temp, float humidity) {
    if (millis() - lastTelegramAlert < 300000) return;

    String message = "";
    if (temp < 10) message += "⚠️ Cảnh báo: Nguy cơ hạ thân nhiệt!\n";
    else if (temp > 35) message += "⚠️ Cảnh báo: Nguy cơ sốc nhiệt!\n";
    if (humidity < 30) message += "⚠️ Độ ẩm thấp có thể gây kích ứng!\n";
    else if (humidity > 70) message += "⚠️ Độ ẩm cao làm tăng nguy cơ nấm mốc!\n";

    if (message != "") {
        bot.sendMessage(CHAT_ID, message, "");
        lastTelegramAlert = millis();
    }
}

void checkTelegramMessages() {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
        for (int i = 0; i < numNewMessages; i++) {
            String chat_id = bot.messages[i].chat_id;
            String text = bot.messages[i].text;

            if (chat_id != CHAT_ID) continue;

            if (text == "/traffic_off") {
                trafficActive = false;
                digitalWrite(gPIN, LOW);
                digitalWrite(yPIN, LOW);
                digitalWrite(rPIN, LOW);
                bot.sendMessage(CHAT_ID, "🚦 Đèn giao thông đã tắt!", "");
            } else if (text == "/traffic_on") {
                trafficActive = true;
                bot.sendMessage(CHAT_ID, "🚦 Đèn giao thông hoạt động trở lại!", "");
            } else {
                bot.sendMessage(CHAT_ID, "❓ Lệnh không hợp lệ. Hãy thử:\n/traffic_off - Tắt đèn\n/traffic_on - Bật đèn", "");
            }
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
}

void loop() {
    Blynk.run();
    ThreeLedBlink();
    updateDHT();
    checkTelegramMessages();
}
