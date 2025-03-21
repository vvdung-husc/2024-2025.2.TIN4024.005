#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";
const char *API_URL = "http://ip4.iothings.vn/?geo=1";

void connectWiFi()
{
  Serial.print("🔌 Connecting to WiFi ");
  WiFi.begin(SSID, PASSWORD);

  int dots = 0;
  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) // Timeout 15s
  {
    Serial.print(".");
    dots++;
    if (dots > 3)
    {
      Serial.print("\r🔌 Connecting to WiFi   "); // Xóa dấu "." cũ
      dots = 0;
    }
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED)
    Serial.println("\r✅ Connected to WiFi     "); // Ghi đè lại với biểu tượng ✅
  else
    Serial.println("\r❌ Failed to connect to WiFi ");
}

bool fetchLocation(String &latitude, String &longitude)
{
  HTTPClient http;
  http.begin(API_URL);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println("🌍 HTTP Response: " + payload);

    int lonStart = payload.lastIndexOf('|', payload.lastIndexOf('|') - 1) + 1;
    int latStart = payload.lastIndexOf('|') + 1;

    if (lonStart > 0 && latStart > lonStart)
    {
      longitude = payload.substring(lonStart, latStart - 1);
      latitude = payload.substring(latStart);
      longitude.trim();
      latitude.trim();
      return true;
    }
  }
  else
  {
    Serial.println("⚠️ HTTP GET failed: " + http.errorToString(httpCode));
  }

  http.end();
  return false;
}

void setup()
{
  Serial.begin(115200);
  connectWiFi();

  if (WiFi.status() == WL_CONNECTED)
  {
    String latitude, longitude;
    if (fetchLocation(latitude, longitude))
    {
      Serial.println("📍 Latitude: " + latitude);
      Serial.println("📍 Longitude: " + longitude);
      Serial.println("🗺️ Google Maps: http://www.google.com/maps/place/" + latitude + "," + longitude);
    }
    else
    {
      Serial.println("❌ Failed to parse location data.");
    }
  }
}

void loop()
{
}
