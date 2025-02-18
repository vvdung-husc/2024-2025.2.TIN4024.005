#include <Arduino.h>

int led = 5;

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
}

void loop() {
  digitalWrite(led, HIGH);  // Bật LED
  Serial.println("LED ON");
  delay(1000);              // Chờ 1 giây
  digitalWrite(led, LOW);   // Tắt LED
  Serial.println("LED OFF");
  delay(1000);              // Chờ 1 giây
}