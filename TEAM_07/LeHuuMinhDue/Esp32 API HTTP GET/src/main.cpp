#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Thông tin WiFi
const char* ssid = "Wokwi-GUEST";  // Đổi theo WiFi của bạn
const char* pass = "";             // Mật khẩu WiFi nếu có

// API lấy thông tin vị trí dựa trên địa chỉ IP
const char* apiURL = "http://ip-api.com/json";

void fetchLocationData() {
  HTTPClient http;
  http.begin(apiURL);

  int httpCode = http.GET();
  if (httpCode <= 0) {  // Kiểm tra lỗi HTTP
      Serial.printf("❌ Lỗi kết nối HTTP: %d\n", httpCode);
      http.end();
      return;
  }

  if (httpCode != HTTP_CODE_OK) {  // Kiểm tra HTTP Code phải là 200
      Serial.printf("❌ Lỗi HTTP %d: Không thể lấy dữ liệu!\n", httpCode);
      http.end();
      return;
  }

  String payload = http.getString();
  Serial.println("\n📥 Dữ liệu JSON nhận được:");
  Serial.println(payload);
  
  // Phân tích dữ liệu JSON
  DynamicJsonDocument doc(2048);  // Tăng kích thước bộ nhớ JSON
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
      Serial.print("❌ Lỗi phân tích JSON: ");
      Serial.println(error.c_str());
      http.end();
      return;
  }

  // Lấy dữ liệu từ JSON (kiểm tra null tránh crash)
  const char* ip = doc["query"] | "Unknown";
  const char* countryCode = doc["countryCode"] | "Unknown";
  const char* country = doc["country"] | "Unknown";
  const char* regionName = doc["regionName"] | "Unknown";
  const char* city = doc["city"] | "Unknown";
  float lon = doc["lon"] | 0.0;
  float lat = doc["lat"] | 0.0;

  // Tạo chuỗi kết quả
  String result = String(ip) + "|" + 
                  String(countryCode) + "|" + 
                  String(country) + "|" + 
                  String(regionName) + "|" + 
                  String(city) + "|" + 
                  String(lon, 6) + "|" + 
                  String(lat, 6);

  Serial.println("\n🔹 Chuỗi kết quả:");
  Serial.println(result);

  // Tạo link Google Maps
  String googleMapsLink = "https://www.google.com/maps?q=" + String(lat) + "," + String(lon);
  Serial.println("\n🔗 Link Google Maps:");
  Serial.println(googleMapsLink);

  http.end();
}


void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, pass);

    Serial.print("Đang kết nối WiFi...");
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20) {  // Giới hạn 10 giây chờ WiFi
        delay(500);
        Serial.print(".");
        attempt++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ Kết nối WiFi thành công!");
        fetchLocationData();  // Gọi hàm lấy dữ liệu từ API
    } else {
        Serial.println("\n❌ Lỗi: Không thể kết nối WiFi!");
    }
}



void loop() {
    // Không làm gì cả
}
