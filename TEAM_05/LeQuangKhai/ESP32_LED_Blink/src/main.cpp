#include <Arduino.h>

#define LED_PIN 5  // Chân kết nối với LED (có sẵn trên Arduino Uno)

void setup() {
  pinMode(LED_PIN, OUTPUT);  // Thiết lập chân LED là OUTPUT
  Serial.begin(115200);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);  // Bật LED
  Serial.println("LED ON");
  delay(1000);                  // Chờ 1 giây
  digitalWrite(LED_PIN, LOW);   // Tắt LED
  Serial.println("LED OFF");
  delay(1000);                  // Chờ 1 giây
}
