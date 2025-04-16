#include <Arduino.h>

// Thông số Blynk
#define BLYNK_TEMPLATE_ID "TMPL6Dvq4dgDP"
#define BLYNK_TEMPLATE_NAME "Esp32ApiHttpGet"
#define BLYNK_AUTH_TOKEN "-6OcGWSW2YWg23DHSRfZyY5F4CycSdqE"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6
#define OPENWEATHERMAP_KEY "3859f9f6c28cece5859f7e6e5f91535d"

// Cấu trúc lưu thông tin IP và tọa độ
struct IP4_Info {
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;
String urlWeather;
ulong currentMiliseconds = 0;

// Kiểm tra thời gian
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Định dạng chuỗi
String StringFormat(const char* fmt, ...) {
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return s;
}

// Phân tích thông tin địa lý
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
  ipInfo.latitude = values[6];
  ipInfo.longitude = values[5];
}

// Lấy thông tin IP và tọa độ
void getAPI() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error in WiFi connection");
    return;
  }
  
  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    parseGeoInfo(response, ip4Info);
    
    // Tạo URL Google Maps
    String urlGoogleMaps = StringFormat("http://www.google.com/maps/place/%s,%s", 
                                      ip4Info.latitude.c_str(), 
                                      ip4Info.longitude.c_str());
    
    // Tạo URL OpenWeatherMap
    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s",
                            ip4Info.latitude.c_str(), 
                            ip4Info.longitude.c_str(), 
                            OPENWEATHERMAP_KEY);
  }
  http.end();
}

// Cập nhật nhiệt độ
void updateTemp() {
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 5000)) return; // Cập nhật mỗi 55 giây
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("updateTemp() Error in WiFi connection");
    return;
  }

  HTTPClient http;
  http.begin(urlWeather);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error) {
      float temp = doc["main"]["temp"];
      if (temp_ != temp) {
        temp_ = temp - 273.15; // Chuyển từ Kelvin sang Celsius
        Serial.printf("Nhiet do: %.2f°C\n", temp_);
        Blynk.virtualWrite(V3, temp_); // Gửi nhiệt độ lên V3
      }
    }
  }
  http.end();
}

// Gửi thông tin IP và Google Maps một lần
void onceCalled() {
  static bool done_ = false;
  if (done_) return;
  done_ = true;
  
  String link = StringFormat("http://www.google.com/maps/place/%s,%s",
                           ip4Info.latitude.c_str(), 
                           ip4Info.longitude.c_str());
  
  Blynk.virtualWrite(V1, ip4Info.ip4);    // Gửi IPv4 lên V1
  Blynk.virtualWrite(V2, link);          // Gửi link Google Maps lên V2
}

// Cập nhật thời gian hoạt động
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; // Cập nhật mỗi 1 giây
  
  ulong uptime = currentMiliseconds / 1000; // Giây
  Blynk.virtualWrite(V0, uptime); // Gửi thời gian hoạt động lên V0
}

void setup() {
  Serial.begin(115200);
  
  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  // Khởi tạo Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  
  // Lấy thông tin IP và tọa độ
  getAPI();
}

void loop() {
  Blynk.run();
  currentMiliseconds = millis();
  
  onceCalled();    // Cập nhật V1, V2 một lần
  updateTemp();    // Cập nhật V3 (nhiệt độ)
  uptimeBlynk();   // Cập nhật V0 (thời gian hoạt động)
}