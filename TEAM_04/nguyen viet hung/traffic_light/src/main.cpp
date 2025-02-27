#include <TM1637Display.h>
#include <Arduino.h>
// Định nghĩa các chân
#define RED_LED 5
#define YELLOW_LED 16
#define GREEN_LED 4
#define CLK 23
#define DIO 22

// Khai báo màn hình 7 đoạn
TM1637Display display(CLK, DIO);

// Cấu hình thời gian cho mỗi đèn (tính bằng mili giây)
const unsigned long RED_TIME = 5000;
const unsigned long YELLOW_TIME = 2000;
const unsigned long GREEN_TIME = 5000;

// Biến lưu trữ trạng thái và thời gian
unsigned long previousMillis = 0;
unsigned long interval = RED_TIME;
int state = 0; // 0 - Đỏ, 1 - Xanh lá, 2 - Vàng
int lastDisplayedTime = -1; // Thời gian đã hiển thị lần trước

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  display.setBrightness(0x0f); // Độ sáng tối đa
  digitalWrite(RED_LED, HIGH); // Bắt đầu với đèn đỏ bật
}

// Hàm cập nhật hiển thị thời gian thực
void updateDisplay(unsigned long timeLeft) {
  int secondsLeft = (timeLeft + 999) / 1000; // Làm tròn chính xác giây còn lại
  if (secondsLeft != lastDisplayedTime) {
    display.showNumberDec(secondsLeft, true);
    lastDisplayedTime = secondsLeft;
  }
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long timeElapsed = currentMillis - previousMillis;
  unsigned long timeLeft = interval - timeElapsed;
  
  updateDisplay(timeLeft);

  if (timeElapsed >= interval) {
    previousMillis = currentMillis;
    lastDisplayedTime = -1; // Reset hiển thị để cập nhật ngay lập tức
    switch (state) {
      case 0: // Chuyển từ Đỏ -> Xanh lá
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        interval = GREEN_TIME;
        state = 1;
        break;
      case 1: // Chuyển từ Xanh lá -> Vàng
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);
        interval = YELLOW_TIME;
        state = 2;
        break;
      case 2: // Chuyển từ Vàng -> Đỏ
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        interval = RED_TIME;
        state = 0;
        break;
    }
  }
}
