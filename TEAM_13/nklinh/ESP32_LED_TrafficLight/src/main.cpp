#include <Arduino.h>
#include <TM1637Display.h>

#define rPin 19
#define yPin 18
#define gPin 17

#define rTime 7
#define yTime 2
#define gTime 5

#define CLK 25
#define DIO 26

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

}

void setup() {
  Serial.begin(115200);
  pinMode(rPin, OUTPUT);
  pinMode(yPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  display.setBrightness(7);
}

void loop() {
  //TrafficLight();
  TrafficLight_ver2();
}