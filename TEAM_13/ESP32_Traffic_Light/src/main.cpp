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

    // Đọc giá trị cảm biến ánh sáng (LDR)
    ldrValue = analogRead(LDR_PIN);
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);

    // Kiểm tra nút nhấn để bật/tắt hiển thị
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH && (currentMillis - lastDebounceTime) > debounceDelay) {
        displayOn = !displayOn; // Bật hoặc tắt hiển thị
        if (!displayOn) {
            display.clear();
            digitalWrite(LED_BLUE, HIGH); // Bật đèn xanh dương khi tắt bảng đếm số
        } else {
            digitalWrite(LED_BLUE, LOW);  // Tắt đèn xanh dương khi mở bảng đếm số
        }
        lastDebounceTime = currentMillis; // Cập nhật thời gian nút nhấn
    }
    lastButtonState = buttonState;

    // Countdown luôn chạy
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        countdown--;

        if (countdown < 0) {
            // Chuyển trạng thái khi hết thời gian đếm
            switch (lightState) {
                case GREEN:
                    lightState = YELLOW;  // Chuyển sang đèn vàng
                    countdown = 3;       // Đếm 3 giây cho đèn vàng
                    break;
                case YELLOW:
                    lightState = RED;    // Chuyển sang đèn đỏ
                    countdown = 10;      // Đếm 10 giây cho đèn đỏ
                    break;
                case RED:
                    lightState = GREEN;  // Quay lại đèn xanh
                    countdown = 10;      // Đếm 10 giây cho đèn xanh
                    break;
            }
        }

        // Cập nhật hiển thị nếu đang bật
        if (displayOn) {
            display.showNumberDec(countdown);
        }
    }

    // Xử lý đèn giao thông
    if (ldrValue > 1000) {  // Trời tối, đèn vàng nhấp nháy
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, LOW);

        if (currentMillis - blinkMillis >= blinkInterval) {
            blinkMillis = currentMillis;
            yellowBlinkState = !yellowBlinkState;
            digitalWrite(LED_YELLOW, yellowBlinkState);
        }
        if (displayOn) display.clear(); // Không hiển thị khi trời tối
    } else {  // Ban ngày, đèn hoạt động bình thường
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
}
