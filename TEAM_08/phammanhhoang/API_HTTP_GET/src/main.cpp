#include <WiFi.h>
#include <HTTPClient.h>

char ssid[] = "Wokwi-GUEST";  
char pass[] = "";  

void PrasData(String data) {
    String parts[7];  
    int index = 0, start = 0, end = 0;

    while ((end = data.indexOf('|', start)) != -1 && index < 6) {
        parts[index++] = data.substring(start, end);
        start = end + 1;
    }
    parts[index] = data.substring(start);

    String latitude = parts[6];
    String longitude = parts[5];
    String googleMapsUrl = "http://www.google.com/maps/place/" + latitude + "," + longitude;

    Serial.println("\n=== Thông tin từ API ===");
    Serial.println("IP: " + parts[0]);
    Serial.println("Mã quốc gia: " + parts[1]);
    Serial.println("Tên quốc gia: " + parts[2]);
    Serial.println("Tỉnh/Thành phố: " + parts[3]);
    Serial.println("Quận/Huyện: " + parts[4]);
    Serial.println("Kinh độ: " + longitude);
    Serial.println("Vĩ độ: " + latitude);
    Serial.println("\n🌍 Google Maps Link: " + googleMapsUrl);
}

void setup() {
    Serial.begin(115200);
    Serial.println("🔄 Đang kết nối WiFi...");

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8)); // Google DNS
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi đã kết nối!");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ Không có kết nối WiFi. Đang thử lại...");
        WiFi.disconnect();
        WiFi.reconnect();
        delay(5000);
        return;
    }

    HTTPClient http;
    String url = "http://ip4.iothings.vn/?geo=1"; 

    Serial.println("\n🌍 Đang gửi yêu cầu HTTP...");
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();  
        Serial.println("✅ Dữ liệu nhận được:");
        Serial.println(payload);
        PrasData(payload);  
    } else {
        Serial.println("❌ Lỗi HTTP, mã lỗi: " + String(httpCode));
    }

    http.end();  
    delay(60000);
}