#define BLYNK_TEMPLATE_ID "TMPL6bUEnFZo4"
#define BLYNK_TEMPLATE_NAME "API"
#define BLYNK_AUTH_TOKEN "1W3kyplzoILnZJoTE7rYEclgsf0gy46T"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define OPENWEATHERMAP_KEY "fb2977fb8cd1c55fa98204e16923d62b"

struct LocationInfo { String latitude, longitude; } location;
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
    Serial.println(" Địa điểm: Huế, Việt Nam");
    Serial.printf(" Tọa độ: %s, %s\n", location.latitude.c_str(), location.longitude.c_str());

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
            Serial.printf("\U0001F321 Nhiệt độ ở Huế: %.2f°C\n", temperature);
        } else {
            Serial.println("\U0000274C JSON parse error!");
        }
    } else {
        Serial.println("\U0000274C Failed to get weather data!");
    }
    http.end();
}

void uptimeBlynk() {
    static ulong lastTime = 0;
    if (IsReady(lastTime, 1000)) Blynk.virtualWrite(V0, lastTime / 1000);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\U0001F680 Khởi động ESP32...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\n\U00002705 Kết nối WiFi thành công!");

    Blynk.config(BLYNK_AUTH_TOKEN);
    setupLocation();
}

void loop() {
    Blynk.run();
    currentMillis = millis();
    updateWeather();
    uptimeBlynk();
}
