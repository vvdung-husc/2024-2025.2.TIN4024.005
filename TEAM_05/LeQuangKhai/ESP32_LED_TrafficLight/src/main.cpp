#include <Arduino.h>
#include <TM1637Display.h>

#define LED_GREEN 17   // Đèn xanh
#define LED_YELLOW 5   // Đèn vàng
#define LED_RED 4      // Đèn đỏ
#define CLK 26         // Chân CLK của TM1637
#define DIO 25         // Chân DIO của TM1637

TM1637Display display(CLK, DIO);

ulong timeStart = 0;
int state = 0; // Bắt đầu từ đèn xanh
const uint32_t intervals[] = {10000, 5000, 9000}; // Thời gian: Xanh, Vàng, Đỏ
int countdown = intervals[0] / 1000; // Lấy giá trị thời gian của đèn xanh

bool IsReady(ulong &iTimer, uint32_t milisecond) {
    ulong t = millis();
    if (t - iTimer < milisecond) return false;
    iTimer = t;
    return true;
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    display.setBrightness(7);

    // Khởi tạo trạng thái ban đầu (đèn xanh)
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    Serial.println("Đèn Xanh Bật (Xe đi)");
}

void loop() {
    if (IsReady(timeStart, 1000)) {
        if (countdown > 0) {
            countdown--; // Giảm thời gian đếm ngược
        } else {
            state = (state + 1) % 3; // Chuyển trạng thái
            countdown = intervals[state] / 1000; // Lấy thời gian mới

            switch (state) {
                case 0:
                    digitalWrite(LED_GREEN, HIGH);
                    digitalWrite(LED_YELLOW, LOW);
                    digitalWrite(LED_RED, LOW);
                    Serial.println("Đèn Xanh Bật (Xe đi)");
                    break;
                case 1:
                    digitalWrite(LED_GREEN, LOW);
                    digitalWrite(LED_YELLOW, HIGH);
                    digitalWrite(LED_RED, LOW);
                    Serial.println("Đèn Vàng Bật (Chuẩn bị dừng)");
                    break;
                case 2:
                    digitalWrite(LED_GREEN, LOW);
                    digitalWrite(LED_YELLOW, LOW);
                    digitalWrite(LED_RED, HIGH);
                    Serial.println("Đèn Đỏ Bật (Dừng lại)");
                    break;
            }
        }
        display.showNumberDec(countdown);
    }
}
