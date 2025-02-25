#include <Arduino.h>

// Chân kết nối LED
const int RED_LED = 2;
const int YELLOW_LED = 0;
const int GREEN_LED = 15;

// Biến thời gian
unsigned long previousMillis = 0;
int currentState = 0; // 0: Red, 1: Green, 2: Yellow

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
}

void loop() {
    unsigned long currentMillis = millis();
    
    if (currentState == 0 && currentMillis - previousMillis >= 30000) {
        // Chuyển từ đèn đỏ sang đèn xanh
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        currentState = 1;
        previousMillis = currentMillis;
    } 
    else if (currentState == 1 && currentMillis - previousMillis >= 30000) {
        // Chuyển từ đèn xanh sang đèn vàng
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);
        currentState = 2;
        previousMillis = currentMillis;
    } 
    else if (currentState == 2 && currentMillis - previousMillis >= 5000) {
        // Chuyển từ đèn vàng sang đèn đỏ
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        currentState = 0;
        previousMillis = currentMillis;
    }
}
