#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6ouiNidYY"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "VyRw5Mn8oG7VOjeGLwJFoKD9BeqxSm_7"

#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Thêm thư viện để phân tích JSON
#include <BlynkSimpleEsp32.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";
const char *geoApiUrl = "http://ip4.iothings.vn/?geo=1";
ulong currentMiliseconds = 0;
const String appId = "e71a8a5e03d91a235e846c8cf99b7724";
String latitude = "";
String longitude = "";
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void uptimeBlynk();
void getWeather();
void getLocation();
void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");

  // Lấy vị trí từ API
  getLocation();
}

void loop()
{
  Blynk.run(); // Chạy Blynk để cập nhật trạng thái từ Blynk Cloud
  currentMiliseconds = millis();
  ulong weatherMiliseconds = 0;
  uptimeBlynk();
  // Nếu lấy được vị trí, tiếp tục gọi API thời tiết
  if (latitude != "" && longitude != "")
  {
    if (!IsReady(weatherMiliseconds, 120000))
      return;
    getWeather();
  }
}

void getLocation()
{
  HTTPClient http;
  http.begin(geoApiUrl);

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println("HTTP Response: " + payload);

    int ipEnd = payload.indexOf('|');
    String ipAddress = payload.substring(0, ipEnd);
    ipAddress.trim();
    Blynk.virtualWrite(V1, ipAddress);

    int lonStart = payload.lastIndexOf('|', payload.lastIndexOf('|') - 1) + 1;
    int latStart = payload.lastIndexOf('|') + 1;

    longitude = payload.substring(lonStart, latStart - 1);
    latitude = payload.substring(latStart);

    longitude.trim();
    latitude.trim();

    Serial.println("IP Address: " + ipAddress);
    Serial.println("Latitude: " + latitude);
    Serial.println("Longitude: " + longitude);

    String googleMapsLink = "http://www.google.com/maps/place/" + latitude + "," + longitude;
    Blynk.virtualWrite(V2, googleMapsLink);
    Serial.println("Google Maps Link: " + googleMapsLink);
  }
  else
  {
    Serial.print("HTTP GET request failed, error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();
}

void getWeather()
{
  HTTPClient http;
  String weatherApiUrl = "https://api.openweathermap.org/data/2.5/weather?lat=" + latitude + "&lon=" + longitude + "&appid=" + appId + "&units=metric";

  Serial.println("Fetching weather data...");
  http.begin(weatherApiUrl);

  int httpCode = http.GET();
  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println("Weather API Response: " + payload);

    // Phân tích JSON
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error)
    {
      float temp = doc["main"]["temp"];
      String description = doc["weather"][0]["description"].as<String>();

      Serial.println("Temperature: " + String(temp) + "°C");

      // Gửi dữ liệu lên Blynk
      Blynk.virtualWrite(V3, temp);
    }
    else
    {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
    }
  }
  else
  {
    Serial.print("Weather API request failed, error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();
}

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return; // Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
}
