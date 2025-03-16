#include <WiFi.h>
#include <HTTPClient.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

// URL API để lấy thông tin IP và vị trí
const char* api_url = "http://ip4.iothings.vn/?geo=1";

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Kết nối WiFi
    WiFi.begin(ssid, pass);
    Serial.print("Đang kết nối WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi đã kết nối!");
    
    // Gửi yêu cầu HTTP GET
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(api_url);
        int httpCode = http.GET();
        
        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println("Dữ liệu nhận được:");
            Serial.println(payload);
            
            // Tách dữ liệu theo dấu '|'
            int index1 = payload.indexOf("|");
            int index2 = payload.indexOf("|", index1 + 1);
            int index3 = payload.indexOf("|", index2 + 1);
            int index4 = payload.indexOf("|", index3 + 1);
            int index5 = payload.indexOf("|", index4 + 1);
            int index6 = payload.indexOf("|", index5 + 1);

            if (index1 != -1 && index6 != -1) {
                String ip = payload.substring(0, index1);
                String latitude = payload.substring(index6 + 1, payload.indexOf("|", index6 + 1));
                String longitude = payload.substring(index5 + 1, index6);

                Serial.println("IPv4: " + ip);
                Serial.println("Latitude: " + latitude);
                Serial.println("Longitude: " + longitude);
                Serial.println("Google Maps Link: http://www.google.com/maps/place/" + latitude + "," + longitude);
            } else {
                Serial.println("Không thể phân tích dữ liệu!");
            }
        } else {
            Serial.println("Lỗi HTTP GET! Mã lỗi: " + String(httpCode));
        }
        
        http.end();
    }
}

void loop() {
    // Không cần thực thi liên tục
}
