
#include <Arduino.h>
#include "utils.h"

#include <Wire.h>
#include <U8g2lib.h>

// Chân LED
#define DEN_DO    5    // D1
#define DEN_VANG  2    // D4
#define DEN_XANH  15   // D8

// OLED I2C
#define OLED_SDA 13    // D7
#define OLED_SCL 12    // D6

// Khởi tạo màn hình OLED
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// Biến hệ thống
float nhietDo = 0.0;
float doAm = 0.0;
unsigned long thoiGianChay = 0;
unsigned long thoiGianTruoc = 0;
unsigned long demThoiGian = 0;

int trangThaiDen = 0; // 0: đỏ, 1: xanh, 2: vàng
unsigned long denThoiGianTruoc = 0;

void setup() {
  Serial.begin(115200);

  
  pinMode(DEN_DO, OUTPUT);
  pinMode(DEN_VANG, OUTPUT);
  pinMode(DEN_XANH, OUTPUT);

  // Bắt đầu với đèn đỏ
  digitalWrite(DEN_DO, HIGH);
  digitalWrite(DEN_VANG, LOW);
  digitalWrite(DEN_XANH, LOW);

  // Khởi tạo OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);




  // Hiển thị khởi động
  oled.drawUTF8(0, 16, "Khoi dong OLED...");
  oled.sendBuffer();
  delay(2000);  

  oled.clearBuffer();
  oled.drawUTF8(0, 16, "Man hinh OLED ");
  oled.drawUTF8(0, 32, " thanh cong");
  oled.sendBuffer();
  delay(2000);
}



void capNhatDuLieuNgauNhien() {
  if (millis() - thoiGianTruoc >= 2000) {
    thoiGianTruoc = millis();

    // Nhiệt độ từ -40.0 đến 100.0
    nhietDo = random(-400, 1000 + 1) / 10.0;

    // Độ ẩm từ 0.0 đến 100.0
    doAm = random(0, 1000 + 1) / 10.0;
  }
}


void capNhatThoiGian() {
  if (millis() - demThoiGian >= 1000) {
    demThoiGian = millis();
    thoiGianChay++;
  }
}

// Hàm: Điều khiển đèn giao thông tự động
void capNhatDenGiaoThong() {
  unsigned long thoiGianDoi[] = {1000, 1000, 1000}; // đỏ 1s, xanh 1s, vàng 1s

  if (millis() - denThoiGianTruoc >= thoiGianDoi[trangThaiDen]) {
    denThoiGianTruoc = millis();

    // Tắt tất cả đèn
    digitalWrite(DEN_DO, LOW);
    digitalWrite(DEN_XANH, LOW);
    digitalWrite(DEN_VANG, LOW);

    // Chuyển trạng thái
    trangThaiDen = (trangThaiDen + 1) % 3;

    // Bật đèn tương ứng
    if (trangThaiDen == 0) digitalWrite(DEN_DO, HIGH);
    else if (trangThaiDen == 1) digitalWrite(DEN_XANH, HIGH);
    else if (trangThaiDen == 2) digitalWrite(DEN_VANG, HIGH);
  }
}

// Hàm: Hiển thị thông tin lên OLED
void hienThiOLED() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);

  oled.drawUTF8(0, 14, ("Nhiet do: " + String(nhietDo, 1) + "°C").c_str());
  oled.drawUTF8(0, 30, ("Do am: " + String(doAm, 1) + "%").c_str());
  oled.drawUTF8(0, 46, ("Thoi gian: " + String(thoiGianChay) + "s").c_str());
  oled.sendBuffer();
}

void loop() {
  capNhatDuLieuNgauNhien();
  capNhatThoiGian();
  capNhatDenGiaoThong();
  hienThiOLED();
}
