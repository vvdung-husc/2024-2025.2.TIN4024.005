#include <Arduino.h>
#include <WiFi.h>

// Blynk credentials
#define BLYNK_TEMPLATE_ID "TMPL6rbfID_FR"
#define BLYNK_TEMPLATE_NAME "ESP32GetAPIBlynk"
#define BLYNK_AUTH_TOKEN "9g0w5ZG5r0BUzi4wn79Sb73M-8H6w3gT"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoJson.h> // Add ArduinoJson library for JSON parsing


// WiFi credentials
const char *ssid = "Wokwi-GUEST"; // Replace with your WiFi SSID
const char *password = "";        // Replace with your WiFi password

// OpenWeatherMap API key
const char *weatherApiKey = "fe20094faa908f1009b55dec0054f72e";

// Virtual pins for Blynk
#define VPIN_ACTIVITY_TIME V0  // Virtual pin for Activity Time (Thời gian hoạt động)
#define VPIN_IP_ADDRESS V1     // Virtual pin for IPv4 Address (Địa chỉ IPv4)
#define VPIN_GOOGLE_MAPS V2    // Virtual pin for Google Maps Link
#define VPIN_TEMPERATURE V3    // Virtual pin for Temperature (Nhiệt độ)

// Timer for periodic updates
BlynkTimer timer;
unsigned long startTime; // To track activity time

// Function to split string by delimiter
void splitString(String data, char delimiter, String result[], int size) {
  int start = 0, end = 0, index = 0;
  while ((end = data.indexOf(delimiter, start)) != -1 && index < size) {
    result[index++] = data.substring(start, end);
    start = end + 1;
  }
  if (index < size)
    result[index] = data.substring(start); // Get the last part
}

// Function to fetch data and update Blynk
void fetchAndUpdateData() {
  HTTPClient http;

  // Step 1: Fetch IP and geolocation data
  Serial.println("Fetching IP and geolocation data...");
  http.begin("http://ip4.iothings.vn/?geo=1"); // This API uses HTTP
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("\nDữ liệu nhận được từ ip4.iothings.vn:");
    Serial.println(payload);

    // Split the payload
    const int numFields = 7;
    String fields[numFields];
    splitString(payload, '|', fields, numFields);

    // Create Google Maps link
    String googleMapsLink = "https://www.google.com/maps/place/" + fields[6] + "," + fields[5];

    // Send data to Blynk
    // Activity Time (in seconds)
    unsigned long activityTime = (millis() - startTime) / 1000;
    Blynk.virtualWrite(VPIN_ACTIVITY_TIME, activityTime);

    // IPv4 Address
    Blynk.virtualWrite(VPIN_IP_ADDRESS, fields[0]);

    // Google Maps Link
    Blynk.virtualWrite(VPIN_GOOGLE_MAPS, googleMapsLink);

    // Step 2: Fetch weather data using latitude and longitude
    String lat = fields[6]; // Latitude
    String lon = fields[5]; // Longitude
    String weatherUrl = "/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&appid=" + weatherApiKey + "&units=metric";

    // Use WiFiClientSecure for HTTPS
    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate verification (for simplicity; not recommended for production)

    Serial.println("Fetching weather data from: https://api.openweathermap.org" + weatherUrl);
    if (!client.connect("api.openweathermap.org", 443)) {
      Serial.println("Failed to connect to OpenWeatherMap API");
      return;
    }

    // Send HTTP GET request
    client.print(String("GET ") + weatherUrl + " HTTP/1.1\r\n" +
                 "Host: api.openweathermap.org\r\n" +
                 "Connection: close\r\n\r\n");

    // Read response headers
    String weatherPayload = "";
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break; // Headers end
      }
    }

    // Read response body
    while (client.available()) {
      weatherPayload += client.readString();
    }

    client.stop();

    Serial.println("\nDữ liệu thời tiết nhận được:");
    Serial.println(weatherPayload);

    // Parse JSON response using ArduinoJson
    DynamicJsonDocument doc(1024); // Allocate memory for JSON document
    DeserializationError error = deserializeJson(doc, weatherPayload);

    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Check if the API returned an error
    if (doc.containsKey("cod") && doc["cod"] != 200) {
      Serial.print("Weather API error: ");
      Serial.println(doc["message"].as<String>());
      return;
    }

    // Extract temperature
    float temperature = doc["main"]["temp"];
    Serial.print("Temperature: ");
    Serial.println(temperature);

    // Send temperature to Blynk
    Blynk.virtualWrite(VPIN_TEMPERATURE, temperature);
  } else {
    Serial.printf("Lỗi HTTP (IP API): %d\n", httpResponseCode);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối!");

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  // Record start time for activity time calculation
  startTime = millis();

  // Set up timer to fetch data every 10 seconds
  timer.setInterval(10000L, fetchAndUpdateData);

  // Fetch data immediately on setup
  fetchAndUpdateData();
}

void loop() {
  Blynk.run(); // Run Blynk
  timer.run(); // Run the timer
}