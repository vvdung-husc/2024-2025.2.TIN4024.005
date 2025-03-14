#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

// LÊ BÁ NHẬT MINH
#define BLYNK_TEMPLATE_ID "TMPL6yean1Ms4"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
#define BLYNK_AUTH_TOKEN "LlapeQu1wAa9Xq5m6Mt1BiYAi_EUzieI"

// Hồ Xuân Lãm
// #define BLYNK_TEMPLATE_ID "TMPL60pMYcGkL"
// #define BLYNK_TEMPLATE_NAME "Lab3"
// #define BLYNK_AUTH_TOKEN "wz1Vwsafs3YS58taQXBeaCc8Y-v2LowA"

// Nguyễn Viết HÙNg
// #define BLYNK_TEMPLATE_ID "TMPL6AtsdJeLp"
// #define BLYNK_TEMPLATE_NAME "cambien"
// #define BLYNK_AUTH_TOKEN "zKgdoJrEHwPj7UvdbVTOmn-bJQJyOFkQ"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define rLED 27
#define yLED 26
#define gLED 25
#define btnBLED 23
#define pinBLED 21
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22
#define ldrPIN 32  

uint rTIME = 5000;
uint yTIME = 3000;
uint gTIME = 5000;

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
int currentLED = 0;
int tmCounter = rTIME / 1000;
ulong counterTime = 0;
bool systemEnabled = true;

int darkThreshold = 1000;

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light_TM1637();
bool isDark();
void YellowLED_Blink();
void updateDisplay();

void sendSensor() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Lỗi: Không đọc được dữ liệu từ cảm biến DHT22!");
        return;
    }

    Serial.print("Nhiệt độ: ");
    Serial.print(temperature);
    Serial.print(" °C  |  Độ ẩm: ");
    Serial.print(humidity);
    Serial.println(" %");

    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);
}

void setup() {
    Serial.begin(115200);
    pinMode(rLED, OUTPUT);
    pinMode(yLED, OUTPUT);
    pinMode(gLED, OUTPUT);
    pinMode(pinBLED, OUTPUT);
    pinMode(btnBLED, INPUT_PULLUP);
    pinMode(ldrPIN, INPUT);

    dht.begin();
    tmCounter = rTIME / 1000;
    display.setBrightness(7);

    // Test TM1637
    Serial.println("Kiểm tra TM1637...");
    display.showNumberDec(1234, true, 4, 0);
    delay(2000);
    display.clear();
    delay(1000);

    digitalWrite(yLED, LOW);
    digitalWrite(gLED, LOW);
    digitalWrite(rLED, HIGH);
    display.showNumberDec(tmCounter, true, 2, 2);

    currentLED = rLED;
    nextTimeTotal += rTIME;
    counterTime = millis();
    ledTimeStart = millis();
    Serial.print("Connecting to "); Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    Serial.println("WiFi connected");
    digitalWrite(pinBLED, systemEnabled ? HIGH : LOW);
    Blynk.virtualWrite(V3, systemEnabled);
    Blynk.virtualWrite(V0, tmCounter);
    Blynk.virtualWrite(V4, darkThreshold);
    Serial.println("== START ==>");
    Serial.print("1. RED    => GREEN  "); Serial.print((nextTimeTotal / 1000) % 60); Serial.println(" (ms)");
}

void updateDisplay() {
    int displayValue;
    if (systemEnabled) {
        if (isDark()) {
            displayValue = 0;
        } else {
            displayValue = tmCounter >= 0 ? tmCounter : 0;
        }
    } else {
        displayValue = -1;
    }

    if (displayValue >= 0) {
        Serial.print("Hiển thị thời gian : "); Serial.println(displayValue);
        display.showNumberDec(displayValue, true, 2, 2);
        Blynk.virtualWrite(V0, displayValue);
    } else {
        Serial.println("Xóa bảng TM1637");
        display.clear();
        Blynk.virtualWrite(V0, 0);
    }
}

void updateButton() {
    static ulong lastTime = 0;
    static int lastValue = HIGH;
    if (!IsReady(lastTime, 50)) return;
    int v = digitalRead(btnBLED);
    if (v == lastValue) return;
    lastValue = v;
    if (v == LOW) return;

    systemEnabled = !systemEnabled;
    if (systemEnabled) {
        Serial.println("System ON");
        digitalWrite(pinBLED, HIGH);
        Blynk.virtualWrite(V3, 1);
    } else {
        Serial.println("System OFF");
        digitalWrite(pinBLED, LOW);
        Blynk.virtualWrite(V3, 0);
    }
    updateDisplay();
}

void updateLDRValue() {
    static ulong lastUpdate = 0;
    if (currentMiliseconds - lastUpdate >= 1000) {
        lastUpdate = currentMiliseconds;
        uint16_t lightValue = analogRead(ldrPIN);
        Serial.print("Giá trị LDR: "); Serial.println(lightValue);
        Blynk.virtualWrite(V5, lightValue);
    }
}

void loop() {
    Blynk.run();
    currentMiliseconds = millis();

    if (!systemEnabled) {
        digitalWrite(rLED, LOW);
        digitalWrite(yLED, LOW);
        digitalWrite(gLED, LOW);
        updateDisplay();
        updateLDRValue();
        return;
    }

    updateLDRValue();

    if (isDark()) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, LOW);
        YellowLED_Blink();
    } else {
        NonBlocking_Traffic_Light_TM1637();
    }

    updateButton();

    static ulong lastCounterUpdate = 0;
    if (currentMiliseconds - lastCounterUpdate >= 1000) {
        lastCounterUpdate = currentMiliseconds;
        if (!isDark() && systemEnabled) {
            tmCounter--;
        }
        updateDisplay();
    }

    static ulong lastDHTRead = 0;
    if (currentMiliseconds - lastDHTRead > 2000) {
        lastDHTRead = currentMiliseconds;
        sendSensor();
    }
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
    if (currentMiliseconds - ulTimer < milisecond) return false;
    ulTimer = currentMiliseconds;
    return true;
}

void NonBlocking_Traffic_Light_TM1637() {
    switch (currentLED) {
        case rLED:
            if (IsReady(ledTimeStart, rTIME)) {
                digitalWrite(rLED, LOW);
                digitalWrite(gLED, HIGH);
                currentLED = gLED;
                nextTimeTotal += gTIME;
                tmCounter = gTIME / 1000;
                counterTime = currentMiliseconds;
                Serial.print("2. GREEN  => YELLOW "); Serial.print((nextTimeTotal / 1000) % 60); Serial.println(" (ms)");
                updateDisplay();
            }
            break;
        case gLED:
            if (IsReady(ledTimeStart, gTIME)) {
                digitalWrite(gLED, LOW);
                digitalWrite(yLED, HIGH);
                currentLED = yLED;
                nextTimeTotal += yTIME;
                tmCounter = yTIME / 1000;
                counterTime = currentMiliseconds;
                Serial.print("3. YELLOW => RED    "); Serial.print((nextTimeTotal / 1000) % 60); Serial.println(" (ms)");
                updateDisplay();
            }
            break;
        case yLED:
            if (IsReady(ledTimeStart, yTIME)) {
                digitalWrite(yLED, LOW);
                digitalWrite(rLED, HIGH);
                currentLED = rLED;
                nextTimeTotal += rTIME;
                tmCounter = rTIME / 1000;
                counterTime = currentMiliseconds;
                Serial.print("1. RED    => GREEN  "); Serial.print((nextTimeTotal / 1000) % 60); Serial.println(" (ms)");
                updateDisplay();
            }
            break;
    }
}

bool isDark() {
    static ulong darkTimeStart = 0;
    static uint16_t lastValue = 0;
    static bool bDark = false;

    uint16_t value = analogRead(ldrPIN);
    if (!IsReady(darkTimeStart, 50)) return bDark;
    if (value == lastValue) return bDark;

    if (value < darkThreshold) {
        if (!bDark) {
            Serial.print("Chế độ tối được kích hoạt : "); Serial.println(value);
            digitalWrite(yLED, HIGH);
        }
        bDark = true;
    } else {
        if (bDark) {
            Serial.print("Chế độ sáng được kích hoạt : "); Serial.println(value);
            digitalWrite(yLED, LOW);
        }
        bDark = false;
    }

    lastValue = value;
    return bDark;
}

void YellowLED_Blink() {
    static ulong yLedStart = 0;
    static bool isON = false;

    if (!IsReady(yLedStart, 1000)) return;
    if (!isON) digitalWrite(yLED, HIGH);
    else digitalWrite(yLED, LOW);
    isON = !isON;
}

BLYNK_WRITE(V3) {
    systemEnabled = param.asInt();
    if (systemEnabled) {
        Serial.println("Blynk -> System ON");
        digitalWrite(pinBLED, HIGH);
    } else {
        Serial.println("Blynk -> System OFF");
        digitalWrite(pinBLED, LOW);
    }
    updateDisplay();
}

BLYNK_WRITE(V4) {
    static ulong lastUpdate = 0;
    if (currentMiliseconds - lastUpdate >= 1000) {
        lastUpdate = currentMiliseconds;
        darkThreshold = param.asInt();
        Serial.print("Ngưỡng trời tối cập nhật: ");
        Serial.println(darkThreshold);
    }
}