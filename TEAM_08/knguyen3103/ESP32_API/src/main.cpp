#define BLYNK_TEMPLATE_ID "TMPL6UHKpBtum"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "72wuC5f1LMQsG1ix1XxAubCYgU-8-PWp"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define OPENWEATHERMAP_KEY "6de87aee08467a2cca84a01bb6894496"

struct LocationInfo { String latitude, longitude; } location;
String deviceIP;
ulong currentMillis = 0;

bool IsReady(ulong &timer, uint32_t interval) { 
    if (currentMillis - timer < interval) return false; 
    timer = currentMillis; return true;
}

String StringFormat(const char* fmt, ...) {
    char buff[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    va_end(args);
    return String(buff);
}

void setupLocation() {
    location = { "16.4637", "107.5909" };  // Tọa độ Huế
    Serial.println("🌍 Địa điểm: Huế, Việt Nam");
    Serial.printf("📍 Tọa độ: %s, %s\n", location.latitude.c_str(), location.longitude.c_str());

    Blynk.virtualWrite(V1, "Huế, Việt Nam");
    Blynk.virtualWrite(V2, StringFormat("https://www.google.com/maps/place/%s,%s", 
                                        location.latitude.c_str(), location.longitude.c_str()));
}

void updateWeather() {
    static ulong lastTime = 0;
    if (!IsReady(lastTime, 10000) || WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    String url = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", 
                              location.latitude.c_str(), location.longitude.c_str(), OPENWEATHERMAP_KEY);
    http.begin(url);
    
    int httpCode = http.GET();
    if (httpCode > 0) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            float temperature = doc["main"]["temp"].as<float>();
            Blynk.virtualWrite(V3, temperature);
            Serial.printf("🌡 Nhiệt độ ở Huế: %.2f°C\n", temperature);
        } else {
            Serial.println("❌ JSON parse error!");
        }
    } else {
        Serial.println("❌ Failed to get weather data!");
    }
    http.end();
}

void uptimeBlynk() {
    static ulong lastTime = 0;
    if (IsReady(lastTime, 1000)) Blynk.virtualWrite(V0, lastTime / 1000);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n🚀 Khởi động ESP32...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    deviceIP = WiFi.localIP().toString();
    Serial.printf("\n✅ Kết nối WiFi thành công! (IP: %s)\n", deviceIP.c_str());

    Blynk.config(BLYNK_AUTH_TOKEN);
    setupLocation();

    // Gửi địa chỉ IP lên Blynk
    Blynk.virtualWrite(V4, deviceIP);
}

void loop() {
    Blynk.run();
    currentMillis = millis();
    updateWeather();
    uptimeBlynk();
}
