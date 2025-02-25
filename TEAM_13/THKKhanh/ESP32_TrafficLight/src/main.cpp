#include <Arduino.h>
#include <TM1637Display.h>

#define LDR_PIN 13
#define RED_LED 27
#define YELLOW_LED 26
#define GREEN_LED 25
#define NIGHT_LED 21
#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(NIGHT_LED, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    
    display.setBrightness(7);
    Serial.begin(115200);
}

void loop() {
    int lightLevel = analogRead(LDR_PIN);
    Serial.print("Light Level: ");
    Serial.println(lightLevel);

    if (lightLevel < 500) { // Giả sử <500 là ban đêm
        digitalWrite(RED_LED, LOW);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
        for (int i = 5; i > 0; i--) {
            digitalWrite(NIGHT_LED, HIGH);
            display.showNumberDec(i);
            delay(1000);
            digitalWrite(NIGHT_LED, LOW);
            delay(500);
        }
    } else {
        digitalWrite(NIGHT_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        for (int i = 10; i > 0; i--) {
            display.showNumberDec(i);
            delay(1000);
        }
        
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        for (int i = 7; i > 0; i--) {
            display.showNumberDec(i);
            delay(1000);
        }
        
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);
        for (int i = 3; i > 0; i--) {
            display.showNumberDec(i);
            delay(1000);
        }
        
        digitalWrite(YELLOW_LED, LOW);
    }
}