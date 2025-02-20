#include <Arduino.h>

const int ledPin = 5; // Chân LED tích hợp trên ESP32

void setup() {
  Serial.begin(115200);
  Serial.println("IOT \n LED Blink \n MDDat");
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH); // Bật đèn
  Serial.println("LED ON");
  delay(1000);

  digitalWrite(ledPin, LOW); // Tắt đèn
  Serial.println("LED OFF");
  delay(1000);
}
