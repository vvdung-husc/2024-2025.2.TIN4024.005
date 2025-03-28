#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// WiFi Info
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Telegram Info
const char* TELEGRAM_BOT_TOKEN = "7405557746:AAGGWwmLWX3N5yjaeSVTItw3YNFs67hpox4";
const char* TELEGRAM_CHAT_ID = "-4691157407";

// Cảm biến
Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Kiểm tra WiFi
void checkWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠ Mất kết nối WiFi! Đang kết nối lại...");
        WiFi.disconnect();
        WiFi.reconnect();
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            delay(1000);
            Serial.print(".");
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✅ WiFi đã kết nối lại!");
        } else {
            Serial.println("\n❌ Không thể kết nối lại WiFi!");
        }
    }
}

// Gửi dữ liệu lên Telegram
void sendDataToTelegram(float ax, float ay, float az) {
    checkWiFi();
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String message = "📡 Dữ liệu MPU6050:\n";
        message += "📊 Gia tốc:\n";
        message += "   X: " + String(ax) + " m/s²\n";
        message += "   Y: " + String(ay) + " m/s²\n";
        message += "   Z: " + String(az) + " m/s²";

        String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) +
                     "/sendMessage?chat_id=" + String(TELEGRAM_CHAT_ID) +
                     "&text=" + message;
        
        http.begin(url);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.println("📩 Gửi dữ liệu lên Telegram thành công!");
        } else {
            Serial.println("❌ Lỗi gửi dữ liệu lên Telegram!");
        }
        http.end();
    } else {
        Serial.println("❌ Không có WiFi, không thể gửi tin nhắn Telegram.");
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi...");

    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 15) {
        delay(1000);
        Serial.print(".");
        attempt++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi Connected!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Connected");
    } else {
        Serial.println("\n❌ WiFi Không kết nối được!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Failed!");
    }

    Wire.begin(); // Khởi động I2C

    // Khởi tạo cảm biến MPU6050
    if (!mpu.begin()) {
        Serial.println("❌ Không tìm thấy MPU6050!");
        lcd.setCursor(0, 1);
        lcd.print("MPU6050 ERROR!");
        while (1);
    }

    Serial.println("✅ Hệ thống đã sẵn sàng!");
    lcd.setCursor(0, 1);
    lcd.print("System Ready!");
    delay(2000);
}

void loop() {
    checkWiFi();

    // Đọc dữ liệu từ MPU6050
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = a.acceleration.z;

    // Hiển thị dữ liệu trên LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("X: "); lcd.print(ax);
    lcd.setCursor(0, 1);
    lcd.print("Y: "); lcd.print(ay);
    
    // Gửi dữ liệu lên Telegram mỗi 15 giây
    static unsigned long lastReportTime = 0;
    if (millis() - lastReportTime > 15000) {
        sendDataToTelegram(ax, ay, az);
        lastReportTime = millis();
    }

    delay(1000);
}