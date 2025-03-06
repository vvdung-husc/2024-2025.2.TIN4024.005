#include <Arduino.h>
#include <TM1637Display.h>

#define RED_LED 27
#define YELLOW_LED 26
#define GREEN_LED 25
#define LDR_PIN 13
#define CLK 18
#define DIO 19
#define BUTTON_PIN 23
#define RIGHT_LED 21 

TM1637Display display(CLK, DIO);
bool isScreenOn = true; 

void checkButton() {
    if (digitalRead(BUTTON_PIN) == LOW) { 
        isScreenOn = false;  
        display.setBrightness(0);
        display.clear(); 
        digitalWrite(RIGHT_LED, HIGH); 
    } else {
        isScreenOn = true;  
        display.setBrightness(7);
        digitalWrite(RIGHT_LED, LOW); 
    }
}

void setLight(int red, int yellow, int green, int time) {
    digitalWrite(RED_LED, red);
    digitalWrite(YELLOW_LED, yellow);
    digitalWrite(GREEN_LED, green);

    unsigned long startTime = millis();
    while (millis() - startTime < time * 1000) {
        checkButton(); 
        int lightLevel = analogRead(LDR_PIN);

       
        if (lightLevel < 1000) return;

        if (isScreenOn) {
            int remainingTime = time - (millis() - startTime) / 1000;
            display.showNumberDec(remainingTime, true);
        } else {
            display.clear(); 
        }
        delay(200);
    }
}

void nightMode() {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);

    while (analogRead(LDR_PIN) < 1000) { 
        checkButton(); 

      
        if (isScreenOn) {
            display.showNumberDec(0, true);
        } else {
            display.clear();
        }

        digitalWrite(YELLOW_LED, HIGH); 
        delay(500);
        digitalWrite(YELLOW_LED, LOW); 
    }

    display.clear();
}

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RIGHT_LED, OUTPUT);
    display.setBrightness(7);
    Serial.begin(115200); 
}

void loop() {
    checkButton();

    int lightLevel = analogRead(LDR_PIN);
    Serial.println(lightLevel);

    if (lightLevel < 1000) {
        nightMode();
    } else {
        setLight(1, 0, 0, 10); 
        setLight(0, 1, 0, 3);  
        setLight(0, 0, 1, 8);  
    }
}
