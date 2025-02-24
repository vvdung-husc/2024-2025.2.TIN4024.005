#include <Wire.h>
#include <TM1637Display.h>

const int buttonPin = 23;        // Nút bấm kết nối với chân 23
const int led1Pin = 21;          // LED 1 (blue) kết nối với chân 21
const int led2Pin = 22;          // LED 2 (yellow) kết nối với chân 22
const int led3Pin = 19;          // LED 3 (red) kết nối với chân 19
const int led4Pin = 25;          // LED 4 (green) kết nối với chân 25

const int ldrPin = 13;           // Photoresistor (LDR) kết nối với chân 13

// Đối tượng hiển thị 7 đoạn (TM1637)
TM1637Display display(18, 17);   // CLK là 18, DIO là 17

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);  // Cài đặt nút bấm với chế độ INPUT_PULLUP
  pinMode(led1Pin, OUTPUT);          // Cài đặt LED 1 là OUTPUT
  pinMode(led2Pin, OUTPUT);          // Cài đặt LED 2 là OUTPUT
  pinMode(led3Pin, OUTPUT);          // Cài đặt LED 3 là OUTPUT
  pinMode(led4Pin, OUTPUT);          // Cài đặt LED 4 là OUTPUT

  pinMode(ldrPin, INPUT);            // Cài đặt Photoresistor là INPUT

  display.setBrightness(0x0f);       // Cài đặt độ sáng của màn hình 7 đoạn
  display.clear();                   // Xóa màn hình
}

void loop() {
  // Đọc giá trị từ nút bấm (LOW khi nhấn)
  int buttonState = digitalRead(buttonPin);

  // Đọc giá trị từ Photoresistor (LDR)
  int ldrValue = analogRead(ldrPin);

  // Hiển thị giá trị LDR trên màn hình 7 đoạn
  display.showNumberDec(ldrValue);

  // Điều khiển đèn LED theo giá trị LDR
  if (ldrValue < 1000) {
    // Nếu ánh sáng thấp, bật LED đỏ (led3)
    digitalWrite(led3Pin, HIGH);
    digitalWrite(led1Pin, LOW);
    digitalWrite(led2Pin, LOW);
    digitalWrite(led4Pin, LOW);
  } else if (ldrValue < 2000) {
    // Nếu ánh sáng trung bình, bật LED vàng (led2)
    digitalWrite(led3Pin, LOW);
    digitalWrite(led1Pin, LOW);
    digitalWrite(led2Pin, HIGH);
    digitalWrite(led4Pin, LOW);
  } else if (ldrValue < 3000) {
    // Nếu ánh sáng cao, bật LED xanh lá (led4)
    digitalWrite(led3Pin, LOW);
    digitalWrite(led1Pin, LOW);
    digitalWrite(led2Pin, LOW);
    digitalWrite(led4Pin, HIGH);
  } else {
    // Nếu quá sáng, bật LED xanh dương (led1)
    digitalWrite(led3Pin, LOW);
    digitalWrite(led1Pin, HIGH);
    digitalWrite(led2Pin, LOW);
    digitalWrite(led4Pin, LOW);
  }

  // Nếu nút bấm được nhấn (LOW)
  if (buttonState == LOW) {
    // Bật tất cả các LED
    digitalWrite(led1Pin, HIGH);
    digitalWrite(led2Pin, HIGH);
    digitalWrite(led3Pin, HIGH);
    digitalWrite(led4Pin, HIGH);
  } else {
    // Tắt tất cả các LED khi nút không được nhấn
    digitalWrite(led1Pin, LOW);
    digitalWrite(led2Pin, LOW);
    digitalWrite(led3Pin, LOW);
    digitalWrite(led4Pin, LOW);
  }

  delay(100); // Delay một chút để tránh chập chờn
}
