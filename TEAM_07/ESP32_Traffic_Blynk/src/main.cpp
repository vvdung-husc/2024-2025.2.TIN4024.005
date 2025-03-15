// Lê Xuân Chương
// #define BLYNK_TEMPLATE_ID "TMPL6Flpzk0c5"
// #define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
// #define BLYNK_AUTH_TOKEN "JaWxIxhzYLLI98nHx2UUeKHEmP7tVqud"

// Hồ Hữu Khải
// #define BLYNK_TEMPLATE_ID "TMPL68mY_wCNz"
// #define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
// #define BLYNK_AUTH_TOKEN "BfA2hzbN9WMSOFqqQSsIDt0NG8Ub-3II"

//Lê Ngọc THịnh
// #define BLYNK_TEMPLATE_ID "TMPL6DQGZ9ciV"
// #define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
// #define BLYNK_AUTH_TOKEN "i715K5_IZLDspBO4bHaCCPcbgpsETrYN"


//Tưởng Văn Đồng
// #define BLYNK_TEMPLATE_ID "TMPL6gJRE1PA0"
// #define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
// #define BLYNK_AUTH_TOKEN "90pyiscc8tA8wRbCxF_hetwm4Rk2BHga"

// Lê Hữu Minh DUệ
#define BLYNK_TEMPLATE_ID "TMPL6RVIr6RuJ"
#define BLYNK_TEMPLATE_NAME "BTH2"
#define BLYNK_AUTH_TOKEN "KhGyGe2kFh8Ct7QvW7dfS0QP6W-je-im"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include "DHTesp.h"

// Chân kết nối
#define GREEN_LED 32
#define YELLOW_LED 25
#define RED_LED 26
#define BUTTON_PIN 18
#define LDR_PIN 34
#define TM1637_CLK 4
#define TM1637_DIO 16
#define DHTPIN 15

// Thời gian đèn
const int GREEN_TIME = 5;
const int YELLOW_TIME = 3;
const int RED_TIME = 4;

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* pass = "";

// Đối tượng
TM1637Display display(TM1637_CLK, TM1637_DIO);
DHTesp dht;

// Trạng thái
bool displayEnabled = true;  // Hiển thị giây đếm ngược
bool nightMode = false;
bool lastNightMode = false;
bool firstRun = true;
volatile bool buttonPressed = false;

int lightThreshold = 1000;  
int currentLdrValue = 0;
int countdown = GREEN_TIME;
int currentLight = 0;

unsigned long lastMillis = 0;
unsigned long lastDHTRead = 0;
unsigned long lastButtonPress = 0;

// Nhận tín hiệu bật/tắt hiển thị giây từ Blynk
BLYNK_WRITE(V1) {
    displayEnabled = param.asInt();
    Serial.printf("Hiển thị giây cập nhật từ Blynk: %s\n", displayEnabled ? "BẬT" : "TẮT");
}

// Nhận ngưỡng ánh sáng từ Blynk
BLYNK_WRITE(V4) {
    int newThreshold = param.asInt();
    if (newThreshold != lightThreshold) {
        lightThreshold = newThreshold;
        Serial.printf("Ngưỡng ánh sáng cập nhật từ Blynk: %d lx\n", lightThreshold);
    }
}

// Đồng bộ giá trị từ Blynk khi kết nối
BLYNK_CONNECTED() {
    Blynk.syncVirtual(V1);  // Đồng bộ hiển thị giây
    Blynk.syncVirtual(V4);  // Đồng bộ ngưỡng sáng
    Serial.println("Đồng bộ giá trị từ Blynk.");
}

// Xử lý nút nhấn trên mạch để bật/tắt hiển thị giây
void IRAM_ATTR handleButtonPress() {
    if (millis() - lastButtonPress > 300) {
        buttonPressed = true;
        lastButtonPress = millis();
    }
}

// Gửi dữ liệu độ sáng thực tế lên Blynk
void sendLdrData() {
    Blynk.virtualWrite(V4, currentLdrValue);
    Serial.printf("Độ sáng hiện tại: %d lx - Ngưỡng: %d lx\n", currentLdrValue, lightThreshold);
}

// Gửi dữ liệu DHT lên Blynk
void sendDHTData() {
    TempAndHumidity data = dht.getTempAndHumidity();
    Blynk.virtualWrite(V2, data.temperature);
    Blynk.virtualWrite(V3, data.humidity);
    Serial.printf("Nhiệt độ: %.1f°C - Độ ẩm: %.1f%%\n", data.temperature, data.humidity);
}

// Cập nhật đèn giao thông
void updateTrafficLight() {
    digitalWrite(GREEN_LED, currentLight == 0);
    digitalWrite(YELLOW_LED, currentLight == 1);
    digitalWrite(RED_LED, currentLight == 2);
}

// Tắt toàn bộ đèn
void turnOffAllLights() {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
}

// Nhấp nháy đèn vàng
void blinkYellowLight() {
    static bool yellowState = false;
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink >= 500) {
        yellowState = !yellowState;
        digitalWrite(YELLOW_LED, yellowState);
        lastBlink = millis();
    }
}

// Hiển thị số đếm ngược
void displayNumber(int num) {
    if (displayEnabled) {
        display.showNumberDec(num, false);
    } else {
        display.clear();
    }
}

// Setup
void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi đã kết nối!");

    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();

    pinMode(GREEN_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

    display.setBrightness(7);
    display.clear();
    dht.setup(DHTPIN, DHTesp::DHT22);
}

// Loop
void loop() {
    Blynk.run();

    // Xử lý nút nhấn để bật/tắt hiển thị giây
    if (buttonPressed) {
        displayEnabled = !displayEnabled;
        Blynk.virtualWrite(V1, displayEnabled);
        Serial.printf("Hiển thị giây thay đổi từ nút nhấn: %s\n", displayEnabled ? "BẬT" : "TẮT");
        buttonPressed = false;
    }

    // Đọc giá trị cảm biến ánh sáng
    int newLdrValue = analogRead(LDR_PIN);
    if (newLdrValue != currentLdrValue) {
        currentLdrValue = newLdrValue;
        sendLdrData();  // Gửi giá trị độ sáng lên Blynk
    }

    // Xác định chế độ sáng / tối
    nightMode = (currentLdrValue < lightThreshold);

    if (nightMode != lastNightMode) {
        if (nightMode) {
            Serial.println("TRỜI TỐI - BẬT ĐÈN VÀNG NHẤP NHÁY");
            turnOffAllLights();
        } else {
            Serial.println("TRỜI SÁNG - ĐÈN GIAO THÔNG HOẠT ĐỘNG");
            currentLight = 0;
            countdown = GREEN_TIME;
            updateTrafficLight();
        }
        lastNightMode = nightMode;
    }

    if (nightMode) {
        blinkYellowLight();
        display.clear();
    } else {
        if (millis() - lastMillis >= 1000) {
            lastMillis = millis();
            countdown--;
            if (countdown <= 0) {
                currentLight = (currentLight + 1) % 3;
                countdown = (currentLight == 0) ? GREEN_TIME : (currentLight == 1) ? YELLOW_TIME : RED_TIME;
                updateTrafficLight();
            }
        }
        displayNumber(countdown);
    }

    Blynk.virtualWrite(V0, countdown);

    if (millis() - lastDHTRead >= 5000) {
        lastDHTRead = millis();
        sendDHTData();
    }

    delay(50);
}
