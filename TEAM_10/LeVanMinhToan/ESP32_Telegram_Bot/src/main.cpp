#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* wifiSSID = "Wokwi-GUEST";
const char* wifiPassword = "";

#define TELEGRAM_BOT_TOKEN "7441033779:AAEOc1s6Gc_TVWQDAtlih7Zdoei2oiXSCD8"
#define TELEGRAM_GROUP_ID "-4621614489"
WiFiClientSecure secureClient;
UniversalTelegramBot telegramBot(TELEGRAM_BOT_TOKEN, secureClient);

const int pirSensorPin = 27;
bool isMotionDetected = false;

String formatString(const char* format, ...) {
    va_list args;
    va_start(args, format);
    va_list argsCopy;
    va_copy(argsCopy, args);
    int length = vsnprintf(NULL, 0, format, argsCopy);
    va_end(argsCopy);
    int bufferSize = length + 1;
    char* buffer = (char*)malloc(bufferSize);
    vsnprintf(buffer, bufferSize, format, args);
    va_end(args);
    String formattedString = buffer;
    free(buffer);
    return formattedString;
}

void IRAM_ATTR motionInterruptHandler() {
    isMotionDetected = true;
}

void setup() {
    Serial.begin(115200);

    pinMode(pirSensorPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pirSensorPin), motionInterruptHandler, RISING);

    Serial.print("Connecting to WiFi: ");
    Serial.println(wifiSSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID, wifiPassword);
    secureClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nWiFi connected");
    telegramBot.sendMessage(TELEGRAM_GROUP_ID, "IoT Motion Detection System Started");
}

void loop() {
    static uint eventCounter = 0;

    if (isMotionDetected) {
        ++eventCounter;
        Serial.printf("%u. Motion detected! Sending alert...\n", eventCounter);
        String message = formatString("%u => Motion detected!", eventCounter);
        telegramBot.sendMessage(TELEGRAM_GROUP_ID, message.c_str());
        Serial.printf("%u. Alert sent successfully!\n", eventCounter);
        isMotionDetected = false;
    }
}
