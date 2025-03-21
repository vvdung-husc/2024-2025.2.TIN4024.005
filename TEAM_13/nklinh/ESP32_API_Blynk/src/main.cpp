#include <Arduino.h>

// Thay thông số BLYNK của bạn vào đây
#define BLYNK_TEMPLATE_ID "TMPL6t8K8IYbg"
#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK API"
#define BLYNK_AUTH_TOKEN "L3ipbkiMUmzWm21-VC_dIjBFHavbBO64"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>   // Thư viện gọi API
#include <ArduinoJson.h>  // Thư viện xử lý JSON

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// Cấu trúc lưu thông tin IPv4, lat, long từ API
struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;  // Biến lưu trữ dữ liệu từ API
ulong currentMiliseconds = 0; // Thời gian hiện tại - miliseconds 

// Kiểm tra khoảng thời gian
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Định dạng chuỗi giống printf
String StringFormat(const char* fmt, ...) {
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  char* buff = (char*)malloc(iLen + 1);
  vsnprintf(buff, iLen + 1, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return s;
}

// Phân tích dữ liệu từ API ip4.iothings.vn
void parseGeoInfo(String payload, IP4_Info& ipInfo) {
  String values[7];
  int index = 0;

  while (payload.length() > 0 && index < 7) {
    int delimiterIndex = payload.indexOf('|');
    if (delimiterIndex == -1) {
      values[index++] = payload;
      break;
    }
    values[index++] = payload.substring(0, delimiterIndex);
    payload = payload.substring(delimiterIndex + 1);
  }

  ipInfo.ip4 = values[0];
  ipInfo.latitude = values[6].c_str();
  ipInfo.longtitude = values[5].c_str();

  Serial.printf("IP Address: %s\r\n", values[0].c_str());
  Serial.printf("Latitude: %s\r\n", values[6].c_str());
  Serial.printf("Longitude: %s\r\n", values[5].c_str());
}

// Key API OpenWeatherMap (thay key của bạn vào đây)
#define OPENWEATHERMAP_KEY "748b77d4134e70731baf0127fe2cbba6"
String urlWeather;

// Gọi API lấy thông tin vị trí
void getAPI() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Lỗi kết nối WiFi");
    return;
  }

  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
    
    parseGeoInfo(response, ip4Info);
    String urlGoogleMaps = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.println(urlGoogleMaps);

    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", 
                              ip4Info.latitude.c_str(), ip4Info.longtitude.c_str(), OPENWEATHERMAP_KEY);
    
    Serial.printf("URL thời tiết: %s\r\n", urlWeather.c_str());
  } else {
    Serial.printf("Lỗi HTTP: %d\r\n", httpResponseCode);
  }
  http.end();
}

// Cập nhật nhiệt độ từ API thời tiết
void updateTemp() {
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 10000)) return;  // Cập nhật mỗi 10 giây
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("updateTemp() Lỗi kết nối WiFi");
    return;
  }

  HTTPClient http;
  http.begin(urlWeather);
  http.addHeader("Content-Type", "text/plain");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.println("Lỗi JSON");
    } else {
      float temp = doc["main"]["temp"];
      if (temp_ != temp) {
        temp_ = temp;
        Serial.printf("Nhiệt độ: %.2f°C\r\n", temp);
        Blynk.virtualWrite(V3, temp_);
      }
    }
  } else {
    Serial.printf("Lỗi HTTP: %d\r\n", httpResponseCode);
  }
  http.end();
}

// Gửi IPv4 & Google Maps lên Blynk (chạy 1 lần)
void onceCalled() {
  static bool done_ = false;
  if (done_) return;
  done_ = true;
  
  String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());
  Blynk.virtualWrite(V2, link.c_str());
}

// Cập nhật uptime lên Blynk
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; // Cập nhật mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
}

// Cấu hình WiFi và Blynk
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Kết nối thành công!");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}

// Vòng lặp chính
void loop() {
  Blynk.run();
  currentMiliseconds = millis();
  onceCalled();
  updateTemp();
  uptimeBlynk();
}
