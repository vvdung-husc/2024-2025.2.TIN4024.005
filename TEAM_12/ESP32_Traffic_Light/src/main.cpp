#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân
#define BUTTON_PIN 23
#define LED_BLUE 21
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27
#define LDR_PIN 13
#define CLK 18
#define DIO 19

// Khởi tạo TM1637
TM1637Display display(CLK, DIO);

bool displayOn = true;       
bool yellowBlink = false;    
int countdown = 10;          
unsigned long lastMillis = 0;
unsigned long buttonPressTime = 0;
unsigned long lastBlinkMillis = 0;
const long blinkDelay = 500;
const long debounceTime = 50;

enum TrafficState { GREEN, YELLOW, RED };
TrafficState currentState = GREEN;

// Thiết lập ban đầu
void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(LDR_PIN, INPUT);

    display.setBrightness(7);
    display.showNumberDec(countdown);
    Serial.begin(115200);
}


void handleButton() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (millis() - buttonPressTime > debounceTime) {
            displayOn = !displayOn;
            buttonPressTime = millis();
        }
    }
}


void handleTrafficLight() {
    unsigned long currentMillis = millis();

    int ldrValue = analogRead(LDR_PIN);
    float ldrLux = map(ldrValue, 0, 4095, 0, 1000);
  
    Serial.print("LDR Lux: ");
    Serial.println(ldrLux);


    if (ldrLux < 50) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, LOW);

        if (currentMillis - lastBlinkMillis >= blinkDelay) {
            lastBlinkMillis = currentMillis;
            yellowBlink = !yellowBlink;
            digitalWrite(LED_YELLOW, yellowBlink);
        }

        display.clear(); 
        return;
    }

    
    if (currentMillis - lastMillis >= 1000) {
        lastMillis = currentMillis;
        countdown--;

        if (countdown < 0) {
            switch (currentState) {
                case GREEN:
                    currentState = YELLOW;
                    countdown = 3;
                    break;
                case YELLOW:
                    currentState = RED;
                    countdown = 10;
                    break;
                case RED:
                    currentState = GREEN;
                    countdown = 10;
                    break;
            }
        }

        if (displayOn) {
            display.showNumberDec(countdown);
        } else {
            display.clear();
        }
    }

    // Điều khiển đèn theo trạng thái
    if (currentState == GREEN) {
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);
    } else if (currentState == YELLOW) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(LED_RED, LOW);
    } else if (currentState == RED) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, HIGH);
    }
}

void loop() {
    handleButton();
    handleTrafficLight();
}
