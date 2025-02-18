#include <Arduino.h>

#define RED 5
#define YELLOW 4
#define GREEN 2

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
}

void loop() {
  // Bật đèn đỏ 5 giây
  digitalWrite(RED, HIGH);
  delay(5000);
  digitalWrite(RED, LOW);

  // Bật đèn xanh 5 giây
  digitalWrite(GREEN, HIGH);
  delay(5000);
  digitalWrite(GREEN, LOW);

  // Bật đèn vàng 2 giây
  digitalWrite(YELLOW, HIGH);
  delay(2000);
  digitalWrite(YELLOW, LOW);
}
