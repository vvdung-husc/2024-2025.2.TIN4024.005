#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

// Định nghĩa các chân
#define RED_PIN 27      // Đèn đỏ
#define YELLOW_PIN 26   // Đèn vàng
#define GREEN_PIN 25    // Đèn xanh lá
#define BLUE_PIN 21     // Đèn xanh dương
#define BUTTON_PIN 23   // Nút nhấn
#define LDR_PIN 32      // Cảm biến ánh sáng
#define CLK_PIN 18      // TM1637 CLK
#define DIO_PIN 19      // TM1637 DIO
#define DHT_PIN 16      // DHT22


// Hồ Xuân Lãm
#define BLYNK_TEMPLATE_ID "TMPL60pMYcGkL"
#define BLYNK_TEMPLATE_NAME "Lab3"
#define BLYNK_AUTH_TOKEN "wz1Vwsafs3YS58taQXBeaCc8Y-v2LowA"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Cấu hình DHT
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// Các hằng số
#define GAMMA 0.7
#define RL10 50
#define DEBOUNCE_TIME 200
#define NIGHT_LUX_THRESHOLD 50

// Biến toàn cục
bool isSystemPaused = false;    
unsigned long lastButtonPress = 0;
TM1637Display display(CLK_PIN, DIO_PIN);
unsigned long previousMillis = 0;
unsigned long dhtPreviousMillis = 0;
unsigned long uptimePreviousMillis = 0; // Thời gian cập nhật Uptime
unsigned long startMillis = 0;
long interval = 1000;
long dhtInterval = 2000;
long uptimeInterval = 1000; // Cập nhật uptime mỗi 1 giây
int countdown = 0;
int displayCountdown = 0;
int trafficState = 0;
bool nightMode = false;
float temperature = 0;
float humidity = 0;
float currentLux = 0;
bool systemEnabled = true;
int nightLuxLimit = NIGHT_LUX_THRESHOLD;

void setup() {
    pinMode(RED_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LDR_PIN, INPUT);

    Serial.begin(9600);
    display.setBrightness(7);
    dht.begin();

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    startMillis = millis();
    Blynk.virtualWrite(V3, 1); // Khởi tạo hệ thống bật
    Blynk.virtualWrite(V0, 0); // Khởi tạo thời gian hoạt động ban đầu

    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    Serial.println("Đèn xanh (5 giây)");
    countdown = 5;
    displayCountdown = countdown;
    trafficState = 0;
}

void changeLight() {
    if (nightMode || !systemEnabled) return;

    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);

    if (trafficState == 0) { // Đèn vàng
        digitalWrite(YELLOW_PIN, HIGH);
        Serial.println("Đèn vàng (2 giây)");
        countdown = 2;
        trafficState = 1;
    } else if (trafficState == 1) { // Đèn đỏ
        digitalWrite(RED_PIN, HIGH);
        Serial.println("Đèn đỏ (5 giây)");
        countdown = 5;
        trafficState = 2;
    } else { // Đèn xanh
        digitalWrite(GREEN_PIN, HIGH);
        Serial.println("Đèn xanh (5 giây)");
        countdown = 5;
        trafficState = 0;
    }
    displayCountdown = countdown;
}

void checkButtonPress() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastButtonPress > DEBOUNCE_TIME) {
            isSystemPaused = !isSystemPaused;
            lastButtonPress = currentMillis;

            if (!isSystemPaused) {
                digitalWrite(BLUE_PIN, HIGH);
                display.showNumberDec(displayCountdown, true);
                Blynk.virtualWrite(V3, 1);
                Serial.println("Hệ thống BẬT - V3 = 1");
            } else {
                digitalWrite(BLUE_PIN, LOW);
                display.clear();
                Blynk.virtualWrite(V3, 0);
                Serial.println("Hệ thống TẮT - V3 = 0");
            }
        }
    }
}

BLYNK_WRITE(V3) {
    int pinValue = param.asInt();
    systemEnabled = pinValue;

    if (pinValue == 1) {
        isSystemPaused = false;
        digitalWrite(BLUE_PIN, HIGH);
        display.showNumberDec(displayCountdown, true);
        Serial.println("Blynk BẬT - V3 = 1");
    } else {
        isSystemPaused = true;
        digitalWrite(BLUE_PIN, LOW);
        display.clear();
        Serial.println("Blynk TẮT - V3 = 0");
    }
}

BLYNK_WRITE(V4) {
    nightLuxLimit = param.asInt();
    Serial.print("Ngưỡng Lux mới: ");
    Serial.println(nightLuxLimit);
}

void readDHT() {
    unsigned long currentMillis = millis();
    if (currentMillis - dhtPreviousMillis >= dhtInterval) {
        dhtPreviousMillis = currentMillis;
        
        humidity = dht.readHumidity();
        temperature = dht.readTemperature();
        
        if (!isnan(humidity) && !isnan(temperature)) {
            Serial.print("Nhiệt độ: ");
            Serial.print(temperature);
            Serial.print("°C | Độ ẩm: ");
            Serial.print(humidity);
            Serial.println("%");
            Blynk.virtualWrite(V1, temperature);
            Blynk.virtualWrite(V2, humidity);
        } else {
            Serial.println("Lỗi đọc DHT22!");
        }
    }
}

float calculateLux(int adcValue) {
    if (adcValue == 0) return 0;
    float voltage = adcValue / 4095.0 * 5.0;
    float resistance = 2000 * voltage / (1 - voltage / 5);
    return pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
}

void updateUptime() {
    unsigned long currentMillis = millis();
    if (currentMillis - uptimePreviousMillis >= uptimeInterval) {
        uptimePreviousMillis = currentMillis;
        unsigned long uptimeSeconds = (currentMillis - startMillis) / 1000;
        Blynk.virtualWrite(V0, uptimeSeconds);
        Serial.print("Thời gian hoạt động: ");
        Serial.print(uptimeSeconds);
        Serial.println(" giây");
    }
}

void checkNightMode() {
    int ldrValue = analogRead(LDR_PIN);
    currentLux = calculateLux(ldrValue);

    if (currentLux < nightLuxLimit) {
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(YELLOW_PIN, HIGH);
        digitalWrite(BLUE_PIN, LOW);
        display.clear();
        
        if (!nightMode) {
            Serial.print("Chế độ ban đêm - Lux: ");
            Serial.println(currentLux);
            nightMode = true;
        }
    } else {
        if (nightMode) {
            digitalWrite(YELLOW_PIN, LOW);
            if (systemEnabled) {
                digitalWrite(GREEN_PIN, HIGH);
                digitalWrite(BLUE_PIN, HIGH);
                Serial.print("Chế độ ban ngày - Lux: ");
                Serial.println(currentLux);
                countdown = 5;
                displayCountdown = countdown;
                trafficState = 0;
            }
            nightMode = false;
        }
    }
}

void loop() {
    Blynk.run();

    // Cập nhật thời gian hoạt động độc lập với trạng thái hệ thống
    updateUptime();

    // Nếu hệ thống bị tắt, chỉ chạy uptime
    if (!systemEnabled) {
        return;
    }

    checkButtonPress();
    readDHT();
    checkNightMode();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        if (!nightMode && countdown < 0 && systemEnabled) {
            changeLight();
        }

        Serial.print("Thời gian còn: ");
        Serial.print(countdown);
        Serial.println(" giây");

        if (!isSystemPaused && !nightMode && systemEnabled) {
            display.showNumberDec(displayCountdown, true);
        }

        countdown--;
        displayCountdown = countdown >= 0 ? countdown : 0;
    }
}
