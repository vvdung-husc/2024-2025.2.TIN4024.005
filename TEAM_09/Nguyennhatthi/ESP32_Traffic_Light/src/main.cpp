#include <Arduino.h>
#include <TM1637Display.h>

#define CLK 18       // Chân CLK của TM1637
#define DIO 19       // Chân DIO của TM1637
#define LED_RED 27   // Đèn đỏ
#define LED_YELLOW 26 // Đèn vàng
#define LED_GREEN 25  // Đèn xanh
#define BUTTON 23     // Nút nhấn

TM1637Display display(CLK, DIO);

unsigned long previousMillis = 0;
int countdownTime = 5;
bool emergency = false;
int phase = 0;
int seconds = countdownTime;

void emergencyMode() {
    emergency = true;
    display.showNumberDec(99);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    
    while (digitalRead(BUTTON) == LOW);
    delay(500);
    
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    emergency = false;
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);  // Kéo lên nội bộ để tránh nhiễu

    display.setBrightness(7);

    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    display.showNumberDec(countdownTime);
}

void loop() {
    if (digitalRead(BUTTON) == LOW) {
        emergencyMode();
    }
    if (emergency) return;

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
        previousMillis = currentMillis;
        display.showNumberDec(seconds);
        seconds--;

        if (seconds < 0) {
            phase = (phase + 1) % 3;
            switch (phase) {
                case 0: // Đèn xanh
                    digitalWrite(LED_RED, LOW);
                    digitalWrite(LED_YELLOW, LOW);
                    digitalWrite(LED_GREEN, HIGH);
                    countdownTime = 5;
                    break;
                case 1: // Đèn vàng
                    digitalWrite(LED_GREEN, LOW);
                    digitalWrite(LED_YELLOW, HIGH);
                    countdownTime = 2;
                    break;
                case 2: // Đèn đỏ
                    digitalWrite(LED_YELLOW, LOW);
                    digitalWrite(LED_RED, HIGH);
                    countdownTime = 5;
                    break;
            }
            seconds = countdownTime;
        }
    }
}
