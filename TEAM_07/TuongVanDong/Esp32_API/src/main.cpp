#define BLYNK_TEMPLATE_ID "TMPL6Q37zfPli"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "lSsq7r7KdIUEpQ3KlWomSe5roIBRUY9a"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define OPENWEATHERMAP_KEY "4c8420fc9c3099db403896a2bcbee77e"
// 🔹 Cấu trúc lưu địa chỉ IP và tọa độ GPS
struct IP4_Info {
    String ip4;
    String latitude;
    String longitude;
  };
  IP4_Info ip4Info;
  String urlWeather;
  ulong currentMiliseconds = 0;
  
  bool IsReady(ulong &ulTimer, uint32_t milisecond) {
    if (currentMiliseconds - ulTimer < milisecond) return false;
    ulTimer = currentMiliseconds;
    return true;
  }
  
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
    return String(s);
  }
  
  // 🔹 Phân tích chuỗi trả về từ API IP
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
  
    Serial.printf("📌 IP: %s\n", ip4Info.ip4.c_str());
    Serial.printf("📍 Tọa độ: %s, %s\n", ip4Info.latitude.c_str(), ip4Info.longitude.c_str());
  }
  
  void getAPI() {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("❌ WiFi chưa kết nối trong getAPI()");
      return;
    }
  
    Serial.println("▶ Gửi GET tới ip4.iothings.vn...");
    HTTPClient http;
    http.begin("http://ip4.iothings.vn/?geo=1");
    http.addHeader("Content-Type", "text/plain");
  
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("✔️ Nhận dữ liệu IP:");
      Serial.println(response);
      parseGeoInfo(response, ip4Info);
  
      String urlGoogleMaps = StringFormat("https://www.google.com/maps/place/%s,%s",
                                          ip4Info.latitude.c_str(), ip4Info.longitude.c_str());
      urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
                                ip4Info.latitude.c_str(), ip4Info.longitude.c_str(), OPENWEATHERMAP_KEY);
  
      Serial.println("📡 Google Maps URL:");
      Serial.println(urlGoogleMaps);
      Serial.println("🌦️ Weather URL:");
      Serial.println(urlWeather);
  
      Blynk.virtualWrite(V1, ip4Info.ip4.c_str());
      Blynk.virtualWrite(V2, urlGoogleMaps.c_str());
      Serial.println("📤 Đã gửi IP & GG Map lên Blynk!");
    } else {
      Serial.printf("❌ Lỗi khi gửi GET API: %d\n", httpResponseCode);
    }
  
    http.end();
  }
  
  void updateTemp() {
    static ulong lastTime = 0;
    static float lastTemp = 0.0;
  
    if (!IsReady(lastTime, 10000)) return;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("❌ updateTemp(): WiFi chưa kết nối");
      return;
    }
    if (urlWeather.length() == 0) {
      Serial.println("⚠️ updateTemp(): Chưa có URL weather");
      return;
    }
  
    HTTPClient http;
    http.begin(urlWeather);
    int httpResponseCode = http.GET();
  
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("🌡️ JSON thời tiết:");
      Serial.println(response);
  
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, response);
      if (error) {
        Serial.println("❌ Lỗi parse JSON từ weather API");
        return;
      }
  
      float temp = doc["main"]["temp"];
      if (temp != lastTemp) {
        lastTemp = temp;
        Serial.printf("🌡️ Nhiệt độ mới: %.2f°C\n", temp);
        Blynk.virtualWrite(V3, temp);
        Serial.println("📤 Đã gửi nhiệt độ lên Blynk!");
      }
    } else {
      Serial.printf("❌ HTTP GET nhiệt độ lỗi: %d\n", httpResponseCode);
    }
  
    http.end();
  }
  
  void uptimeBlynk() {
    static ulong lastTime = 0;
    if (!IsReady(lastTime, 1000)) return;
    ulong uptimeSec = lastTime / 1000;
    Blynk.virtualWrite(V0, uptimeSec);
    Serial.printf("⏱️ Uptime: %lu giây\n", uptimeSec);
  }
  
  void setup() {
    Serial.begin(115200);
    Serial.println("🚀 Bắt đầu setup...");
  
    // 🔁 Sử dụng Blynk.begin() để đảm bảo tự kết nối WiFi và Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
  
    while (!Blynk.connected()) {
      Serial.println("⏳ Đang chờ kết nối Blynk...");
      delay(500);
    }
    Serial.println("✅ Đã kết nối Blynk!");
  
    getAPI();  // Gửi IP và Google Maps ban đầu
  }
  
  void loop() {
    Blynk.run();
    currentMiliseconds = millis();
    updateTemp();
    uptimeBlynk();
  }
  