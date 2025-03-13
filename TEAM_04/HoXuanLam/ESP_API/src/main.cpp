#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Wokwi-GUEST";  // Thay bằng WiFi của bạn
const char* password = "";         // Nếu có mật khẩu, nhập vào đây

void splitString(String data, char delimiter, String result[], int size) {
    int start = 0, end = 0, index = 0;
    while ((end = data.indexOf(delimiter, start)) != -1 && index < size) {
        result[index++] = data.substring(start, end);
        start = end + 1;
    }
    if (index < size) result[index] = data.substring(start);  // Lấy phần cuối cùng
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    Serial.print("Đang kết nối WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi đã kết nối!");

    HTTPClient http;
    http.begin("http://ip4.iothings.vn/?geo=1");  // API lấy thông tin IP và địa lý
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("\nDữ liệu nhận được:");
        Serial.println(payload);

        // Tách chuỗi theo ký tự "|"
        const int numFields = 7;  // Số lượng trường dữ liệu
        String fields[numFields];

        splitString(payload, '|', fields, numFields);

        // Tạo link Google Maps
        String googleMapsLink = "https://www.google.com/maps/search/?api=1&query=" + fields[6] + "," + fields[5];

        // In thông tin ra Serial với căn chỉnh cột
        Serial.println("\n----------------------------------------------");
        Serial.print("IP             : "); Serial.println(fields[0]); Serial.flush();
        Serial.print("Mã quốc gia    : "); Serial.println(fields[1]); Serial.flush();
        Serial.print("Quốc gia       : "); Serial.println(fields[2]); Serial.flush();
        Serial.print("Vùng           : "); Serial.println(fields[3]); Serial.flush();
        Serial.print("Thành phố      : "); Serial.println(fields[4]); Serial.flush();
        Serial.print("Kinh độ        : "); Serial.println(fields[5]); Serial.flush();
        Serial.print("Vĩ độ          : "); Serial.println(fields[6]); Serial.flush();
        Serial.println("----------------------------------------------");

        // In Google Maps Link
        Serial.print("Google Maps: "); Serial.println(googleMapsLink);
        Serial.println("----------------------------------------------");

    } else {
        Serial.printf("Lỗi HTTP: %d\n", httpResponseCode);
    }

    http.end();
}

void loop() {
    // Chương trình không cần vòng lặp liên tục
}