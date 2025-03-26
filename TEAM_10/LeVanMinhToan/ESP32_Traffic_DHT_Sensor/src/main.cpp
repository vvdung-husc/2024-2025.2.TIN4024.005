#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* wifiNetwork = "Wokwi-GUEST";
const char* wifiPass = "";

#define TELEGRAM_BOT_KEY "7441033779:AAEOc1s6Gc_TVWQDAtlih7Zdoei2oiXSCD8"
#define TELEGRAM_CHAT_ID "-4621614489"
WiFiClientSecure secureWifiClient;
UniversalTelegramBot telegramNotifier(TELEGRAM_BOT_KEY, secureWifiClient);

const int motionSensorPin = 27;
bool motionDetected = false;

String createFormattedMessage(const char* format, ...) {
    va_list args;
    va_start(args, format);
    va_list argsCopy;
    va_copy(argsCopy, args);
    int messageLength = vsnprintf(NULL, 0, format, argsCopy);
    va_end(argsCopy);
    int bufferSize = messageLength + 1;
    char* buffer = (char*)malloc(bufferSize);
    vsnprintf(buffer, bufferSize, format, args);
    va_end(args);
    String formattedMessage = buffer;
    free(buffer);
    return formattedMessage;
}

void IRAM_ATTR handleMotionInterrupt() {
    motionDetected = true;
}

void setup() {
    Serial.begin(115200);

    pinMode(motionSensorPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(motionSensorPin), handleMotionInterrupt, RISING);

    Serial.print("Connecting to WiFi: ");
    Serial.println(wifiNetwork);

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiNetwork, wifiPass);
    secureWifiClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nWiFi connected");
    telegramNotifier.sendMessage(TELEGRAM_CHAT_ID, "IoT Motion Detection System Started");
}

void loop() {
    static uint motionEventCounter = 0;

    if (motionDetected) {
        ++motionEventCounter;
        Serial.printf("%u. Motion detected! Sending alert...\n", motionEventCounter);
        String alertMessage = createFormattedMessage("%u => Motion detected!", motionEventCounter);
        telegramNotifier.sendMessage(TELEGRAM_CHAT_ID, alertMessage.c_str());
        Serial.printf("%u. Alert sent successfully!\n", motionEventCounter);
        motionDetected = false;
    }
}