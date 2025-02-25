#include <Arduino.h>

// Khai báo chân kết nối LED
const int RED_LED = 4;
const int YELLOW_LED = 5;
const int GREEN_LED = 18;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Traffic Light Simulation");
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
}

void loop() {
  // Bật đèn xanh, tắt các đèn khác
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  delay(5000); // Đèn xanh sáng trong 5 giây

  // Bật đèn vàng, tắt các đèn khác
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  delay(2000); // Đèn vàng sáng trong 2 giây

  // Bật đèn đỏ, tắt các đèn khác
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(5000); // Đèn đỏ sáng trong 5 giây
}