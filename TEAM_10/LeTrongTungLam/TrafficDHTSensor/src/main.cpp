#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

// Chân kết nối
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define CLK 18
#define DIO 19

// Khởi tạo đối tượng
TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT22);

// Biến trạng thái
bool ledState = false;
unsigned long startTime = 0;
unsigned long elapsedTime = 0;
bool buttonPressed = false;

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    display.setBrightness(7); // Độ sáng cao nhất
    dht.begin();
    Serial.begin(115200);
}

void loop() {
    // Đọc trạng thái nút nhấn
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (!buttonPressed) { // Tránh nhấn giữ
            ledState = !ledState;
            buttonPressed = true;

            if (ledState) {
                startTime = millis(); // Lưu thời gian bật LED
                Serial.println("ON");
                display.showNumberDec(0); // Hiển thị "0" ngay lập tức
            } else {
                elapsedTime = millis() - startTime; // Lưu thời gian hoạt động
                Serial.println("OFF");
                display.clear(); // Xóa màn hình khi tắt
            }

            digitalWrite(LED_PIN, ledState);
        }
    } else {
        buttonPressed = false;
    }

    // Cập nhật thời gian trên màn hình mỗi giây khi LED bật
    if (ledState) {
        static unsigned long lastUpdate = 0;
        if (millis() - lastUpdate >= 1000) {
            lastUpdate = millis();
            unsigned long seconds = (millis() - startTime) / 1000;
            display.showNumberDec(seconds);
        }
    }

    // Đọc cảm biến DHT22 mỗi 2 giây
    static unsigned long lastDHTRead = 0;
    if (millis() - lastDHTRead > 2000) {
        lastDHTRead = millis();
        float temp = dht.readTemperature();
        float hum = dht.readHumidity();
        Serial.print("Temp: "); Serial.print(temp); Serial.print("C, ");
        Serial.print("Humidity: "); Serial.print(hum); Serial.println("%");
    }
}
