#include <Arduino.h>

#define RED_PIN 18    // Chân GPIO cho đèn đỏ (phù hợp với diagram.json)
#define YELLOW_PIN 19 // Chân GPIO cho đèn vàng
#define GREEN_PIN 5   // Chân GPIO cho đèn xanh

unsigned long previousMillis = 0;      // Lưu thời gian trước đó
const unsigned long redTime = 7000;    // 7 giây cho đèn đỏ
const unsigned long yellowTime = 4000; // 4 giây cho đèn vàng
const unsigned long greenTime = 7000;  // 7 giây cho đèn xanh
int currentState = 2;                  // Trạng thái bắt đầu từ đèn đỏ
int lastState = -1;                    // Trạng thái trước đó

void setup()
{
    Serial.begin(115200);
    pinMode(RED_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentState != lastState)
    {
        previousMillis = currentMillis; // Cập nhật ngay khi vào trạng thái mới

        if (lastState == 0)
            Serial.println("Đèn xanh: OFF");
        if (lastState == 1)
            Serial.println("Đèn vàng: OFF");
        if (lastState == 2)
            Serial.println("Đèn đỏ: OFF");

        lastState = currentState;
        if (currentState == 0)
            Serial.println("Đèn xanh: ON");
        if (currentState == 1)
            Serial.println("Đèn vàng: ON");
        if (currentState == 2)
            Serial.println("Đèn đỏ: ON");
    }

    switch (currentState)
    {
    case 2: // Đèn đỏ
        digitalWrite(RED_PIN, HIGH);
        digitalWrite(YELLOW_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);
        if (currentMillis - previousMillis >= redTime)
        {
            previousMillis = currentMillis;
            currentState = 0; // Chuyển sang đèn xanh
        }
        break;

    case 0: // Đèn xanh
        digitalWrite(RED_PIN, LOW);
        digitalWrite(YELLOW_PIN, LOW);
        digitalWrite(GREEN_PIN, HIGH);
        if (currentMillis - previousMillis >= greenTime)
        {
            previousMillis = currentMillis;
            currentState = 1; // Chuyển sang đèn vàng
        }
        break;

    case 1: // Đèn vàng
        digitalWrite(RED_PIN, LOW);
        digitalWrite(YELLOW_PIN, HIGH);
        digitalWrite(GREEN_PIN, LOW);
        if (currentMillis - previousMillis >= yellowTime)
        {
            previousMillis = currentMillis;
            currentState = 2; // Quay lại đèn đỏ
        }
        break;
    }
}