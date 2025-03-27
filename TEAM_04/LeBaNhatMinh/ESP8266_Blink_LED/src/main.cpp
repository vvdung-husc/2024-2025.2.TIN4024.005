#include <Arduino.h>
#include <Wire.h>

#define LED1 15 // GPIO5 (D1)
#define LED2 2  // GPIO4 (D2)
#define LED3 5  // GPIO0 (D3)

void setup()
{
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
}

void loop()
{
  // Bật LED1, tắt các LED khác
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  delay(500);

  // Bật LED2, tắt các LED khác
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, LOW);
  delay(500);

  // Bật LED3, tắt các LED khác
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, HIGH);
  delay(500);

  // Tắt hết LED
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  delay(500);
}
