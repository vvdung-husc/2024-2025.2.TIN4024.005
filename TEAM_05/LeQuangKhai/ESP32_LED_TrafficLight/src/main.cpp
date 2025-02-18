#include <Arduino.h>

#define LED_GREEN 17   // Đèn xanh
#define LED_YELLOW 5   // Đèn vàng
#define LED_RED 4      // Đèn đỏ

// Biến thời gian & trạng thái
ulong timeStart = 0;
int state = 0;  
const uint32_t intervals[] = {6000, 2000, 5000}; // Thời gian: Xanh, Vàng, Đỏ

// Kiểm tra xem đã đến lúc thay đổi trạng thái chưa
bool IsReady(ulong& iTimer, uint32_t milisecond) {
  ulong t = millis();
  if (t - iTimer < milisecond) return false;
  iTimer = t;
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

void loop() {
  if (IsReady(timeStart, intervals[state])) {
    state = (state + 1) % 3; // Chuyển trạng thái: 0 -> 1 -> 2 -> 0

    // Điều khiển đèn theo trạng thái
    switch (state) {
      case 0: // Đèn xanh sáng
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);
        Serial.println("Đèn Xanh Bật (Xe đi)");
        break;
        
      case 1: // Đèn vàng sáng
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(LED_RED, LOW);
        Serial.println("Đèn Vàng Bật (Chuẩn bị dừng)");
        break;
        
      case 2: // Đèn đỏ sáng
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, HIGH);
        Serial.println("Đèn Đỏ Bật (Dừng lại)");
        break;
    }
  }
}
