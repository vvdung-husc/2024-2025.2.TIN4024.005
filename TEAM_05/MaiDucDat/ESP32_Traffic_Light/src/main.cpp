#include <Arduino.h>

const int redPin = 17;
const int yellowPin = 16;
const int greenPin = 4;

unsigned long timeStart = 0;
int state = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("IOT \n Traffic Light \n MDDat");
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  digitalWrite(redPin, HIGH);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
  timeStart = millis();
}

void loop() {
  unsigned long now = millis();

  if (state == 0 && now - timeStart >= 5000) { // Đỏ -> Vàng
    digitalWrite(redPin, LOW);
    digitalWrite(yellowPin, HIGH);
    state = 1;
    timeStart = now;
    Serial.println("RED OFF, YELLOW ON");
  } 
  else if (state == 1 && now - timeStart >= 2000) { // Vàng -> Xanh
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, HIGH);
    state = 2;
    timeStart = now;
    Serial.println("YELLOW OFF, GREEN ON");
  } 
  else if (state == 2 && now - timeStart >= 5000) { // Xanh -> Đỏ
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    state = 0;
    timeStart = now;
    Serial.println("GREEN OFF, RED ON");
  }
}
