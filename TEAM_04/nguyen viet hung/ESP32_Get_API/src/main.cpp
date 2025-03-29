#include <WiFi.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>
#include <time.h>

// Thông tin WiFi của bạn
const char* ssid     = "Wokwi-GUEST";
const char* password = "";

// Thông tin Blynk
#define BLYNK_TEMPLATE_ID "TMPL6uxYWXU9M"
#define BLYNK_TEMPLATE_NAME "API"
#define BLYNK_AUTH_TOKEN "4ZZqOSESENAPTkgKxZI61r2nWUyhmfoS"

// Cấu hình NTP để lấy thời gian
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;          // Điều chỉnh theo múi giờ (vd: +7*3600 cho GMT+7)
const int   daylightOffset_sec = 0;      // Điều chỉnh nếu có giờ tiết kiệm

// Khai báo BlynkTimer để cập nhật định kỳ
BlynkTimer timer;

// Hàm tách chuỗi theo delimiter
void splitString(String data, char delimiter, String result[], int size) {
  int start = 0, end = 0, index = 0;
  while ((end = data.indexOf(delimiter, start)) != -1 && index < size) {
    result[index++] = data.substring(start, end);
    start = end + 1;
  }
  if (index < size) {
    result[index] = data.substring(start);  // Lấy phần cuối cùng
  }
}

// Hàm lấy thời gian hiện tại (định dạng: dd/mm/yyyy HH:MM:SS)
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time Not Available";
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

// Hàm đọc nhiệt độ (giá trị mẫu, thay thế bằng cảm biến nếu có)
float readTemperature() {
  // Ở đây trả về giá trị mẫu 25.0 độ C
  return 25.0;
}

// Hàm cập nhật dữ liệu từ API và gửi lên Blynk
void updateData() {
  Serial.println("=== Bắt đầu cập nhật dữ liệu ===");

  // Lấy thời gian hiện tại
  String currentTime = getFormattedTime();

  // Gọi API lấy thông tin IP và địa lý
  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");  // API
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("Dữ liệu từ API:");
    Serial.println(payload);
    
    // Tách chuỗi theo ký tự "|"
    const int numFields = 7;  // Số lượng trường dữ liệu dự kiến từ API
    String fields[numFields];
    splitString(payload, '|', fields, numFields);
    
    // Tạo link Google Maps: sử dụng trường vĩ độ (fields[6]) và kinh độ (fields[5])
    String googleMapsLink = "https://www.google.com/maps/search/?api=1&query=" 
                            + fields[6] + "," + fields[5];
    
    // Lấy nhiệt độ (giá trị mẫu hoặc đọc từ cảm biến thực)
    float temperature = readTemperature();
    
    // Debug log
    Serial.println("----------------------------------------------");
    Serial.print("Thời gian   : "); Serial.println(currentTime);
    Serial.print("IP         : "); Serial.println(fields[0]);
    Serial.print("Google Map : "); Serial.println(googleMapsLink);
    Serial.print("Nhiệt độ   : "); Serial.println(temperature);
    Serial.println("----------------------------------------------");
    
    // Cập nhật thông tin lên Blynk:
    // V0: Thời gian, V1: Địa chỉ IP, V2: Link Google Map, V3: Nhiệt độ
    Blynk.virtualWrite(V0, currentTime);
    Blynk.virtualWrite(V1, fields[0]);
    Blynk.virtualWrite(V2, googleMapsLink);
    Blynk.virtualWrite(V3, temperature);
  } else {
    Serial.printf("Lỗi HTTP: %d\n", httpResponseCode);
  }
  
  http.end();
}

void setup() {
  Serial.begin(115200);

  // Kết nối WiFi
  Serial.print("Đang kết nối WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối!");

  // Khởi tạo NTP để lấy thời gian
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Kết nối với Blynk
  Serial.println("Kết nối với Blynk Cloud...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  // Đợi vài giây để đảm bảo Blynk đã kết nối đầy đủ
  delay(2000);

  // Cập nhật dữ liệu ngay sau khi kết nối thành công
  updateData();
  
  // Thiết lập BlynkTimer cập nhật dữ liệu mỗi 30 giây
  timer.setInterval(30000L, updateData);
}

void loop() {
  Blynk.run();
  timer.run();
}
