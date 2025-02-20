#include <Arduino.h>

// put function declarations here:
int ledPin = 25; //

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT); //
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ledPin, HIGH);
  Serial.println("LED ON"); //
  delay(1000); //
  Serial.println("LED OFF"); //
  digitalWrite(ledPin, LOW); //
  delay(1000); //
   
}
// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}