#include <WiFi.h>
#include <HTTPClient.h>

char wifiSSID[] = "Wokwi-GUEST";
char wifiPassword[] = "";

String gpsLatitude = "";
String gpsLongitude = "";

void parseLocationData(String rawData) {
    String locationParts[7];  
    int partIndex = 0, startIndex = 0, endIndex = 0;

    while ((endIndex = rawData.indexOf('|', startIndex)) != -1 && partIndex < 6) {
        locationParts[partIndex++] = rawData.substring(startIndex, endIndex);
        startIndex = endIndex + 1;
    }
    locationParts[partIndex] = rawData.substring(startIndex);

    Serial.println("\n=== Location Data from API ===");
    Serial.println("IP Address: " + locationParts[0]);
    Serial.println("Country Code: " + locationParts[1]);
    Serial.println("Country Name: " + locationParts[2]);
    Serial.println("Province/City: " + locationParts[3]);
    Serial.println("District: " + locationParts[4]);
    Serial.println("Longitude: " + locationParts[5]);
    Serial.println("Latitude: " + locationParts[6]);

    gpsLongitude = locationParts[5];
    gpsLatitude = locationParts[6];
}

void setup() {
    Serial.begin(115200);
    Serial.println("🔄 Connecting to WiFi...");

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8)); // Google DNS
    WiFi.begin(wifiSSID, wifiPassword);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi Connected!");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ No WiFi connection. Retrying...");
        WiFi.disconnect();
        WiFi.reconnect();
        delay(5000);
        return;
    }

    HTTPClient http;
    String apiURL = "http://ip4.iothings.vn/?geo=1";

    Serial.println("\n🌍 Sending HTTP request...");
    http.begin(apiURL);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String responsePayload = http.getString();  
        Serial.println("✅ Received Data:");
        Serial.println(responsePayload);
        parseLocationData(responsePayload);

        if (gpsLatitude != "" && gpsLongitude != "") {
            String googleMapsURL = "http://www.google.com/maps/place/" + gpsLatitude + "," + gpsLongitude;
            Serial.println("\n📍 Google Maps Link:");
            Serial.println(googleMapsURL);
        }
    } else {
        Serial.println("❌ HTTP Error, Code: " + String(httpResponseCode));
    }

    http.end();  
    delay(60000);
}
