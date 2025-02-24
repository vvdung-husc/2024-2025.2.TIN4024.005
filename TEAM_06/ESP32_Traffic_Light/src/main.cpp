#include <Arduino.h>
#include <TM1637Display.h>

//Định nghĩa các chân kết nối
#define BTUTTON_PIN 23
#define LED_RED 27
#define LED_YELLOW 26
#define LED_BLUE 25
#define LED_PURPLE 21

#define LDR_PIN 13

#define CLK 18
#define DIO 19

//TML1637 4-wire interface
TM1637Display display(CLK,DIO);

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}