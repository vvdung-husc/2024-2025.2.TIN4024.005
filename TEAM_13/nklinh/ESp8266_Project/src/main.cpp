#include <Arduino.h>

#define LED_PIN 15  // Chân số 15

void setup() {
    pinMode(LED_PIN, OUTPUT); // Đặt chân 15 là OUTPUT
}

void loop() {
    digitalWrite(LED_PIN, HIGH);  // Bật LED
    delay(500);                   // Đợi 500ms
    digitalWrite(LED_PIN, LOW);   // Tắt LED
    delay(500);                   // Đợi 500ms
}