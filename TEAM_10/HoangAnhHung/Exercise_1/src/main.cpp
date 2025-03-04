#include <Arduino.h>
#include <TM1637Display.h>

// LED
const uint rLED = 12;
const uint yLED = 14;
const uint gLED = 27;

const uint bLED = 18;

//7-segment display
const uint CLK = 5;
const uint DIO = 21;

//button & light sensor
const uint LDR = 13;
const uint button = 22;

const ulong rTime = 3000;
const ulong yTime = 3000;
const ulong gTime = 3000;

bool isNightMode = true;
ulong baseTime = 0;
int cd = 0;
bool lastButtonState = LOW;
bool toggleButtonState = true;
bool isNoticed_1 = false; 
bool isNoticed_2 = false;

TM1637Display display(CLK, DIO);

//Return `True` if the state of the button is on
bool checkButton() {
    static unsigned long lastDebounceTime = 0; 
    const unsigned long debounceDelay = 100;   

    bool buttonState = digitalRead(button);

    if (buttonState == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
        toggleButtonState = !toggleButtonState;
        toggleButtonState ? Serial.println("7-Segment Display is on") : Serial.println("7-Segment Display is off");
        digitalWrite(bLED, toggleButtonState);
        lastDebounceTime = millis();
    }
    lastButtonState = buttonState;

    return toggleButtonState;
}

/*
  Turn on the signal light
  `(char) signal`:
    - `G` turn on the green light
    - `R` turn on the yellow light
    - `B` turn on the red light
*/
void turnOn(char signal) {
    if (signal == 'R') {
    digitalWrite(yLED, LOW);
    digitalWrite(gLED, LOW);
    digitalWrite(rLED, HIGH);  
    //Serial.print("RED is on. Remaining time: )");
    }
    else if (signal == 'Y') {
    digitalWrite(yLED, HIGH);
    digitalWrite(gLED, LOW);
    digitalWrite(rLED, LOW);  
    //Serial.println("YELLOW is on. Remaining time: )"); 
    }
    else if (signal == 'G') {
    digitalWrite(yLED, LOW);
    digitalWrite(gLED, HIGH);
    digitalWrite(rLED, LOW);  
    //Serial.println("GREEN is on. Remaining time: )");       
    }
}

void resetMillis() {
    baseTime = millis();
}

uint getAdjustedMillis() {
    if (!isNightMode)
        checkButton();
    return millis() - baseTime;
}

void showNumberIn7SD(uint cd) {
    cd < 0 ? cd = +0 : cd;
    display.showNumberDec(cd, true, 2,2);
}

// Turn of the 7-segment display
void clear7SD() {
    display.clear();
}

void TrafficLight() {
    char signals[3] = {'R', 'G', 'Y'};
    ulong periods[3] = {rTime, gTime, yTime};

    for (int i = 0; i < 3; i++) {
        turnOn(signals[i]);
        toggleButtonState ? display.showNumberDec(uint(periods[i]/1000)-1, true, 2,2) : clear7SD();

        while (getAdjustedMillis() <= periods[i] and int(periods[i] - getAdjustedMillis()) > 0) {
            if (getAdjustedMillis() % 1000 == 0) {
                cd = int((periods[i] - getAdjustedMillis())/1000)-1;
                toggleButtonState ? display.showNumberDec(cd, true, 2,2) : clear7SD();
                //Serial.println(int(periods[i] - getAdjustedMillis())/1000);
            }
        }
        resetMillis();
    }
}

void NightMode() {
    clear7SD();
    digitalWrite(bLED, LOW);
    turnOn('Y');
    resetMillis();
    while (getAdjustedMillis() < 1000) 
        true;

    digitalWrite(yLED, LOW);
    resetMillis();
    while (getAdjustedMillis() < 1000) 
        true;
}

void setup() {
    Serial.begin(115200);

    pinMode(rLED, OUTPUT);
    pinMode(yLED, OUTPUT);
    pinMode(gLED, OUTPUT);
    pinMode(bLED, OUTPUT);

    pinMode(button, INPUT_PULLUP); 

    digitalWrite(yLED, LOW);
    digitalWrite(gLED, LOW);
    digitalWrite(rLED, HIGH);  
    digitalWrite(bLED, HIGH);

    display.setBrightness(7);
}

void loop() {
    int sensorValue = analogRead(LDR);
    if (sensorValue < 1000) {
        if (!isNoticed_1) {
            Serial.print("Night mode is on (");
            Serial.print(sensorValue);
            Serial.println(")");
            isNoticed_1 = true;
            isNoticed_2 = false;
        }
        isNightMode = true;

        NightMode();
    }
    else {
        isNightMode = false;  
        if (!isNoticed_2) {
            Serial.print("Night mode is off (");
            Serial.print(sensorValue);
            Serial.println(")");
            isNoticed_2 = true;
            isNoticed_1 = false;
        }
        isNightMode = false;
        
        TrafficLight();
    }

    //Serial.println("Test night mode");
    //NightMode();
    //TrafficLight();
    //Serial.println(millis());
    //delay(100);
}