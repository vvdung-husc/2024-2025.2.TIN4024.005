#include <Arduino.h>

#define LED_PIN 5  // Chân kết nối đèn LED

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);  // Thiết lập chân LED là đầu ra
 
}

void loop() {
  digitalWrite(LED_PIN, HIGH); // Bật đèn LED
  Serial.println("LED ON");
  delay(1000);                 // Đợi 1 giây
  digitalWrite(LED_PIN, LOW);  // Tắt đèn LED
  Serial.println("LED OFF");
  delay(1000);                 // Đợi 1 giây
}
