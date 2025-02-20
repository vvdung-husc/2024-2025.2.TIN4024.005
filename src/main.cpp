#include <Arduino.h>
int yellow = 2;
int red = 5;
int green = 15;
void setup() {
  pinMode(yellow, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  Serial.begin(115200); // Bắt buộc để Serial hoạt động
}

void loop() {
  // Tắt tất cả các đèn trước khi bắt đầu
  digitalWrite(green, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(red, LOW);

  // Đèn xanh sáng 3 giây
  digitalWrite(green, HIGH);
  for (int i = 3; i > 0; i--) {
    Serial.print("Xanh còn: ");
    Serial.print(i);
    Serial.println(" giây");
    delay(1000);
  }
  digitalWrite(green, LOW);

  // Đèn vàng sáng 1 giây
  digitalWrite(yellow, HIGH);
  for (int i = 1; i > 0; i--) {
    Serial.print("Vàng còn: ");
    Serial.print(i);
    Serial.println(" giây");
    delay(1000);
  }
  digitalWrite(yellow, LOW);

  // Đèn đỏ sáng 2 giây
  digitalWrite(red, HIGH);
  for (int i = 2; i > 0; i--) {
    Serial.print("Đỏ còn: ");
    Serial.print(i);
    Serial.println(" giây");
    delay(1000);
  }
  digitalWrite(red, LOW);
}