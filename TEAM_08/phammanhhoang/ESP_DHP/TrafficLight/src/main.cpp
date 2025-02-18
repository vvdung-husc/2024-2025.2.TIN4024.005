#include <Arduino.h>

#define RED_LED 13      // Chân nối đèn đỏ
#define YELLOW_LED 12   // Chân nối đèn vàng
#define GREEN_LED 14    // Chân nối đèn xanh
#define BUTTON_PIN 5    // Chân nối nút nhấn

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Nút nhấn có điện trở kéo lên

    // Tắt hết đèn ban đầu
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) { // Khi nút được nhấn
        // Bật đèn xanh trong 5 giây
        digitalWrite(GREEN_LED, HIGH);
        delay(5000);
        digitalWrite(GREEN_LED, LOW);

        // Bật đèn vàng trong 2 giây
        digitalWrite(YELLOW_LED, HIGH);
        delay(2000);
        digitalWrite(YELLOW_LED, LOW);

        // Bật đèn đỏ trong 5 giây
        digitalWrite(RED_LED, HIGH);
        delay(5000);
        digitalWrite(RED_LED, LOW);
    }
}
