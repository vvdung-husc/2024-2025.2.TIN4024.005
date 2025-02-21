#include <Arduino.h>
#include <TM1637Display.h>

// Khai báo chân GPIO cho LED
#define LED_GREEN 2
#define LED_YELLOW 4
#define LED_RED 17

// Khai báo chân cho nút bấm
#define BUTTON_PIN 13
  
// Khai báo chân TM1637
#define CLK 25
#define DIO 26  

TM1637Display display(CLK, DIO);

bool showCountdown = false;  // Trạng thái hiển thị số giây
unsigned long previousMillis = 0;
int currentCountdown = 0;
int currentLight = 0;

// Thời gian sáng của từng đèn
int lightDurations[] = {3, 1, 2};
int lightPins[] = {LED_GREEN, LED_YELLOW, LED_RED};

void IRAM_ATTR handleButtonPress() {
  showCountdown = !showCountdown;  // Đảo trạng thái hiển thị
}

void setup() {
  // Khởi tạo LED
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  // Khởi tạo nút bấm với PULLUP
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

  // Khởi tạo Serial
  Serial.begin(115200);
  
  // Khởi tạo TM1637
  display.setBrightness(7);
  display.showNumberDec(0, false);
}

void loop() {
  unsigned long currentMillis = millis();

  // Nếu đã đủ 1 giây, cập nhật bộ đếm
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;

    if (currentCountdown == 0) {
      // Chuyển sang đèn tiếp theo
      currentLight = (currentLight + 1) % 3;
      currentCountdown = lightDurations[currentLight];

      // Tắt tất cả đèn
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, LOW);

      // Bật đèn tương ứng
      digitalWrite(lightPins[currentLight], HIGH);
    } else {
      currentCountdown--; // Giảm bộ đếm
    }

    // Hiển thị số giây nếu bật hiển thị
    if (showCountdown) {
      display.showNumberDec(currentCountdown, false);
    } else {
      display.clear();
    }

    Serial.print("Đèn còn: ");
    Serial.print(currentCountdown);
    Serial.println(" giây");
  }
}
