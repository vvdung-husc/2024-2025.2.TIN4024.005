#include <Arduino.h>
#include <TM1637Display.h>

//Pin
#define rLED  12
#define yLED  14
#define gLED  27
#define bLED  18

#define CLK   5
#define DIO   21

//1000 ms = 1 seconds
#define rTIME  5000   //5 seconds
#define yTIME  3000
#define gTIME  5000

// Chân analog kết nối với LDR
#define LDR_PIN 13
// Chân kết nối với nút nhấn
#define BUTTON_PIN 22 

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
int currentLED = rLED;
int tmCounter = rTIME / 1000;
ulong counterTime = 0;

TM1637Display display(CLK, DIO);

bool ldrActivated = false;
bool systemActive = true; // Trạng thái hệ thống ban đầu là hoạt động
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light_TM1637();
void checkLDRAndSwitchToYellow();
void checkButtonAndToggleSystem();
void setup() {
  // put your setup code here, to run once:
  // Khởi tạo giao tiếp serial với tốc độ baud 115200
  Serial.begin(115200);
  // Cấu hình các chân đèn LED làm ngõ ra
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(bLED, OUTPUT);

// Cấu hình nút nhấn là ngõ vào với điện trở kéo lên
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  // Đặt trạng thái ban đầu của các đèn LED
  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);  // Bắt đầu với đèn LED đỏ bật
  digitalWrite(bLED, LOW); // Mặc định đèn xanh dương tắt

  currentLED = rLED;
  nextTimeTotal += rTIME;
  // Khởi tạo màn hình TM1637 với mức độ sáng 7 (độ sáng tối đa)
  display.setBrightness(7);
  
  // Hiển thị giá trị đếm ngược ban đầu trên màn hình
  tmCounter = (rTIME / 1000) - 1; // Thời gian đèn đỏ tính bằng giây trừ đi 1
  display.showNumberDec(tmCounter--, true, 2, 2);  
  
  // In thông báo khởi động lên màn hình serial
  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  "); 
  Serial.print(nextTimeTotal / 1000);
  Serial.println(" (ms)");
}
void loop() {
  // put your main code here, to run repeatedly:
  checkButtonAndToggleSystem(); // Kiểm tra trạng thái nút nhấn và thay đổi trạng thái hệ thống nếu cần

  if (systemActive) { // Chỉ chạy các hàm khi hệ thống đang hoạt động
    currentMiliseconds = millis();
    checkLDRAndSwitchToYellow();
    NonBlocking_Traffic_Light_TM1637();
  }
}
void checkButtonAndToggleSystem() {
  static bool buttonPressed = false; // Trạng thái của nút nhấn (đã nhấn chưa)
  int buttonState = digitalRead(BUTTON_PIN); // Đọc trạng thái nút nhấn

  if (buttonState == LOW && !buttonPressed) { // Nút nhấn được nhấn xuống
    buttonPressed = true; // Đánh dấu đã nhấn nút
    systemActive = !systemActive; // Thay đổi trạng thái hệ thống (bật/tắt)
    
    if (!systemActive) { // Nếu hệ thống bị tắt
      display.clear(); // Tắt màn hình LED
      digitalWrite(bLED, HIGH); // Bật đèn xanh dương
      Serial.println("Hệ thống đã tắt.");
    } else {
      digitalWrite(bLED, LOW);// Tắt đèn xanh dương
      Serial.println("Hệ thống đã bật.");
    }
  } else if (buttonState == HIGH) { // Nút nhấn được nhả ra
    buttonPressed = false; // Đặt lại trạng thái nút nhấn
  }
}
void checkLDRAndSwitchToYellow() {
  int ldrValue = analogRead(LDR_PIN); // Đọc giá trị từ LDR
  Serial.print("LDR Value: "); Serial.println(ldrValue);

  if (ldrValue < 500) { // Nếu ánh sáng yếu (giá trị LDR thấp)
    // Chuyển ngay sang đèn vàng
    digitalWrite(currentLED, LOW); // Tắt đèn hiện tại
    digitalWrite(yLED, HIGH); // Bật đèn vàng
    currentLED = yLED;
    tmCounter = (yTIME / 1000) - 1; // Đặt lại bộ đếm ngược cho đèn vàng
    nextTimeTotal = currentMiliseconds + yTIME;
    counterTime = currentMiliseconds;
    display.showNumberDec(tmCounter--, true, 2, 2);
    Serial.println("Trời tối, chuyển sang đèn vàng.");
  }
}
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}
// void NonBlocking_Traffic_Light(){
//   switch (currentLED) {
//     case rLED: // Đèn đỏ: 5 giây
//       if (IsReady(ledTimeStart, rTIME)) {
//         digitalWrite(rLED, LOW);
//         digitalWrite(gLED, HIGH);
//         currentLED = gLED;
//         nextTimeTotal += gTIME;
//         Serial.print("2. GREEN\t => Next "); Serial.println(nextTimeTotal);        
//       } 
//       break;

//     case gLED: // Đèn xanh: 7 giây
//       if (IsReady(ledTimeStart,gTIME)) {        
//         digitalWrite(gLED, LOW);
//         digitalWrite(yLED, HIGH);
//         currentLED = yLED;
//         nextTimeTotal += yTIME;
//         Serial.print("3. YELLOW\t => Next "); Serial.println(nextTimeTotal);        
//       }
//       break;

//     case yLED: // Đèn vàng: 2 giây
//       if (IsReady(ledTimeStart,yTIME)) {        
//         digitalWrite(yLED, LOW);
//         digitalWrite(rLED, HIGH);
//         currentLED = rLED;
//         nextTimeTotal += rTIME;
//         Serial.print("1. RED \t\t => Next "); Serial.println(nextTimeTotal);        
//       }
//       break;
//   }  
// }

void NonBlocking_Traffic_Light_TM1637(){
  bool bShowCounter = false;
  switch (currentLED) {
    case rLED: // Đèn đỏ: 5 giây
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        nextTimeTotal += gTIME;
        tmCounter = (gTIME / 1000) - 1 ; 
        bShowCounter = true;  
        counterTime = currentMiliseconds;        
        Serial.print("2. GREEN  => YELLOW "); Serial.print(nextTimeTotal/1000);Serial.println(" (ms)");       
      } 
      break;

    case gLED: // Đèn xanh: 7 giây
      if (IsReady(ledTimeStart,gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        tmCounter = (yTIME / 1000) - 1; 
        bShowCounter = true;   
        counterTime = currentMiliseconds;    
        Serial.print("3. YELLOW => RED    "); Serial.print(nextTimeTotal/1000);Serial.println(" (ms)");      
      }
      break;

    case yLED: // Đèn vàng: 2 giây
      if (IsReady(ledTimeStart,yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        tmCounter = (rTIME / 1000) - 1; 
        bShowCounter = true;       
        counterTime = currentMiliseconds;        
        Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal/1000);Serial.println(" (ms)");       
      }
      break;
  }
  if (!bShowCounter) bShowCounter = IsReady(counterTime, 1000);
  if (bShowCounter) {
    display.showNumberDec(tmCounter--, true, 2, 2);
  }
}
