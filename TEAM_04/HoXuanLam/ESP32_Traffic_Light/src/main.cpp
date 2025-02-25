#include <Arduino.h>
#include <TM1637Display.h>

#define RED_LED 5
#define YELLOW_LED 16
#define GREEN_LED 4

const int CLK = 23;
const int DIO = 22;

ulong previousMillis = 0;
int state = 0;
int countdown = 0; // Biến đếm ngược
TM1637Display display(CLK, DIO);

void setup()
{
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  display.setBrightness(7);

  // Bắt đầu với đèn đỏ
  digitalWrite(RED_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  countdown = 5; // Đèn đỏ ban đầu sáng trong 5 giây
  display.showNumberDec(countdown, true);
}

void loop()
{
  ulong currentMillis = millis();

  if (currentMillis - previousMillis >= 1000)
  {
    previousMillis = currentMillis;

    // Giảm thời gian đếm ngược
    if (countdown > 0)
    {
      Serial.print("Thời gian còn lại: ");
      Serial.print(countdown);
      Serial.println(" giây");
      display.showNumberDec(countdown, true);
      if (countdown > 0)
      {
        countdown--;
      }
    }
  }

  if (state == 0 && countdown == 0)
  { // Chuyển sang đèn xanh
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    state = 1;
    countdown = 5;
    previousMillis = currentMillis;
  }
  else if (state == 1 && countdown == 0)
  { // Chuyển sang đèn vàng
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    state = 2;
    countdown = 2;
    previousMillis = currentMillis;
  }
  else if (state == 2 && countdown == 0)
  { // Quay lại đèn đỏ
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    state = 0;
    countdown = 5;
    previousMillis = currentMillis;
  }
}
