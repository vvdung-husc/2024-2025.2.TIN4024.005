#include <Arduino.h>
#include <TM1637Display.h>

#define CLK 25
#define DIO 26

#define rPin 19
#define yPin 18
#define gPin 17

#define buttonPin 14

uint rTime = 7;
uint yTime = 2;
uint gTime = 5;

bool isOn = false;
ulong timeStart = 0;
bool lastButtonState = HIGH;

TM1637Display display(CLK, DIO);

/* keep the signal light on during a period of time displayed on seven-segment display
  `(int) s` the signal light's on time (second)
*/
void countdown(int s) {
  while (s >= 0) {
    display.showNumberDec(s, true, 2, 2);
    Serial.print(s);
    Serial.print("\r");
    delay(1000);
    s--;
  }
  Serial.print("\n");
}

/* check the period of time
return `true` when `iTimer` is great than `milisecond`
*/
bool isReady(ulong& iTimer, uint32_t milisecond) {
  ulong t = millis();
  if (t - iTimer < (ulong)milisecond) 
    return false;
  iTimer = t;
  return true;
}

ulong baseTime = 0;

void resetMillis() {
  baseTime = millis();
}

uint getAdjustedmillis() {
  return millis() - baseTime;
}


/*
  Turn on the signal light
  `(char) signal`:
    - `g` turn on the green light
    - `y` turn on the yellow light
    - `r` turn on the red light
*/
void turnOn(char signal) {
  if (signal == 'G') {
    digitalWrite(rPin, LOW);
    digitalWrite(yPin, LOW);
    digitalWrite(gPin, HIGH);
    Serial.println("Green:");
  }
  else if (signal == 'Y') {
    digitalWrite(rPin, LOW);
    digitalWrite(yPin, HIGH);
    digitalWrite(gPin, LOW);
    Serial.println("Yellow:");
  }
  else if (signal == 'R') {
    digitalWrite(rPin, HIGH);
    digitalWrite(yPin, LOW);
    digitalWrite(gPin, LOW);
    Serial.println("Red:");
  }
}

//Traffic light without a seven-segment display
void TrafficLight() {
  char signal[3] = {'G', 'Y', 'R'};
  for (int i = 0; i < 3; i++) {
    turnOn(signal[i]);
    if (signal[i] == 'G') delay(gTime*1000);
    else if (signal[i] == 'Y') delay(yTime*1000);
    else delay(rTime*1000);
  }
}

//Traffic light with seven-segment display to show the remaining time of the signal light
void TrafficLight_ver2() {
  char signal[3] = {'G', 'Y', 'R'};
  for (int i = 0; i < 3; i++) {
    turnOn(signal[i]);
    if (signal[i] == 'G') countdown(gTime);
    else if (signal[i] == 'Y') countdown(yTime);
    else countdown(rTime);
  }
}

// Traffic light with code that dosen't use `delay()` function
void TrafficLight_ver3() {
  char signal[3] = {'G', 'Y', 'R'};
  uint period[3] = {gTime*1000, yTime*1000, rTime*1000};

  for (int i = 0; i < 3; i++) {
    turnOn(signal[i]);

    while (getAdjustedmillis() <= period[i]) {
      if (getAdjustedmillis() % 1000 == 0) {
        // ulong second = (int)millis()/1000;
        // ulong cd = (period[i]/1000) - (second % (period[i]/1000+1));
        // display.showNumberDec(cd, true, 2,2);
        Serial.print(getAdjustedmillis());
        Serial.print("\r");
        }
      }
    
    resetMillis();
  }
}

// Traffic light version 3 with seven-segment display
void TrafficLight_ver4() {
  char signal[3] = {'G', 'Y', 'R'};
  uint period[3] = {gTime*1000, yTime*1000, rTime*1000};

  for (int i = 0; i < 3; i++) {
    turnOn(signal[i]);

    while (getAdjustedmillis() <= period[i]) {
      if (getAdjustedmillis() % 1000 == 0) {
        uint cd = (period[i] - getAdjustedmillis())/1000;
        display.showNumberDec(cd, true, 2,2);
        Serial.print(getAdjustedmillis());
        Serial.print("\r");
        }
      }
    
    resetMillis();
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(rPin, OUTPUT);
  pinMode(yPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); //default state of the button is HIGH
  display.setBrightness(7);
}

void loop() {
  //TrafficLight();
  //TrafficLight_ver2();
  //TrafficLight_ver3();
  //TrafficLight_ver4();
  bool buttonState = digitalRead(buttonPin);
  Serial.print("The last status of the button is: ");
  Serial.println(lastButtonState == 1 ? "HIGH" : "LOW");

  if (buttonState == HIGH && lastButtonState == LOW) {
    delay(50);
    buttonState = LOW;
    display.setBrightness(0);
  }
  if (buttonState == LOW && lastButtonState == LOW) {
    delay(50);
    buttonState = HIGH;
    display.setBrightness(7);
  }
  if (buttonState == LOW && lastButtonState == HIGH) {
    delay(50);
    buttonState = LOW;
    display.setBrightness(0);
  }

  lastButtonState = buttonState;
  Serial.print("The status of the button is: ");
  Serial.println(buttonState == 1 ? "HIGH" : "LOW");

  buttonState == HIGH ? TrafficLight_ver4() : TrafficLight_ver3();
}

