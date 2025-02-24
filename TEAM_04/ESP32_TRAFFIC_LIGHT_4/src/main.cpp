#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa các chân LED
#define led 21
#define ledRed 27
#define ledYellow 26
#define ledGreen 25

// Định nghĩa các chân kết nối cho module hiển thị TM1637
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// Định nghĩa chân nút nhấn và cảm biến ánh sáng (LDR)
#define buttonPin 23
#define ldrPin 13  // Chân analog của LDR

// Định nghĩa thời gian cho từng pha của đèn giao thông
const uint32_t RED_TIME = 5000;
const uint32_t YELLOW_TIME = 3000;
const uint32_t GREEN_TIME = 10000;

// Biến trạng thái hệ thống
int currentState = 0;  // Trạng thái hiện tại của đèn giao thông
int countdown = 0;      // Thời gian đếm ngược
uint32_t preTime = 0;   // Biến lưu thời gian trước đó
bool isDisplayOn = true;
bool isLedOn = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Hàm kiểm tra khoảng thời gian không chặn chương trình
bool isReady(uint32_t &ulTimer, uint32_t milisecond) {
    uint32_t t = millis();
    if (t - ulTimer < milisecond) return false;
    ulTimer = t;
    return true;
}

// Cập nhật hiển thị số đếm ngược trên module TM1637
void UpdateDisplay(int seconds) {
    if (isDisplayOn) {
        display.showNumberDec(seconds, true);
    } else {
        display.clear();
    }
}

// Xử lý khi nhấn nút để bật/tắt màn hình
void checkButtonPress() {
    bool buttonState = digitalRead(buttonPin);

    // Kiểm tra xem nút có được nhấn hay không
    if (buttonState == LOW && lastButtonState == HIGH) {
        if (millis() - lastDebounceTime > debounceDelay) {
            isDisplayOn = !isDisplayOn;  // Đảo trạng thái hiển thị
            isLedOn = !isLedOn;
            UpdateDisplay(countdown);    // Cập nhật hiển thị theo trạng thái mới
            Serial.println(isDisplayOn ? "Màn hình bật" : "Màn hình tắt");
            lastDebounceTime = millis(); // Cập nhật thời gian debounce
        }
    }
    lastButtonState = buttonState;
}

// Cập nhật trạng thái LED
void checkLed() {
    digitalWrite(led, isLedOn ? LOW : HIGH);
}

// Điều khiển nhấp nháy đèn vàng khi cần thiết
void blinkYellowLed() {
    static uint32_t lastBlinkTime = 0;
    static bool ledState = LOW;

    if (millis() - lastBlinkTime >= 500) {  // Nhấp nháy mỗi 500ms
        lastBlinkTime = millis();
        ledState = !ledState;
        digitalWrite(ledYellow, ledState);
    }
}

// Điều khiển trạng thái đèn giao thông không chặn chương trình
void Non_block() {
    static uint32_t countdownTimer = millis();
    uint32_t currentMillis = millis(); // Chỉ gọi millis() một lần

    // Giảm thời gian đếm ngược sau mỗi giây
    if (currentMillis - countdownTimer >= 1000) {
        countdownTimer = currentMillis;
        if (countdown > 0) {
countdown--;
            UpdateDisplay(countdown);
        }
    }

    // Đọc giá trị từ cảm biến ánh sáng (LDR)
    int ldrValue = analogRead(ldrPin);
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);

    // Nếu ánh sáng yếu, đèn vàng nhấp nháy, các đèn khác tắt
    if (ldrValue < 500) {
        digitalWrite(ledRed, LOW);
        digitalWrite(ledGreen, LOW);
        digitalWrite(ledYellow, LOW);
        blinkYellowLed();
        return;
    }

    // Điều khiển các trạng thái đèn giao thông
    switch (currentState) {
        case 0: // Đèn xanh
            digitalWrite(ledGreen, HIGH);
            digitalWrite(ledYellow, LOW);
            digitalWrite(ledRed, LOW);
            if (isReady(preTime, GREEN_TIME)) {
                currentState = 1;
                countdown = YELLOW_TIME / 1000;
                Serial.println("Đèn vàng sáng!");
            }
            break;

        case 1: // Đèn vàng
            digitalWrite(ledGreen, LOW);
            digitalWrite(ledYellow, HIGH);
            digitalWrite(ledRed, LOW);
            if (isReady(preTime, YELLOW_TIME)) {
                currentState = 2;
                countdown = RED_TIME / 1000;
                Serial.println("Đèn đỏ sáng!");
            }
            break;

        case 2: // Đèn đỏ
            digitalWrite(ledGreen, LOW);
            digitalWrite(ledYellow, LOW);
            digitalWrite(ledRed, HIGH);
            if (isReady(preTime, RED_TIME)) {
                currentState = 0;
                countdown = GREEN_TIME / 1000;
                Serial.println("Đèn xanh sáng!");
            }
            break;
    }
}

// Cấu hình ban đầu
void setup() {
    pinMode(ledRed, OUTPUT);
    pinMode(ledYellow, OUTPUT);
    pinMode(ledGreen, OUTPUT);
    pinMode(led, OUTPUT);
    pinMode(ldrPin, INPUT);
    pinMode(buttonPin, INPUT_PULLUP); // Sử dụng PULLUP để tránh nhiễu

    // Khởi tạo trạng thái ban đầu của đèn
    digitalWrite(ledRed, LOW);
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledGreen, HIGH);
    digitalWrite(led, LOW);

    Serial.begin(115200);
    display.setBrightness(7); // Đặt độ sáng cho màn hình hiển thị
    currentState = 0;
    countdown = GREEN_TIME / 1000;
    UpdateDisplay(countdown);
}

// Vòng lặp chính của chương trình
void loop() {
    checkButtonPress(); // Kiểm tra trạng thái nút nhấn
    checkLed();         // Kiểm tra trạng thái LED phụ
    Non_block();        // Kiểm soát đèn giao thông không chặn chương trình
}