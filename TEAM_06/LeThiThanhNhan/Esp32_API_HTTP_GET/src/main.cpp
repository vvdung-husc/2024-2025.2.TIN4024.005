#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";
const char *apiUrl = "http://ip4.iothings.vn/?geo=1";

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");

  HTTPClient http;
  http.begin(apiUrl);

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println("HTTP Response: " + payload);

    // Phân tích chuỗi phản hồi
    int lonStart = payload.lastIndexOf('|', payload.lastIndexOf('|') - 1) + 1;
    int latStart = payload.lastIndexOf('|') + 1;

    String longitude = payload.substring(lonStart, latStart - 1);
    String latitude = payload.substring(latStart);

    longitude.trim();
    latitude.trim();

    Serial.println("Latitude: " + latitude);
    Serial.println("Longitude: " + longitude);

    String googleMapsLink = "http://www.google.com/maps/place/" + latitude + "," + longitude;
    Serial.println("Google Maps Link: " + googleMapsLink);
  }
  else
  {
    Serial.print("HTTP GET request failed, error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();
}

void loop()
{
  // Do nothing in loop
}
