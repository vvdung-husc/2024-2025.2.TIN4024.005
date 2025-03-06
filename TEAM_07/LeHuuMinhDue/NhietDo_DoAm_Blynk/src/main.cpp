#define BLYNK_TEMPLATE_ID "TMPL6xlKHTdLD"
#define BLYNK_TEMPLATE_NAME "DoNhietDo"
#define BLYNK_AUTH_TOKEN "zp1K5K5sGH3ZwEJUfHczuQch7mSCf5qE"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include "DHTesp.h"  // Thư viện DHT trên Wokwi

// Chân kết nối
#define CLK 32
#define DIO 33
#define LED_PIN 35
#define BUTTON_PIN 26
#define DHTPIN 21

// Đối tượng
TM1637Display display(CLK, DIO);
DHTesp dht;  // DHT trên Wokwi

const char* ssid = "Wokwi-GUEST";
const char* pass = "";

bool displayEnabled = true;
volatile bool buttonPressed = false;
unsigned long lastMillis = 0;
unsigned long lastButtonPress = 0;
unsigned long startMillis = 0;  // Thời điểm bắt đầu chạy (để tính uptime)

// Chống dội nút
void IRAM_ATTR handleButtonPress() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastButtonPress > 300) {  // debounce 300ms
        buttonPressed = true;  // Đánh dấu nút đã nhấn
        lastButtonPress = currentMillis;
    }
}

// Blynk: Điều khiển bật/tắt hiển thị qua V1
BLYNK_WRITE(V1) {
    displayEnabled = param.asInt();
}

// Đọc và gửi cảm biến DHT22 lên Blynk
void sendSensorData() {
    TempAndHumidity data = dht.getTempAndHumidity();

    if (isnan(data.temperature) || isnan(data.humidity)) {
        Serial.println("Lỗi đọc DHT22!");
        return;
    }

    Blynk.virtualWrite(V2, data.temperature);  // Gửi nhiệt độ
    Blynk.virtualWrite(V3, data.humidity);     // Gửi độ ẩm

    Serial.print("Nhiệt độ: ");
    Serial.print(data.temperature);
    Serial.print(" °C, Độ ẩm: ");
    Serial.print(data.humidity);
    Serial.println(" %");
}

// Kết nối WiFi và Blynk
void connectWiFiAndBlynk() {
    WiFi.begin(ssid, pass);
    int timeout = 20;  // 20 x 500ms = 10 giây
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(500);
        Serial.print(".");
        timeout--;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi đã kết nối!");
        Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    } else {
        Serial.println("\nKết nối WiFi thất bại!");
    }
}

void setup() {
    Serial.begin(115200);

    connectWiFiAndBlynk();

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, handleButtonPress, FALLING);

    display.setBrightness(7);

    dht.setup(DHTPIN, DHTesp::DHT22);  // Khởi động DHT22
    Serial.println("Setup hoàn tất!");

    startMillis = millis();  // Lưu thời điểm bắt đầu chạy
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        Blynk.run();
    } else {
        Serial.println("Mất kết nối WiFi...");
        delay(1000);
    }

    // Xử lý nút nhấn để bật/tắt đồng hồ
    if (buttonPressed) {
        displayEnabled = !displayEnabled;
        buttonPressed = false;  // Reset cờ sau khi xử lý
        Blynk.virtualWrite(V1, displayEnabled);  // Đồng bộ với Blynk
    }

    // Tính số giây đã chạy từ khi bắt đầu
    unsigned long currentSeconds = (millis() - startMillis) / 1000;

    // Hiển thị thời gian hoạt động trên TM1637
    if (displayEnabled) {
        display.showNumberDec(currentSeconds % 100, true);
    } else {
        display.clear();
    }

    // Điều khiển LED theo trạng thái
    digitalWrite(LED_PIN, displayEnabled ? HIGH : LOW);

    // Đọc và gửi nhiệt độ, độ ẩm mỗi 5 giây
    if (millis() - lastMillis > 5000) {
        sendSensorData();
        lastMillis = millis();
    }

    // Gửi thời gian hoạt động lên Blynk
    Blynk.virtualWrite(V0, currentSeconds);

    delay(100);  // Giảm tải CPU
}
