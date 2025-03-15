#include <Arduino.h>
#include <TM1637Display.h>

// Lại Văn Minh
// #define BLYNK_TEMPLATE_ID "TMPL6zydB78I4"
// #define BLYNK_TEMPLATE_NAME "TRAFFICBLYNK"
// #define BLYNK_AUTH_TOKEN "ddYhBGFVvAZVMMBgh73zS1nQx9TVrcl_"

// Nguyễn Thị Diệu Anh
<<<<<<< HEAD
//   #define BLYNK_TEMPLATE_ID "TMPL6yjnfI3kl"
//   #define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
//   #define BLYNK_AUTH_TOKEN "pti792X8DQI82xrWQebH3-hNlIEb24ny"

//Phạm Ngọc Cảm
#define BLYNK_TEMPLATE_ID "TMPL6l90z6d8N"
#define BLYNK_TEMPLATE_NAME "ESP32BlynkTrafficNhom"
#define BLYNK_AUTH_TOKEN "v2WiKiGIRkz2GMj1mqsF8ocsLFHcTq4x"
=======
#define BLYNK_TEMPLATE_ID "TMPL6yjnfI3kl"
#define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
#define BLYNK_AUTH_TOKEN "pti792X8DQI82xrWQebH3-hNlIEb24ny"
>>>>>>> 10eb65d5ea7578fb6dcbac7b9180525d962bbdf0

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Pin Definitions
#define BUTTON_PIN 23
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define LED_PINK 21
#define LDR_PIN 34
#define CLK 18
#define DIO 19
#define DHT_PIN 16 // DHT22 sensor pin
#define DHT_TYPE DHT22

// Timing for Traffic Lights
#define RED_TIME 5000
#define GREEN_TIME 7000
#define YELLOW_TIME 3000

TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

enum TrafficLightState
{
    GREEN,
    YELLOW,
    RED
};
TrafficLightState lightState = GREEN;
int countDown = GREEN_TIME / 1000;
unsigned long currentMillis = 0;
bool buttonState = true;

void BlinkingYellowLight();
void NormalTrafficLight();
void ClearTM();
bool IsRead(unsigned long &previousMillis, unsigned long milliseconds);
void updateTemperatureHumidity();
void uptimeBlynk();

void setup()

{
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_PINK, OUTPUT);
    pinMode(LDR_PIN, INPUT);

    display.setBrightness(7);
    dht.begin();

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("❌ WiFi connection failed!");
    }
    else
    {
        Serial.println("✅ WiFi connected!");
    }

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
    display.showNumberDec(countDown);

    Blynk.virtualWrite(V0, countDown);
    Blynk.virtualWrite(V1, buttonState);
}

int darkThreshold = 1000;
void loop()
{
    Blynk.run();
    currentMillis = millis();
    float ldr = analogRead(LDR_PIN);
    static unsigned long previousButtonMillis = 0;

    Serial.print("LDR Raw Value: ");
    Serial.println(ldr);

    // Check button state
    if (digitalRead(BUTTON_PIN) == LOW && IsRead(previousButtonMillis, 50))
    {
        buttonState = !buttonState;
        Serial.print("Button State: ");
        Serial.println(buttonState);
    }

    if (ldr < darkThreshold)
    {
        BlinkingYellowLight();
    }
    else
    {
        if (buttonState)
        {
            digitalWrite(LED_PINK, buttonState);
            NormalTrafficLight();
            Blynk.virtualWrite(V1, buttonState);
        }
        else
        {
            digitalWrite(LED_PINK, buttonState);
            ClearTM();
            Blynk.virtualWrite(V1, buttonState);
        }
    }

    updateTemperatureHumidity();
    uptimeBlynk();
    Blynk.virtualWrite(V4, darkThreshold);
}

bool IsRead(unsigned long &previousMillis, unsigned long milliseconds)
{
    if (currentMillis - previousMillis < milliseconds)
        return false;
    previousMillis = currentMillis;
    return true;
}

void NormalTrafficLight()
{
    static unsigned long previousMillis = 0;
    if (IsRead(previousMillis, 1000))
    {
        countDown--;

        if (countDown < 0)
        {
            switch (lightState)
            {
            case GREEN:
                lightState = YELLOW;
                countDown = YELLOW_TIME / 1000;
                break;
            case YELLOW:
                lightState = RED;
                countDown = RED_TIME / 1000;
                break;
            case RED:
                lightState = GREEN;
                countDown = GREEN_TIME / 1000;
                break;
            }
        }

        display.showNumberDec(countDown);
    }

    switch (lightState)
    {
    case GREEN:
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, LOW);
        break;
    case RED:
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        break;
    case YELLOW:
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        break;
    }
}

void ClearTM()
{
    static unsigned long previousMillis = 0;
    if (IsRead(previousMillis, 1000))
    {
        countDown--;
        if (countDown < 0)
        {
            switch (lightState)
            {
            case GREEN:
                lightState = YELLOW;
                countDown = YELLOW_TIME / 1000;
                break;
            case YELLOW:
                lightState = RED;
                countDown = RED_TIME / 1000;
                break;
            case RED:
                lightState = GREEN;
                countDown = GREEN_TIME / 1000;
                break;
            }
        }
        display.clear();
    }

    switch (lightState)
    {
    case GREEN:
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, LOW);
        break;
    case RED:
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        break;
    case YELLOW:
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        break;
    }
}

void BlinkingYellowLight()
{
    static bool yellowState = false;
    static unsigned long previousBlinkMillis = 0;
    if (IsRead(previousBlinkMillis, 500))
    {
        yellowState = !yellowState;
        digitalWrite(LED_YELLOW, yellowState);
    }

    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_PINK, LOW);
    display.clear();
}

void updateTemperatureHumidity()
{
    static ulong lastUpdate = 0;
    if (!IsRead(lastUpdate, 2000))
        return;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum))
    {
        Blynk.virtualWrite(V2, temp);
        Blynk.virtualWrite(V3, hum);

        Serial.print("🌡 Temperature: ");
        Serial.print(temp);
        Serial.print(" °C | 💧 Humidity: ");
        Serial.print(hum);
        Serial.println(" %");
    }
    else
    {
        Serial.println("⚠️ Error reading DHT22 sensor!");
    }
}

void uptimeBlynk()
{
    static ulong lastTime = 0;
    if (!IsRead(lastTime, 1000))
        return; // Kiểm tra và cập nhật lastTime sau mỗi 1 giây
    ulong value = lastTime / 1000;
    Blynk.virtualWrite(V0, value);
}

BLYNK_WRITE(V1)
{
    buttonState = param.asInt();
    digitalWrite(LED_PINK, buttonState ? HIGH : LOW);
    if (!buttonState)
        display.clear();
}

BLYNK_WRITE(V4)
{
    darkThreshold = param.asInt(); // Cập nhật giá trị ngưỡng từ Blynk
    Serial.print("Updated LDR Threshold: ");
    Serial.println(darkThreshold);
}
