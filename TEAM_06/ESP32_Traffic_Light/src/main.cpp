#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa các chân kết nối
#define BUTTON_PIN 23
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define LED_PINK 21

#define LDR_PIN 13

#define CLK 18
#define DIO 19

#define RED_TIME 5000
#define GREEN_TIME 7000
#define YELLOW_TIME 3000

// TML1637 4-wire interface
TM1637Display display(CLK, DIO);

enum TrafficLightState
{
  GREEN,
  YELLOW,
  RED
};

TrafficLightState lightState = GREEN;
int countDown = GREEN_TIME / 1000;
unsigned long currentMillis = 0;
bool buttonState = false;

bool IsRead(unsigned long &previousMillis, unsigned long milliseconds)
{
  if (currentMillis - previousMillis < milliseconds)
    return false;

  previousMillis = currentMillis; // Cập nhật thời gian lần cuối cùng
  return true;
}

void NormalTrafficLight()
{
  static unsigned long previousMillis = 0;
  if (IsRead(previousMillis, 1000))
  {
    countDown--;

    if (countDown < 0)
    {
      switch (lightState)
      {
      case GREEN:
        lightState = YELLOW;
        countDown = YELLOW_TIME / 1000;
        break;
      case YELLOW:
        lightState = RED;
        countDown = RED_TIME / 1000;
        break;
      case RED:
        lightState = GREEN;
        countDown = GREEN_TIME / 1000;
        break;
      }
    }

    display.showNumberDec(countDown);
  }

  switch (lightState)
  {
  case GREEN:
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    break;
  case RED:
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    break;
  case YELLOW:
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    break;
  }
}

void BlinkingLights()
{
  static bool blinkState = false;
  static unsigned long previousBlinkMillis = 0;

  if (IsRead(previousBlinkMillis, 500))
  {
    blinkState = !blinkState;
    digitalWrite(LED_RED, blinkState);
    digitalWrite(LED_YELLOW, blinkState);
    digitalWrite(LED_GREEN, blinkState);
  }

  display.clear();
}

void BlinkingYellowLight()
{
  static bool yellowState = false;
  static unsigned long previousBlinkMillis = 0;

  if (IsRead(previousBlinkMillis, 500))
  {
    yellowState = !yellowState;
    digitalWrite(LED_YELLOW, yellowState);
  }

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  display.clear();
}

void setup()
{
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_PINK, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(7);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  display.showNumberDec(countDown);
}

void loop()
{
  currentMillis = millis();
  float ldr = analogRead(LDR_PIN);
  float ldrLux = map(ldr, 0, 4095, 1000, 0);
  static unsigned long previousDarkStart = 0;
  static unsigned long previousButtonMillis = 0;

  Serial.print("LDR Raw Value: ");
  Serial.print(ldr);
  Serial.print(" | LDR Lux: ");
  Serial.println(ldrLux);

  if (digitalRead(BUTTON_PIN) == LOW && IsRead(previousButtonMillis, 50))
  {
    buttonState = !buttonState;
    Serial.print("Button State: ");
    Serial.println(buttonState);
  }

  if (IsRead(previousDarkStart, 50))
  {
    if (ldrLux < 300)
    {
      BlinkingYellowLight();
    }
    else
    {
      if (buttonState)
      {
        digitalWrite(LED_PINK, HIGH);
        BlinkingLights();
      }
      else
      {
        digitalWrite(LED_PINK, LOW);
        NormalTrafficLight();
      }
    }
  }
}
