#include <Arduino.h>
#include "utils.h"

#include <Wire.h>
#include <U8g2lib.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

bool WelcomeDisplayTimeout(uint msSleep = 5000){
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;    
  return bDone;
}

void setup() {
  Serial.begin(115200);
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);
  
  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);  // SDA, SCL

  oled.begin();
  oled.clearBuffer();
  
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");  
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT"); 

  oled.sendBuffer();
}

void updateTemperature(){
  static ulong lastTimer = 0;  
  if (!IsReady(lastTimer, 2000)) return;

  int t = random(-40, 81); // Sinh nhiệt độ ngẫu nhiên từ -40°C đến 80°C

  char buffer[20];
  sprintf(buffer, "Nhiệt độ: %d°C", t);

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 42, buffer);
  oled.sendBuffer();

  Serial.println(buffer);
}

void loop() {
  if (!WelcomeDisplayTimeout()) return;
  updateTemperature();
}