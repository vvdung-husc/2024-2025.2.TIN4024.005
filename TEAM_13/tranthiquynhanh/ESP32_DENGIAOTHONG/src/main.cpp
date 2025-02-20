#include <Arduino.h>
#include <TM1637Display.h>

#define LED_GREEN 17
#define LED_YELLOW 18
#define LED_RED 19
#define BTN 13
#define CLK 25
#define DIO 26

TM1637Display display(CLK, DIO);
bool isPaused = false;

void setup() {
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(BTN, INPUT_PULLUP);
    display.setBrightness(7);
    display.clear();
}

void countDown(int seconds, int ledPin) {
    digitalWrite(ledPin, HIGH);
    for (int i = seconds; i >= 0; i--) {
        if (digitalRead(BTN) == LOW) {
            isPaused = !isPaused;
            while (digitalRead(BTN) == LOW);  // Chờ nút nhả ra
            if (isPaused) {
                // Tắt tất cả đèn LED và màn hình hiển thị
                digitalWrite(LED_GREEN, LOW);
                digitalWrite(LED_YELLOW, LOW);
                digitalWrite(LED_RED, LOW);
                display.clear();
                while (digitalRead(BTN) == HIGH); // Chờ nhấn nút để tiếp tục
            }
            return;
        }
        if (!isPaused) {
            display.showNumberDec(i); // Hiển thị số đếm ngược trên bảng LED
            delay(1000);
        }
    }
    digitalWrite(ledPin, LOW);
}

void loop() {
    if (!isPaused) {
        countDown(7, LED_GREEN);
        countDown(7, LED_RED);
        countDown(3, LED_YELLOW);
    }
}

