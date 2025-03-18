#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

// Nguyễn Việt Nam
#define BLYNK_TEMPLATE_ID "TMPL6YeCq4kLS"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
#define BLYNK_AUTH_TOKEN "yHBEJfZ9T7gfaPf_iNwT6ccPgvYH_WGT"


// LÊ BÁ NHẬT MINH
// #define BLYNK_TEMPLATE_ID "TMPL6yean1Ms4"
// #define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
// #define BLYNK_AUTH_TOKEN "LlapeQu1wAa9Xq5m6Mt1BiYAi_EUzieI"

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


// Định nghĩa các chân theo sơ đồ kết nối
#define RED_PIN 27      // Đèn đỏ (với điện trở 220 ohm)
#define YELLOW_PIN 26   // Đèn vàng (với điện trở 330 ohm)
#define GREEN_PIN 25    // Đèn xanh lá
#define BLUE_PIN 21     // Đèn xanh dương
#define BUTTON_PIN 23   // Nút nhấn
#define LDR_PIN 32      // Cảm biến ánh sáng
#define CLK_PIN 18      // Chân CLK của TM1637
#define DIO_PIN 19      // Chân DIO của TM1637
#define DHT_PIN 16      // Chân DHT22

// Cấu hình DHT
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// Các hằng số khác
#define GAMMA 0.7
#define RL10 50
#define DEBOUNCE_TIME 200
#define NIGHT_LUX_THRESHOLD 50
#define HYSTERESIS 50  // Vùng đệm 50 Lux
#define LUX_CHECK_INTERVAL 1000  // Kiểm tra Lux mỗi 1 giây

// Biến toàn cục
bool isPaused = false;    
unsigned long lastButtonPress = 0;
TM1637Display display(CLK_PIN, DIO_PIN);
unsigned long previousMillis = 0;
unsigned long dhtPreviousMillis = 0;
unsigned long luxPreviousMillis = 0; // Thời gian kiểm tra Lux cuối cùng
unsigned long startMillis = 0; // Thời gian bắt đầu hệ thống
long interval = 1000;
long dhtInterval = 2000;  // Đọc DHT mỗi 2 giây
int countdown = 0;
int state = 0;
bool nightMode = false;
float temperature = 0;
float humidity = 0;
bool systemEnabled = true; // Trạng thái hệ thống (bật/tắt từ Blynk)
int luxThreshold = NIGHT_LUX_THRESHOLD;

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
    Blynk.virtualWrite(V3, 1); // Khởi tạo trạng thái bật trên Blynk

    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    Serial.println("Đèn xanh (5 giây)");
    countdown = 5;
    state = 0;
}

void changeLight() {
    if (nightMode) return; // Không đổi đèn nếu ở chế độ ban đêm

    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);

    if (state == 0) {
        digitalWrite(YELLOW_PIN, HIGH);
        Serial.println("Đèn vàng (2 giây)");
        countdown = 2;
        state = 1;
    } else if (state == 1) {
        digitalWrite(RED_PIN, HIGH);
        Serial.println("Đèn đỏ (5 giây)");
        countdown = 5;
        state = 2;
    } else {
        digitalWrite(GREEN_PIN, HIGH);
        Serial.println("Đèn xanh (5 giây)");
        countdown = 5;
        state = 0;
    }
}

void checkButtonPress() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastButtonPress > DEBOUNCE_TIME) {
            isPaused = !isPaused;
            lastButtonPress = currentMillis;

            if (!isPaused) {
                Serial.println("BẬT - V3 = 1");
                Blynk.virtualWrite(V3, 1); // Đồng bộ trạng thái với Blynk
                systemEnabled = true;      // Đồng bộ systemEnabled
            } else {
                Serial.println("TẮT - V3 = 0");
                Blynk.virtualWrite(V3, 0); // Đồng bộ trạng thái với Blynk
                systemEnabled = false;     // Đồng bộ systemEnabled
            }
        }
    }
}

BLYNK_WRITE(V3) {
    int pinValue = param.asInt();
    systemEnabled = pinValue;
    isPaused = !pinValue; // Đồng bộ isPaused với systemEnabled (1 = bật, 0 = tắt)

    if (systemEnabled) {
        Serial.println("Bật từ Blynk - V3 = 1");
    } else {
        Serial.println("Tắt từ Blynk - V3 = 0");
    }
}

BLYNK_WRITE(V4) {
    luxThreshold = param.asInt();
    Serial.print("Ngưỡng Lux cập nhật: ");
    Serial.println(luxThreshold);
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
            Serial.println("Lỗi đọc dữ liệu từ DHT22!");
        }
    }
}

float calculateLux(int adcValue) {
    if (adcValue == 0) return 0;

    float voltage = adcValue / 4095.0 * 5.0; // Giả định nguồn 5V
    float resistance = 2000 * voltage / (1 - voltage / 5);
    float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
    return lux;
}

void updateUptime() {
    unsigned long currentMillis = millis();
    unsigned long uptimeSeconds = (currentMillis - startMillis) / 1000;
    Blynk.virtualWrite(V0, uptimeSeconds);
}

void loop() {
    Blynk.run();

    checkButtonPress();
    readDHT();
    updateUptime();

    unsigned long currentMillis = millis();
    if (currentMillis - luxPreviousMillis >= LUX_CHECK_INTERVAL) {
        luxPreviousMillis = currentMillis;

        float currentLux = calculateLux(analogRead(LDR_PIN));

        // Kiểm tra chế độ với hysteresis
        if (!nightMode && currentLux < (luxThreshold - HYSTERESIS)) {
            digitalWrite(RED_PIN, LOW);
            digitalWrite(GREEN_PIN, LOW);
            digitalWrite(YELLOW_PIN, HIGH);
            digitalWrite(BLUE_PIN, LOW);
            display.clear();
            Serial.println("Chuyển sang chế độ ban đêm - Chỉ đèn vàng sáng");
            nightMode = true;
        } else if (nightMode && currentLux > (luxThreshold + HYSTERESIS)) {
            digitalWrite(YELLOW_PIN, LOW);
            digitalWrite(GREEN_PIN, HIGH);
            digitalWrite(BLUE_PIN, HIGH);
            Serial.println("Chuyển sang chế độ ban ngày - Đèn xanh sáng (5 giây)");
            countdown = 5;
            state = 0;
            nightMode = false;
        }
    }

    // Luôn cập nhật đèn và đếm ngược khi ở chế độ ban ngày
    if (!nightMode) {
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;

            if (countdown < 0) {
                changeLight();
            }

            Serial.print("Còn ");
            Serial.print(countdown);
            Serial.println(" giây");

            // Chỉ hiển thị countdown trên TM1637 khi không pause
            if (!isPaused) {
                display.showNumberDec(countdown, true);
            } else {
                display.clear();
            }

            countdown--;
        }
    }
}