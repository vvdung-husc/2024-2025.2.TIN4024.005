#include <Arduino.h>

int ledPin = 5;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ledPin,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 digitalWrite(ledPin, HIGH);
 Serial.println("LED ON");
 delay(1000);
 digitalWrite(ledPin, LOW);
 Serial.println("LED OFF");
 delay(1000);

}

