#include <Arduino.h>

// put function declarations here:
int ledPin = 5;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH);
  Serial.println("LED ON");
  delay(1000);
  digitalWrite(ledPin, LOW);
  Serial.println("LED OFF");
  delay(1000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}