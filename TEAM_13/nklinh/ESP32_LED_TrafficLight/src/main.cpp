#include <Arduino.h>
#include <TM1637Display.h> // Thư viện điều khiển màn hình 7 đoạn TM1637

// Định nghĩa các chân GPIO kết nối với các đèn LED
#define LED_YELLOW 26  // Đèn vàng
#define LED_RED    27  // Đèn đỏ
#define LED_GREEN  25  // Đèn xanh lá

// Định nghĩa các chân GPIO kết nối với màn hình 7 đoạn TM1637
#define CLK_PIN 18 // Chân CLK của TM1637
#define DIO_PIN 19 // Chân DIO của TM1637

// Thời gian cho mỗi trạng thái (đơn vị: giây)
#define YELLOW_TIME 3  // Thời gian đèn vàng nháy
#define RED_TIME    5  // Thời gian đèn đỏ
#define GREEN_TIME  5  // Thời gian đèn xanh lá

// Khởi tạo đối tượng màn hình 7 đoạn
TM1637Display display(CLK_PIN, DIO_PIN);

// Biến lưu trạng thái hiện tại của đèn giao thông
enum TrafficLightState {
  YELLOW_BLINKING, // Đèn vàng nháy
  RED,             // Đèn đỏ
  GREEN            // Đèn xanh lá
};
TrafficLightState currentState = YELLOW_BLINKING;

// Biến lưu thời gian bắt đầu của trạng thái hiện tại
unsigned long stateStartTime = 0;

// Hàm thiết lập
void setup() {
  // Thiết lập các chân GPIO là OUTPUT
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Tắt tất cả các đèn LED ban đầu
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  // Khởi tạo màn hình 7 đoạn
  display.setBrightness(7); // Độ sáng tối đa (0-7)
  display.clear();          // Xóa màn hình

  // Bắt đầu với trạng thái đèn vàng nháy
  currentState = YELLOW_BLINKING;
  stateStartTime = millis();
}

// Hàm lặp chính
void loop() {
  unsigned long currentTime = millis(); // Lấy thời gian hiện tại
  unsigned long elapsedTime = (currentTime - stateStartTime) / 100; // Tính thời gian đã trôi qua (giây)

  switch (currentState) {
    case YELLOW_BLINKING:
      // Đèn vàng nháy mỗi giây
      if (elapsedTime % 2 == 0) {
        digitalWrite(LED_YELLOW, HIGH);
      } else {
        digitalWrite(LED_YELLOW, LOW);
      }

      // Hiển thị thời gian còn lại trên màn hình 7 đoạn
      display.showNumberDec(YELLOW_TIME - elapsedTime, false);

      // Chuyển sang trạng thái đèn đỏ sau YELLOW_TIME giây
      if (elapsedTime >= YELLOW_TIME) {
        digitalWrite(LED_YELLOW, LOW); // Tắt đèn vàng
        digitalWrite(LED_RED, HIGH);   // Bật đèn đỏ
        currentState = RED;
        stateStartTime = millis(); // Cập nhật thời gian bắt đầu trạng thái mới
      }
      break;

    case RED:
      // Đèn đỏ sáng liên tục
      // Hiển thị thời gian còn lại trên màn hình 7 đoạn
      display.showNumberDec(RED_TIME - elapsedTime, false);

      // Chuyển sang trạng thái đèn xanh lá sau RED_TIME giây
      if (elapsedTime >= RED_TIME) {
        digitalWrite(LED_RED, LOW);    // Tắt đèn đỏ
        digitalWrite(LED_GREEN, HIGH); // Bật đèn xanh lá
        currentState = GREEN;
        stateStartTime = millis(); // Cập nhật thời gian bắt đầu trạng thái mới
      }
      break;

    case GREEN:
      // Đèn xanh lá sáng liên tục
      // Hiển thị thời gian còn lại trên màn hình 7 đoạn
      display.showNumberDec(GREEN_TIME - elapsedTime, false);

      // Chuyển sang trạng thái đèn vàng nháy sau GREEN_TIME giây
      if (elapsedTime >= GREEN_TIME) {
        digitalWrite(LED_GREEN, LOW); // Tắt đèn xanh lá
        digitalWrite(LED_YELLOW, HIGH); // Bật đèn vàng
        currentState = YELLOW_BLINKING;
        stateStartTime = millis(); // Cập nhật thời gian bắt đầu trạng thái mới
      }
      break;
  }
}