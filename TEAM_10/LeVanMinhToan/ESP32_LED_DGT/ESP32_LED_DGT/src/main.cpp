#include <Arduino.h>
#include <TM1637Display.h>  // Thư viện điều khiển LED 7 đoạn

// Khai báo chân kết nối với TM1637
#define CLK 27   // Chân CLK của TM1637 nối với GPIO 27
#define DIO 14   // Chân DIO của TM1637 nối với GPIO 14
TM1637Display display(CLK, DIO); // Khởi tạo module TM1637

// Khai báo chân điều khiển đèn giao thông
#define LED_RED 19     // Đèn đỏ
#define LED_YELLOW 18  // Đèn vàng
#define LED_GREEN 5    // Đèn xanh

void setup() {
  // Cấu hình chân LED làm đầu ra
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Khởi tạo TM1637 với độ sáng tối đa
  display.setBrightness(7);
}

// Hàm điều khiển đèn và hiển thị đếm ngược trên LED 7 đoạn
void trafficLight(int ledPin, int duration) {
  digitalWrite(ledPin, HIGH); // Bật đèn
  for (int i = duration; i > 0; i--) {
    display.showNumberDec(i, false); // Hiển thị số giây đếm ngược
    delay(1000); // Dừng 1 giây
  }
  digitalWrite(ledPin, LOW); // Tắt đèn sau khi hết thời gian
}

void loop() {
  // Điều khiển đèn đỏ
  trafficLight(LED_RED, 20); // Bật đèn đỏ trong 5 giây

  // Điều khiển đèn vàng
  trafficLight(LED_YELLOW, 5); // Bật đèn vàng trong 2 giây

  // Điều khiển đèn xanh
  trafficLight(LED_GREEN, 20); // Bật đèn xanh trong 5 giây
}


