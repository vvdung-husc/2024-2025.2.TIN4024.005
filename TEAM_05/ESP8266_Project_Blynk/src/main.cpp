#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5
#define OLED_SDA 13
#define OLED_SCL 12

// Blynk credentials
#define BLYNK_TEMPLATE_ID "TMPL6BB21OMBX"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project Blynk"
#define BLYNK_AUTH_TOKEN "kgdfoQHneDMkL5gIAHWlL33IkVKT7pT3"

// WiFi credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
bool yellowBlinkMode = false;
float fHumidity = 0.0, fTemperature = 0.0;
unsigned long startTime;

void setup() {
    Serial.begin(115200);
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
    oled.drawUTF8(0, 42, "Lê Nguyễn Thiện Bình");  
    oled.sendBuffer();
    
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    startTime = millis();
}

// Nhấp nháy đèn giao thông
void TrafficLightControl() {
    static unsigned long lastTimer = 0;
    static int currentLed = 0;
    const int ledPin[3] = {gPIN, yPIN, rPIN};
    if (millis() - lastTimer < 1000 || yellowBlinkMode) return;
    lastTimer = millis();
    digitalWrite(ledPin[(currentLed + 2) % 3], LOW);
    digitalWrite(ledPin[currentLed], HIGH);
    currentLed = (currentLed + 1) % 3;
}

// Chế độ đèn vàng nhấp nháy
void YellowBlinkMode() {
    static unsigned long lastTimer = 0;
    if (millis() - lastTimer < 500) return;
    lastTimer = millis();
    digitalWrite(yPIN, !digitalRead(yPIN));
}

// Gửi dữ liệu lên Blynk
void SendDataToBlynk() {
    static unsigned long lastTimer = 0;
    if (millis() - lastTimer < 2000) return;
    lastTimer = millis();
    
    float h = random(0, 101) + random(0, 100) / 100.0;
    float t = random(-40, 101) + random(0, 100) / 100.0;
    fHumidity = h;
    fTemperature = t;
    
    Blynk.virtualWrite(V1, fTemperature);
    Blynk.virtualWrite(V2, fHumidity);
    Blynk.virtualWrite(V0, millis() / 1000);
    
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);
    String s = String("Nhiet do: ") + String(t) + " °C";
    oled.drawUTF8(0, 14, s.c_str());  
    s = String("Do am: ") + String(h) + " %";
    oled.drawUTF8(0, 42, s.c_str());      
    oled.sendBuffer();
}

// Nhận dữ liệu từ Blynk (Switch V3)
BLYNK_WRITE(V3) {
    yellowBlinkMode = param.asInt();
    if (yellowBlinkMode) {
        digitalWrite(gPIN, LOW);
        digitalWrite(rPIN, LOW);
    } else {
        digitalWrite(yPIN, LOW);
    }
}

void loop() {
    Blynk.run();
    SendDataToBlynk();
    if (yellowBlinkMode) {
        YellowBlinkMode();
    } else {
        TrafficLightControl();
    }
}
