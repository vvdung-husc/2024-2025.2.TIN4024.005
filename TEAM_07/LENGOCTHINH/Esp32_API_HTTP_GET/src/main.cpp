#include <WiFi.h>
#include <HTTPClient.h>

// 🔹 Cấu hình WiFi
const char* ssid = "Wokwi-GUEST";  // Nhập SSID WiFi của bạn
const char* pass = "";             // Nhập mật khẩu WiFi

// 🔹 URL API lấy thông tin địa chỉ IP và tọa độ
const char* apiURL = "http://ip4.iothings.vn/?geo=1";

// Hàm phân tích dữ liệu từ API
void parseData(String data) {
    String parts[7];  
    int index = 0, start = 0, end = 0;

    while ((end = data.indexOf('|', start)) != -1 && index < 6) {
        parts[index++] = data.substring(start, end);
        start = end + 1;
    }
    parts[index] = data.substring(start);

    // Lấy thông tin quan trọng
    String ip = parts[0];
    String latitude = parts[6];
    String longitude = parts[5];

    // Tạo link Google Maps
    String googleMapsUrl = "https://www.google.com/maps/place/" + latitude + "," + longitude;

    // Hiển thị dữ liệu trên Terminal
    Serial.println("\n=== 🌍 Thông tin từ API ===");
    Serial.println("🌐 IPv4: " + ip);
    Serial.println("🧭 Kinh độ (Longitude): " + longitude);
    Serial.println("🧭 Vĩ độ (Latitude): " + latitude);
    Serial.println("\n📍 Google Maps: " + googleMapsUrl);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n🔄 Đang kết nối WiFi...");

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi đã kết nối!");
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        Serial.println("\n🌍 Đang gửi yêu cầu HTTP...");

        http.begin(apiURL);
        int httpCode = http.GET();  // Gửi HTTP GET

        if (httpCode > 0) {
            String payload = http.getString();  // Nhận dữ liệu phản hồi từ API
            Serial.println("✅ Dữ liệu nhận được:");
            Serial.println(payload);
            parseData(payload);
        } else {
            Serial.println("❌ Lỗi HTTP, mã lỗi: " + String(httpCode));
        }

        http.end();  
    } else {
        Serial.println("⚠️ WiFi mất kết nối. Đang thử lại...");
        WiFi.disconnect();
        WiFi.reconnect();
    }

    delay(60000);  // Lặp lại sau 60 giây
}
