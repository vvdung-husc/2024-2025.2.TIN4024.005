#include <Arduino.h>

int ledPin = 5;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH); // Bật LED
  Serial.println("LED ON");
  delay(1000); // Đợi 1 giây
  digitalWrite(ledPin, LOW);  // Tắt LED
  Serial.println("LED OFF");
  delay(1000); // Đợi 1 giây

}
