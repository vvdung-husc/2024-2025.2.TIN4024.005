#define BLYNK_TEMPLATE_ID "TMPL6Y4IvecNy"
#define BLYNK_TEMPLATE_NAME "Lê Xuân Chương"
#define BLYNK_AUTH_TOKEN "VAxwl8X2khwTLztv8s4nEHp26sTs4s6A
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include "DHTesp.h"

#define CLK 19
#define DIO 18
#define LED_PIN 21
#define BUTTON_PIN 23
#define DHTPIN 16  

TM1637Display display(CLK, DIO);
DHTesp dht;

const char* ssid = "Wokwi-GUEST";
const char* pass = "";

bool ledState = false;
unsigned long lastMillis = 0;
unsigned long startMillis = 0;

BLYNK_WRITE(V1) {
    ledState = param.asInt();
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

void connectWiFi() {
    WiFi.begin(ssid, pass);
    Serial.print("Connecting to WiFi...");
    int timeout = 20;
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        Serial.print(".");
        delay(500);
        timeout--;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        Blynk.config(BLYNK_AUTH_TOKEN);
    } else {
        Serial.println("Failed to connect to WiFi.");
    }
}

void setup() {
    Serial.begin(115200);
    connectWiFi();
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    display.setBrightness(7);
    dht.setup(DHTPIN, DHTesp::DHT22);
    startMillis = millis();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    } else {
        Blynk.run();
    }

    unsigned long elapsedSeconds = (millis() - startMillis) / 1000;
    display.showNumberDec(elapsedSeconds % 100, true);
    Blynk.virtualWrite(V0, elapsedSeconds);

    if (millis() - lastMillis > 5000) {
        TempAndHumidity data = dht.getTempAndHumidity();
        if (!isnan(data.temperature) && !isnan(data.humidity)) {
            Blynk.virtualWrite(V2, data.temperature);
            Blynk.virtualWrite(V3, data.humidity);
        }
        lastMillis = millis();
    }

    delay(100);
}
