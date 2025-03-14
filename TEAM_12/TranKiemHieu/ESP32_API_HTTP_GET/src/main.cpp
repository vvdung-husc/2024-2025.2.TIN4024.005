#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Wokwi-GUEST";     
const char* password = "";            
const char* serverUrl = "http://ip4.iothings.vn/?geo=1";

String latitude = "";
String longitude = "";

void PrasData(String data) {
    int firstSep = data.indexOf('|');
    int secondSep = data.indexOf('|', firstSep + 1);
    int thirdSep = data.indexOf('|', secondSep + 1);
    int fourthSep = data.indexOf('|', thirdSep + 1);
    int fifthSep = data.indexOf('|', fourthSep + 1);
    int sixthSep = data.indexOf('|', fifthSep + 1);

    if (firstSep > 0 && secondSep > firstSep && thirdSep > secondSep && 
        fourthSep > thirdSep && fifthSep > fourthSep && sixthSep > fifthSep) {
        String ip = data.substring(0, firstSep);
        String countryCode = data.substring(firstSep + 1, secondSep);
        String countryName = data.substring(secondSep + 1, thirdSep);
        String region = data.substring(thirdSep + 1, fourthSep);
        String district = data.substring(fourthSep + 1, fifthSep);
        longitude = data.substring(fifthSep + 1, sixthSep);  // Kinh độ
        latitude = data.substring(sixthSep + 1);             // Vĩ độ

        Serial.println("\n=== Thông tin từ API ===");
        Serial.println("IP: " + ip);
        Serial.println("Mã quốc gia: " + countryCode);
        Serial.println("Tên quốc gia: " + countryName);
        Serial.println("Tỉnh/Thành phố: " + region);
        Serial.println("Quận/Huyện: " + district);
        Serial.println("Kinh độ: " + longitude);
        Serial.println("Vĩ độ: " + latitude);
    } else {
        Serial.println("Failed to parse response");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Đang kết nối WiFi...");

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8)); // Google DNS
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi đã kết nối!");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Không có kết nối WiFi. Đang thử lại...");
        WiFi.disconnect();
        WiFi.reconnect();
        delay(5000);
        return;
    }

    HTTPClient http;
    Serial.println("\nĐang gửi yêu cầu HTTP...");
    http.begin(serverUrl);  
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();
        Serial.println("Dữ liệu nhận được:");
        Serial.println(payload);
        PrasData(payload);

        
        if (latitude != "" && longitude != "") {
            String googleMapsLink = "https://www.google.com/maps?q=" + latitude + "," + longitude;
            Serial.println("\nGoogle Maps Link:");
            Serial.println(googleMapsLink);
        }
    } else {
        Serial.println("Lỗi HTTP, mã lỗi: " + String(httpCode));
    }

    http.end();
    delay(60000);  
}