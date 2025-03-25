#include <Arduino.h>

// LE VAN MINH TOAN
#define BLYNK_TEMPLATE_ID "TMPL6eDvyBqz2"
#define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
#define BLYNK_AUTH_TOKEN "RP-9PNRNu-xTRTCNClk-LGQNvj6r77_a"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>   //Thư viện gọi API
#include <ArduinoJson.h>  //Thư viện xử lý JSON

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

struct GeoLocation {
  String ipAddress;
  String latitude;
  String longitude;
};

GeoLocation geoData;
ulong currentMillis = 0;

bool isElapsed(ulong &lastMillis, uint32_t interval) {
  if (currentMillis - lastMillis < interval) return false;
  lastMillis = currentMillis;
  return true;
}

String formatString(const char* fmt, ...){
  va_list args;
  va_start(args, fmt);
  va_list argsCopy;
  va_copy(argsCopy, args);
  int len = vsnprintf(NULL, 0, fmt, argsCopy);
  va_end(argsCopy);
  char* buffer = (char*)malloc(len + 1);
  vsnprintf(buffer, len + 1, fmt, args);
  va_end(args);
  String result = buffer;
  free(buffer);
  return result;
}

void parseGeoData(String payload, GeoLocation& geo) {
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
  geo.ipAddress = values[0];
  geo.latitude = values[6].c_str();
  geo.longitude = values[5].c_str();
  Serial.printf("IP Address: %s\r\n", values[0].c_str());
  Serial.printf("Longitude: %s\r\n", values[5].c_str());
  Serial.printf("Latitude: %s\r\n", values[6].c_str());
}

#define WEATHER_API_KEY "85551d6842f85f3149f73c5004bff347" 
String weatherApiUrl; 

void fetchGeoData(){
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("fetchGeoData() Error: WiFi not connected"); return;
  }
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");
  int responseCode = http.GET();
  if(responseCode > 0){
    String response = http.getString();
    Serial.println(response);
    parseGeoData(response, geoData);
    String googleMapsUrl = formatString("https://www.google.com/maps/place/%s,%s", geoData.latitude.c_str(), geoData.longitude.c_str());
    Serial.println(googleMapsUrl);
    weatherApiUrl = formatString("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", 
      geoData.latitude.c_str(), geoData.longitude.c_str(), WEATHER_API_KEY);
    Serial.printf("Weather API URL: %s\r\n", weatherApiUrl.c_str());      
  } else {
    Serial.print("Error in GET request: ");
    Serial.println(responseCode);
  }
  http.end();
}

void updateTemperature(){
  static ulong lastUpdate = 0;
  static float lastTemp = 0.0;
  if (!isElapsed(lastUpdate, 10000)) return;
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("updateTemperature() Error: WiFi not connected"); 
    return;
  }
  HTTPClient http;   
  http.begin(weatherApiUrl);
  http.addHeader("Content-Type", "text/plain");
  int responseCode = http.GET();
  if(responseCode > 0){
    String response = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.f_str());  
      return;
    }
    float temp = doc["main"]["temp"].as<float>();
    Serial.print("Temperature received: ");
    Serial.println(temp);
    if (lastTemp != temp) {  
      lastTemp = temp;
      Blynk.virtualWrite(V3, lastTemp);
    }
  } else {
    Serial.print("Error in GET request: ");
    Serial.println(responseCode);
  }
  http.end();
}

void sendInitialData(){
  static bool sent = false;
  if (sent) return;
  sent = true;
  String mapLink = formatString("https://www.google.com/maps/place/%s,%s", geoData.latitude.c_str(), geoData.longitude.c_str());
  Blynk.virtualWrite(V1, geoData.ipAddress.c_str());
  Blynk.virtualWrite(V2, mapLink.c_str());
}

void updateUptime(){
  static ulong lastUpdate = 0;
  if (!isElapsed(lastUpdate, 1000)) return;
  ulong uptimeSeconds = lastUpdate / 1000;
  Blynk.virtualWrite(V0, uptimeSeconds);
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  fetchGeoData();
}

void loop(void) {  
  Blynk.run();  
  currentMillis = millis();
  sendInitialData(); 
  updateTemperature();
  updateUptime();
}
