#include <Arduino.h>

const int ledGreen = 5;   // Chân nối đèn xanh
const int ledYellow = 18; // Chân nối đèn vàng
const int ledRed = 19;    // Chân nối đèn đỏ

int state = 0; // 0: Green, 1: Yellow, 2: Red
ulong timeStart = 0;  

bool IsReady(ulong& iTimer, uint32_t milisecond) {
  ulong t = millis();
  if (t - iTimer < milisecond) return false;
  iTimer = t;
  return true;
}

void Use_Non_Blocking() {
  if (state == 0 && IsReady(timeStart, 5000)) { 
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledYellow, HIGH);
    Serial.println("Green OFF, Yellow ON");
    state = 1;
  } 
  else if (state == 1 && IsReady(timeStart, 2000)) { 
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledRed, HIGH);
    Serial.println("Yellow OFF, Red ON");
    state = 2;
  } 
  else if (state == 2 && IsReady(timeStart, 5000)) {
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
    Serial.println("Red OFF, Green ON");
    state = 0;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  
  digitalWrite(ledGreen, HIGH);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledRed, LOW);
  timeStart = millis();
}

void loop() {
  Use_Non_Blocking();
}
