#include <Arduino.h>

// // put function declarations here:
// int myFunction(int, int);

// void setup() {
//   // put your setup code here, to run once:
//   int result = myFunction(2, 3);
//   Serial.print("Ket qua");
//   Serial.println(result);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   delay(1000);
//   unsigned long m = millis();
//   Serial.print("TimerL: ");
//   Serial.print(m);
// }

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }

const int ledPin = 5; 


void setup() {
    
    pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH);
  Serial.print("LED ON");
  delay(1000);
  digitalWrite(ledPin, LOW);
  Serial.print("LED OFF");
  delay(1000);
}