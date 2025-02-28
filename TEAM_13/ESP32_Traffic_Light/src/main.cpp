#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân kết nối
#define BUTTON_PIN 23
#define LED_BLUE 21
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27
#define LDR_PIN 13
#define CLK 18
#define DIO 19

// Khởi tạo đối tượng hiển thị TM1637
TM1637Display display(CLK, DIO);

// Biến toàn cục
int buttonState = 0;
int lastButtonState = HIGH;
bool displayOn = true;
int ldrValue = 0;
int countdown = 10; // Bắt đầu đếm từ 10 (cho đèn xanh)
unsigned long previousMillis = 0;
const long interval = 1000; // Thời gian đếm lùi (1 giây)
unsigned long blinkMillis = 0;
const long blinkInterval = 500; // Tốc độ nhấp nháy đèn vàng
const long debounceDelay = 50; // Thời gian chống rung nút
unsigned long lastDebounceTime = 0;
bool yellowBlinkState = false; // Trạng thái nhấp nháy của đèn vàng

enum TrafficLightState { GREEN, YELLOW, RED }; // Trạng thái đèn giao thông
TrafficLightState lightState = GREEN;         // Đèn bắt đầu từ xanh

void setup() {
    // Cài đặt chế độ cho các chân
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(LDR_PIN, INPUT);

    // Cấu hình hiển thị
    display.setBrightness(0x0F); // Độ sáng tối đa
    display.showNumberDec(countdown);

    // Khởi động Serial để debug
    Serial.begin(115200);
}

void loop() {
    unsigned long currentMillis = millis();

    // Đọc giá trị cảm biến ánh sáng (LDR) và chuyển đổi sang lux
    int ldrRaw = analogRead(LDR_PIN);
    float ldrLux = map(ldrRaw, 0, 4095, 0, 1000); // Giả định 0-4095 tương ứng 0-1000 lux
    Serial.print("LDR Lux: ");
    Serial.println(ldrLux);

    // Kiểm tra nút nhấn để bật/tắt hiển thị
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH && (currentMillis - lastDebounceTime) > debounceDelay) {
        displayOn = !displayOn;
        if (!displayOn) {
            display.clear();
        }
        lastDebounceTime = currentMillis;
    }
    lastButtonState = buttonState;

    // Nếu trời tối (lux < 50), chỉ bật đèn vàng nhấp nháy
    if (ldrLux < 50) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, LOW);

        if (currentMillis - blinkMillis >= blinkInterval) {
            blinkMillis = currentMillis;
            yellowBlinkState = !yellowBlinkState;
            digitalWrite(LED_YELLOW, yellowBlinkState);
        }

        if (displayOn) display.clear(); // Không hiển thị khi trời tối
        return; // Dừng loop để không chạy logic đèn giao thông
    }

    // Nếu trời sáng (lux >= 50), chạy đèn giao thông bình thường
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        countdown--;

        if (countdown < 0) {
            switch (lightState) {
                case GREEN:
                    lightState = YELLOW;
                    countdown = 3;
                    break;
                case YELLOW:
                    lightState = RED;
                    countdown = 10;
                    break;
                case RED:
                    lightState = GREEN;
                    countdown = 10;
                    break;
            }
        }

        if (displayOn) {
            display.showNumberDec(countdown);
        }
    }

    // Điều khiển đèn giao thông
    switch (lightState) {
        case GREEN:
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_RED, LOW);
            break;
        case YELLOW:
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_YELLOW, HIGH);
            digitalWrite(LED_RED, LOW);
            break;
        case RED:
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_RED, HIGH);
            break;
    }
}
