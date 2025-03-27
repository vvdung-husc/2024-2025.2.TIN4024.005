#define BLYNK_TEMPLATE_ID "TMPL6ab_8qXIk"
#define BLYNK_TEMPLATE_NAME "API"
#define BLYNK_AUTH_TOKEN "9NRAZIypVH-ZCLuZwW6rtwS82gEtI8lk"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// OpenWeatherMap API Key
const char* openWeatherApiKey = "f5df92a7b543a9c13030df5d3d92c0e1";

// Biến toàn cục
unsigned long startMillis = 0;
unsigned long uptimePreviousMillis = 0;
unsigned long weatherPreviousMillis = 0;
long uptimeInterval = 1000;
long weatherInterval = 60000;
float latitude = 0;
float longitude = 0;
float temperature = 0;
String ipAddress = "";

void getLocation();
void getWeather();
void updateUptime();

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    startMillis = millis();
    ipAddress = WiFi.localIP().toString();
    Serial.print("Địa chỉ IPv4: ");
    Serial.println(ipAddress);
    Blynk.virtualWrite(V1, ipAddress);
    
    Blynk.virtualWrite(V0, 0);
    getLocation();
    getWeather();
}

void getLocation() {
    HTTPClient http;
    http.begin("http://ip4.iothings.vn?geo=1");
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            if (doc["lat"].is<float>() && doc["lon"].is<float>()) {
                latitude = doc["lat"].as<float>();
                longitude = doc["lon"].as<float>();
                Serial.printf("Vị trí: lat=%.6f, lon=%.6f\n", latitude, longitude);
                Blynk.virtualWrite(V2, String(latitude, 6) + "," + String(longitude, 6));
            } else {
                Serial.println("Dữ liệu JSON không chứa tọa độ hợp lệ.");
            }
        } else {
            Serial.println("Lỗi phân tích JSON: " + String(error.c_str()));
        }
    } else {
        Serial.println("Lỗi khi lấy tọa độ từ API, HTTP Code: " + String(httpCode));
    }
    http.end();
}

void getWeather() {
    HTTPClient http;
    String url = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(latitude) + "&lon=" + String(longitude) + "&appid=" + openWeatherApiKey + "&units=metric";
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            if (doc["main"]["temp"].is<float>()) {
                temperature = doc["main"]["temp"].as<float>();
                Serial.printf("Nhiệt độ: %.2f°C\n", temperature);
                Blynk.virtualWrite(V3, temperature);
            } else {
                Serial.println("Dữ liệu JSON không chứa nhiệt độ hợp lệ.");
            }
        } else {
            Serial.println("Lỗi phân tích JSON: " + String(error.c_str()));
        }
    } else {
        Serial.println("Lỗi khi lấy dữ liệu thời tiết, HTTP Code: " + String(httpCode));
    }
    http.end();
}

void updateUptime() {
    unsigned long currentMillis = millis();
    if (currentMillis - uptimePreviousMillis >= uptimeInterval) {
        uptimePreviousMillis = currentMillis;
        unsigned long uptimeSeconds = (currentMillis - startMillis) / 1000;
        Blynk.virtualWrite(V0, uptimeSeconds);
        Serial.printf("Thời gian hoạt động: %lu giây\n", uptimeSeconds);
    }
}

void loop() {
    Blynk.run();
    updateUptime();

    unsigned long currentMillis = millis();
    if (currentMillis - weatherPreviousMillis >= weatherInterval) {
        weatherPreviousMillis = currentMillis;
        getLocation();
        getWeather();
    }
}