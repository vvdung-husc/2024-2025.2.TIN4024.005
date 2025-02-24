#include <Arduino.h>

#include <TM1637Display.h>
//
#define CLK 18
#define DIO 19
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define BUTTON 23

TM1637Display display(CLK, DIO);
int state = 0;
unsigned long previousMillis = 0;
int countdownTime = 5;
bool emergency = false;

void emergencyMode() {
    emergency = true;
    display.showNumberDec(99);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    
    // Chờ trong chế độ khẩn cấp cho đến khi nút được nhả ra
    while (digitalRead(BUTTON) == LOW);
    delay(500); // Tránh trigger lại ngay lập tức
    
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    emergency = false;
}

void setup() {
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);
    display.setBrightness(7);
}

void loop() {
    if (digitalRead(BUTTON) == LOW) {
        emergencyMode();
    }

    if (emergency) return; // Không chạy logic bình thường nếu đang ở chế độ khẩn cấp

    unsigned long currentMillis = millis();
    static int phase = 0;
    static int seconds = countdownTime;

    if (currentMillis - previousMillis >= 1000) {
        previousMillis = currentMillis;
        display.showNumberDec(seconds);
        seconds--;

        if (seconds < 0) {
            phase = (phase + 1) % 3;
            switch (phase) {
                case 0: // Đèn Xanh
                    digitalWrite(LED_RED, LOW);
                    digitalWrite(LED_YELLOW, LOW);
                    digitalWrite(LED_GREEN, HIGH);
                    countdownTime = 5;
                    break;
                case 1: // Đèn Vàng
                    digitalWrite(LED_GREEN, LOW);
                    digitalWrite(LED_YELLOW, HIGH);
                    countdownTime = 2;
                    break;
                case 2: // Đèn Đỏ
                    digitalWrite(LED_YELLOW, LOW);
                    digitalWrite(LED_RED, HIGH);
                    countdownTime = 5;
                    break;
            }
            seconds = countdownTime;
        }
    }
}
