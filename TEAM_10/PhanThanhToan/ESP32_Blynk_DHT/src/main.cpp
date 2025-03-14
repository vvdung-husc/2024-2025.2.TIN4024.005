#define BLYNK_TEMPLATE_ID "TMPL6t86pyTYo"
#define BLYNK_TEMPLATE_NAME "PTToan"
#define BLYNK_AUTH_TOKEN "sQ6UIq1s8X0RFQ8sMJVdusM8R2Na16mL"

#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>



char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED  23 // Chân kết nối nút bấm
#define pinBLED  21 // Chân kết nối đèn xanh

#define CLK 18  // Chân kết nối CLK của TM1637
#define DIO 19  // Chân kết nối DIO của TM1637

// Biến toàn cục
ulong currentMiliseconds = 0; // Thời gian hiện tại - miliseconds 
bool blueButtonON = true;     // Trạng thái của nút bấm ON -> đèn xanh sáng và hiển thị LED TM1637

// Khởi tạo màn hình TM1637
TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  
  display.setBrightness(0x0f);
  
  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  Serial.println("== START ==>");
}


void loop() {  
  Blynk.run();
  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON) {
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  } else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
    display.clear();
  }    
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  display.showNumberDec(value); // Hiển thị giá trị uptime trên màn hình LED TM1637
}

BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  if (blueButtonON) {
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  } else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    display.clear();
  }
}