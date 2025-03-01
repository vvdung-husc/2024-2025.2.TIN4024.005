#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân,,,
#define rLED 5
#define yLED 17
#define gLED 16
#define CLK 15
#define DIO 2
#define ldrPIN 13

// Thời gian sáng của đèn (ms)
#define rTIME 5000   // 5 giây
#define yTIME 3000   // 3 giây
#define gTIME 10000  // 10 giây

// Biến toàn cục
unsigned long currentMillis = 0;
unsigned long ledTimeStart = 0;
unsigned long counterTime = 0;
int currentLED = rLED;
int tmCounter = rTIME / 1000;
const int darkThreshold = 1000;
bool darkMode = false;

TM1637Display display(CLK, DIO);

bool isReady(unsigned long &timer, uint32_t interval);
void handleTrafficLight();
void handleTrafficLightTM1637();
bool isDark();
void blinkYellowLED();

void setup() {
  Serial.begin(115200);
  
  // Cấu hình chân đầu ra
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(ldrPIN, INPUT);
  
  // Khởi tạo màn hình TM1637
  display.setBrightness(7);
  display.showNumberDec(tmCounter, true, 2, 2);

  // Khởi động đèn đỏ
  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  
  Serial.println("== START ==> Hệ thống điều khiển đèn giao thông đã khởi động!");
}

void loop() {
  currentMillis = millis();

  if (isDark()) {
    blinkYellowLED();
  } else {
    handleTrafficLightTM1637();
  }
}

// Hàm kiểm tra khoảng thời gian có trôi qua không
bool isReady(unsigned long &timer, uint32_t interval) {
  if (currentMillis - timer < interval) return false;
  timer = currentMillis;
  return true;
}

// Điều khiển đèn giao thông và cập nhật màn hình TM1637
void handleTrafficLightTM1637() {
  bool updateDisplay = false;

  switch (currentLED) {
    case rLED:
      if (isReady(ledTimeStart, rTIME)) {
        Serial.println("==> Chuyển sang ĐÈN XANH");
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        tmCounter = gTIME / 1000;
        updateDisplay = true;
      }
      break;
      
    case gLED:
      if (isReady(ledTimeStart, gTIME)) {
        Serial.println("==> Chuyển sang ĐÈN VÀNG");
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        tmCounter = yTIME / 1000;
        updateDisplay = true;
      }
      break;
      
    case yLED:
      if (isReady(ledTimeStart, yTIME)) {
        Serial.println("==> Chuyển sang ĐÈN ĐỎ");
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        tmCounter = rTIME / 1000;
        updateDisplay = true;
      }
      break;
  }

  // Giảm bộ đếm giây và hiển thị trên TM1637
  if (isReady(counterTime, 1000)) {
    if (tmCounter > 0) tmCounter--;
    display.showNumberDec(tmCounter, true, 2, 2);
  }
}

// Kiểm tra điều kiện trời tối dựa trên cảm biến ánh sáng
bool isDark() {
  static unsigned long lastCheck = 0;

  if (!isReady(lastCheck, 100)) return darkMode; // Giảm tần suất đọc cảm biến

  int value = analogRead(ldrPIN);
  
  if (value < darkThreshold && !darkMode) {
    Serial.println("==> PHÁT HIỆN TRỜI TỐI! Chuyển sang chế độ nhấp nháy.");
    darkMode = true;
  } 
  else if (value >= darkThreshold && darkMode) {
    Serial.println("==> PHÁT HIỆN TRỜI SÁNG! Quay lại chế độ đèn giao thông.");
    darkMode = false;
  }

  return darkMode;
}

// Chế độ nhấp nháy đèn vàng khi trời tối
void blinkYellowLED() {
  static unsigned long blinkStart = 0;
  static bool isOn = false;

  if (isReady(blinkStart, 1000)) {
    isOn = !isOn;
    digitalWrite(yLED, isOn ? HIGH : LOW);
    Serial.print("==> Đèn vàng ");
    Serial.println(isOn ? "BẬT" : "TẮT");
  }
}
