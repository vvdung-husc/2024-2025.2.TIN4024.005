#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân kết nối
#define BUTTON_PIN 23
#define LED_BLUE 21
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27
#define LDR_PIN 13
#define CLK 18
#define DIO 19

// Khởi tạo đối tượng hiển thị TM1637
TM1637Display display(CLK, DIO);

// Biến toàn cục
int buttonState = 0;
int lastButtonState = HIGH;
bool displayOn = true;
int ldrValue = 0;
int countdown = 10;
unsigned long previousMillis = 0;
const long interval = 1000;
unsigned long blinkMillis = 0;
const long blinkInterval = 500;
const long debounceDelay = 50;
unsigned long lastDebounceTime = 0;
bool yellowBlinkState = false;

enum TrafficLightState { GREEN, YELLOW, RED };
TrafficLightState lightState = GREEN;

void setupPins() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    display.setBrightness(0x0F);
    display.showNumberDec(countdown);
    Serial.begin(115200);
}

void readLDR() {
    ldrValue = analogRead(LDR_PIN);
    Serial.print("LUX Value: ");
    Serial.println(ldrValue);
}
void checkButtonPress() {
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
        displayOn = !displayOn;
        if (!displayOn) display.clear();
        lastDebounceTime = millis();
    }
    lastButtonState = buttonState;
}
void updateCountdown() {
    if (millis() - previousMillis >= interval) {
        previousMillis = millis();
        countdown--;
        if (countdown < 0) {
            switch (lightState) {
                case GREEN: lightState = YELLOW; countdown = 3; break;
                case YELLOW: lightState = RED; countdown = 10; break;
                case RED: lightState = GREEN; countdown = 10; break;
            }
        }
        if (displayOn) display.showNumberDec(countdown);
    }
}
void handleTrafficLights() {
    if (ldrValue < 1000) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, LOW);
        if (millis() - blinkMillis >= blinkInterval) {
            blinkMillis = millis();
            yellowBlinkState = !yellowBlinkState;
            digitalWrite(LED_YELLOW, yellowBlinkState);
        }
        if (displayOn) display.clear();
    } else {
        digitalWrite(LED_GREEN, lightState == GREEN);
        digitalWrite(LED_YELLOW, lightState == YELLOW);
        digitalWrite(LED_RED, lightState == RED);
    }
}
void setup() {
    setupPins();
}

void loop() {
    readLDR();
    checkButtonPress();
    updateCountdown();
    handleTrafficLights();
}
