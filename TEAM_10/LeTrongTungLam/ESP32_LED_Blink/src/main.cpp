#include <Arduino.h>

int  ledPin = 5;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
 pinMode(ledPin,OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH); //bật đèn
  Serial.println("LEB ON");
  delay(1000); // 1s
  digitalWrite(ledPin, LOW); // tắt đèn
  Serial.println("LEB OFF");
  delay(1000);
}

