#include <Arduino.h>

const int ledPin = 17; // Chân kết nối LED

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT); // Thiết lập chân LED là OUTPUT
}

void loop() {
  Serial.println("LED ON");
  digitalWrite(ledPin, HIGH); // Bật LED
  delay(500); // Chờ 500ms
  Serial.println("LED OFF");
  digitalWrite(ledPin, LOW); // Tắt LED
  delay(500); // Chờ 500ms
  // a 
}