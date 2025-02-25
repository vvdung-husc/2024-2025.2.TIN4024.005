#include <Arduino.h>

// Chân kết nối LED
const int ledXanh = 5;
const int ledVang = 18;
const int ledDo = 19;

// Các trạng thái đèn
enum TrafficState { GREEN, YELLOW, RED };
TrafficState currentState = GREEN;

// Biến dùng cho Non-blocking
ulong timeStart = 0;
ulong timeDelay = 10000;  // Mặc định 10s cho đèn xanh

// Hàm kiểm tra thời gian non-blocking
bool IsReady(ulong &iTimer, uint32_t milisecond) {
  ulong t = millis();
  if (t - iTimer < milisecond) return false;
  iTimer = t;
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(ledXanh, OUTPUT);
  pinMode(ledVang, OUTPUT);
  pinMode(ledDo, OUTPUT);

  // Bật đèn xanh đầu tiên
  digitalWrite(ledXanh, HIGH);
  digitalWrite(ledVang, LOW);
  digitalWrite(ledDo, LOW);
}
void loop() {
  static ulong lastPrintTime = 0;  // Biến lưu thời gian in lần trước
  static int countdown = timeDelay / 1000;  // Biến đếm ngược

  if (millis() - lastPrintTime >= 1000) {  // Cứ mỗi giây in ra
    Serial.print("Thời gian còn lại: ");
    Serial.print(countdown);
    Serial.println(" giây");
    lastPrintTime = millis();
    countdown--;
  }

  if (IsReady(timeStart, timeDelay)) {
    switch (currentState) {
      case GREEN:
        Serial.println("Chuyển sang đèn vàng");
        digitalWrite(ledXanh, LOW);
        digitalWrite(ledVang, HIGH);
        timeDelay = 3000;  // Đèn vàng sáng 3s
        currentState = YELLOW;
        break;

      case YELLOW:
        Serial.println("Chuyển sang đèn đỏ");
        digitalWrite(ledVang, LOW);
        digitalWrite(ledDo, HIGH);
        timeDelay = 10000;  // Đèn đỏ sáng 10s
        currentState = RED;
        break;

      case RED:
        Serial.println("Chuyển sang đèn xanh");
        digitalWrite(ledDo, LOW);
        digitalWrite(ledXanh, HIGH);
        timeDelay = 10000;  // Đèn xanh sáng 10s
        currentState = GREEN;
        break;
    }

    countdown = timeDelay / 1000;  // Reset bộ đếm ngược
  }
}

// void loop() {
//   if (IsReady(timeStart, timeDelay)) {
//     switch (currentState) {
//       case GREEN:
//         Serial.println("Chuyển sang đèn vàng");
//         digitalWrite(ledXanh, LOW);
//         digitalWrite(ledVang, HIGH);
//         timeDelay = 3000;  // Đèn vàng sáng 3s
//         currentState = YELLOW;
//         break;

//       case YELLOW:
//         Serial.println("Chuyển sang đèn đỏ");
//         digitalWrite(ledVang, LOW);
//         digitalWrite(ledDo, HIGH);
//         timeDelay = 10000;  // Đèn đỏ sáng 10s
//         currentState = RED;
//         break;

//       case RED:
//         Serial.println("Chuyển sang đèn xanh");
//         digitalWrite(ledDo, LOW);
//         digitalWrite(ledXanh, HIGH);
//         timeDelay = 10000;  // Đèn xanh sáng 10s
//         currentState = GREEN;
//         break;
//     }
//   }
// }
