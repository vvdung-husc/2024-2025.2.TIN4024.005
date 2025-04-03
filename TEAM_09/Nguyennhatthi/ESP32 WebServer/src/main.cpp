#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// Thông tin WiFi
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Cấu hình các thiết bị IoT
struct Device {
    const char* id;
    const char* name;
    int pin;
    String state;
};

Device devices[] = {
    {"led", "LED Light", 2, "OFF"},    // GPIO2 cho đèn LED
    {"fan", "Fan", 4, "OFF"},          // GPIO4 cho quạt
    {"pump", "Water Pump", 5, "OFF"}   // GPIO5 cho máy bơm
};
const int numDevices = sizeof(devices) / sizeof(devices[0]);

// Tạo đối tượng server
AsyncWebServer server(80);

void setup() {
    Serial.begin(115200);

    // Khởi tạo các chân GPIO
    for (int i = 0; i < numDevices; i++) {
        pinMode(devices[i].pin, OUTPUT);
        digitalWrite(devices[i].pin, LOW); // Tắt tất cả thiết bị ban đầu
    }

    // Khởi tạo SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    // Kết nối WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Route: Trang chính
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });

    // Route: Lấy danh sách thiết bị (JSON)
    server.on("/devices", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "[";
        for (int i = 0; i < numDevices; i++) {
            json += "{\"id\":\"" + String(devices[i].id) + "\",";
            json += "\"name\":\"" + String(devices[i].name) + "\",";
            json += "\"state\":\"" + devices[i].state + "\"}";
            if (i < numDevices - 1) json += ",";
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    // Route: Điều khiển thiết bị
    server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("id") && request->hasParam("state")) {
            String id = request->getParam("id")->value();
            String state = request->getParam("state")->value();

            for (int i = 0; i < numDevices; i++) {
                if (id == devices[i].id) {
                    if (state == "ON") {
                        digitalWrite(devices[i].pin, HIGH);
                        devices[i].state = "ON";
                    } else if (state == "OFF") {
                        digitalWrite(devices[i].pin, LOW);
                        devices[i].state = "OFF";
                    }
                    request->send(200, "text/plain", devices[i].state);
                    return;
                }
            }
            request->send(404, "text/plain", "Device not found");
        } else {
            request->send(400, "text/plain", "Invalid request");
        }
    });

    // Khởi động server
    server.begin();
}

void loop() {
    // Không cần xử lý trong loop
}