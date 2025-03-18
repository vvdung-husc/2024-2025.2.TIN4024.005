#include <Arduino.h>

int gPin = 15;
int yPin = 4;
int rPin = 2;

int currenPin = 0;
int listPIN[] = {gPin,yPin, rPin };
void setup() {
  // put your setup code here, to run once:
  pinMode(gPin,OUTPUT);
  pinMode(yPin,OUTPUT);
  pinMode(rPin,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 digitalWrite(listPIN[currenPin], HIGH);
 Serial.println("LED ON");
 delay(1000);
 digitalWrite(listPIN[currenPin], LOW);
 Serial.println("LED OFF");
 delay(1000);
if(++currenPin > 2) currenPin = 0;
}

