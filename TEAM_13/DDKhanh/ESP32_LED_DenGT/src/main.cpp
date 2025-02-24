#include <Arduino.h>
#include <TM1637Display.h>

// Khai báo chân kết nối đèn giao thông
int LedDo = 5;
int LedVang = 17;
int LedXanh = 16;
int CLK = 4;
int DIO = 2;
TM1637Display display(CLK, DIO);

int countRed = 5;
int countYellow = 2;
int countGreen = 7;

// Các biến cho thời gian và trạng thái
ulong ledStart = 0;
int LedState = 0;

// Lưu giá trị đếm trước đó để kiểm tra thay đổi
int lastNumber = -1;
int lastState = -1;

// Hàm kiểm tra thời gian không chặn (Non-Blocking)
bool IsReady(ulong& ulTimer, uint32_t millisecond){
  ulong t = millis();
  if(t - ulTimer < millisecond) return false;
  ulTimer = t;
  return true;
}

void UpdateDisplay(int number) {
  // Chỉ cập nhật nếu giá trị thay đổi
  if (lastNumber != number) {
    display.showNumberDec(number);
    lastNumber = number;
  }
}

void Use_Non_Blocking() {
  unsigned long currentMillis = millis();
  switch (LedState)
  {
    case 0: // Đèn Xanh
      digitalWrite(LedDo, LOW);
      digitalWrite(LedVang, LOW);
      digitalWrite(LedXanh, HIGH);
      UpdateDisplay(countGreen);
      if (IsReady(ledStart, 1000)) { 
        countGreen--;
        if (countGreen < 0) {
          LedState = 1;
          ledStart = currentMillis;
          countGreen = 7; // Reset lại đếm cho lần sau
          if (lastState != LedState) {
            Serial.println("ĐÈN XANH -> ĐÈN VÀNG");
            lastState = LedState;
          }
        }
      }
      break;
    
    case 1: // Đèn Vàng
      digitalWrite(LedDo, LOW);
      digitalWrite(LedVang, HIGH);
      digitalWrite(LedXanh, LOW);
      UpdateDisplay(countYellow);
      if (IsReady(ledStart, 1000)) { 
        countYellow--;
        if (countYellow < 0) {
          LedState = 2;
          ledStart = currentMillis;
          countYellow = 2; // Reset lại đếm cho lần sau
          if (lastState != LedState) {
            Serial.println("ĐÈN VÀNG -> ĐÈN ĐỎ");
            lastState = LedState;
          }
        }
      }
      break;

    case 2: // Đèn Đỏ
      digitalWrite(LedDo, HIGH);
      digitalWrite(LedVang, LOW);
      digitalWrite(LedXanh, LOW);
      UpdateDisplay(countRed);
      if (IsReady(ledStart, 1000)) { 
        countRed--;
        if (countRed < 0) {
          LedState = 0;
          ledStart = currentMillis;
          countRed = 5; // Reset lại đếm cho lần sau
          if (lastState != LedState) {
            Serial.println("ĐÈN ĐỎ -> ĐÈN XANH");
            lastState = LedState;
          }
        }
      }
      break;
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(LedDo, OUTPUT);
  pinMode(LedVang, OUTPUT);
  pinMode(LedXanh, OUTPUT);

  // Tắt tất cả các đèn khi bắt đầu
  digitalWrite(LedDo, LOW);
  digitalWrite(LedVang, LOW);
  digitalWrite(LedXanh, LOW);

  display.setBrightness(0x0f); // Độ sáng cao nhất

  // Bắt đầu từ LedState = 0 để hiện đèn Xanh trước
  LedState = 2;
  lastState = -1;
}


void loop() {
  Use_Non_Blocking();
}