#include <Arduino.h>

const int LED_PIN = 4; // Chân điều khiển LED

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT); // Cấu hình chân LED là OUTPUT
}

void loop() {
  digitalWrite(LED_PIN, HIGH); // Bật LED
  Serial.println("LED ON");
  delay(1000); // Chờ 1 giây

  digitalWrite(LED_PIN, LOW); // Tắt LED
  Serial.println("LED OFF");
  delay(1000); // Chờ 1 giây
}

