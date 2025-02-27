#include <Arduino.h>

int led = 5;

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
}

void loop() {
/**
 * @brief Set up the initial state of the microcontroller and peripherals.
 *
 * This function initializes the serial communication at a baud rate of 115200 and sets the pin connected to the LED as an output.
 *
 * @return void
 */
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
}
  digitalWrite(led, HIGH);  // Bật LED
  Serial.println("LED ON");
  delay(1000);              // Chờ 1 giây
  digitalWrite(led, LOW);   // Tắt LED
  Serial.println("LED OFF");
  delay(1000);              // Chờ 1 giây
}