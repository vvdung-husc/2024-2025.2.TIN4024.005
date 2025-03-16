#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#define BLYNK_TEMPLATE_ID "TMPL627QxysZi"
#define BLYNK_TEMPLATE_NAME "Lab2"
#define BLYNK_AUTH_TOKEN "DmNVWx8pR-aKjTmnvstBekuRyKTvhgeT"

#include <BlynkSimpleEsp32.h>
#include "DHT.h"

char wifi_ten[] = "Wokwi-GUEST";
char wifi_mk[] = "";

#define nut_bam 23
#define den_xanh 21
#define chan_CLK 18
#define chan_DIO 19
#define cam_bien_DHT 16
#define loai_DHT DHT22

ulong tg_hientai = 0;
bool den_bat = true;
float do_am = 0.0;
float nhiet_do = 0.0;

TM1637Display man_hinh(chan_CLK, chan_DIO);
DHT dht(cam_bien_DHT, loai_DHT);

bool KiemTraTG(ulong &demTG, uint32_t mili);
void capNhatNutBam();
void guiTG_Blynk();
void docCamBienDHT();

BLYNK_WRITE(V3) {  // Nút bấm (V3)
  den_bat = param.asInt();
  digitalWrite(den_xanh, den_bat ? HIGH : LOW);
  if (!den_bat) {
    man_hinh.clear();
  }
  Serial.print("Trạng thái đèn: ");
  Serial.println(den_bat ? "BẬT" : "TẮT");
}

BLYNK_CONNECTED() {
  Serial.println("✅ Đã kết nối Blynk");
  Blynk.syncVirtual(V3);
  Blynk.virtualWrite(V0, millis() / 1000);
}

void setup() {
  Serial.begin(115200);
  pinMode(den_xanh, OUTPUT);
  pinMode(nut_bam, INPUT_PULLUP);
  man_hinh.setBrightness(0x0f);
  
  Serial.print("🔄 Đang kết nối WiFi: ");
  Serial.println(wifi_ten);
  Blynk.begin(BLYNK_AUTH_TOKEN, wifi_ten, wifi_mk);
  Serial.println("✅ WiFi đã kết nối");
  
  dht.begin();
  digitalWrite(den_xanh, den_bat ? HIGH : LOW);
  Blynk.virtualWrite(V3, den_bat);
}

void loop() {
  Blynk.run();
  if (Blynk.connected()) {
    tg_hientai = millis();
    guiTG_Blynk();
    capNhatNutBam();
    docCamBienDHT();
  } else {
    Serial.println("⚠️ Mất kết nối Blynk, đang thử lại...");
    Blynk.begin(BLYNK_AUTH_TOKEN, wifi_ten, wifi_mk);
  }
}

bool KiemTraTG(ulong &demTG, uint32_t mili) {
  if (tg_hientai - demTG < mili) return false;
  demTG = tg_hientai;
  return true;
}

void capNhatNutBam() {
  static ulong tg_cuoi = 0;
  static int gia_tri_cuoi = HIGH;
  if (!KiemTraTG(tg_cuoi, 50)) return;
  int gia_tri_hientai = digitalRead(nut_bam);
  if (gia_tri_hientai == gia_tri_cuoi) return;
  gia_tri_cuoi = gia_tri_hientai;
  if (gia_tri_hientai == LOW) return;
  
  den_bat = !den_bat;
  digitalWrite(den_xanh, den_bat ? HIGH : LOW);
  if (!den_bat) {
    man_hinh.clear();
  }
  Serial.println(den_bat ? "💡 Đèn BẬT" : "💡 Đèn TẮT");
  Blynk.virtualWrite(V3, den_bat);
}

void guiTG_Blynk() {
  static ulong tg_cuoi = 0;
  if (!KiemTraTG(tg_cuoi, 1000)) return;
  ulong tg_chay = millis() / 1000;
  Blynk.virtualWrite(V0, tg_chay);  // Gửi thời gian lên V0
  if (den_bat) {
    man_hinh.showNumberDec(tg_chay);
  }
}

void docCamBienDHT() {
  static ulong tg_cuoi = 0;
  if (!KiemTraTG(tg_cuoi, 2000)) return;
  
  do_am = dht.readHumidity();
  nhiet_do = dht.readTemperature();
  
  if (isnan(do_am) || isnan(nhiet_do)) {
    Serial.println("⚠️ Lỗi đọc cảm biến DHT!");
    return;
  }
  
  Blynk.virtualWrite(V1, nhiet_do);  // Gửi nhiệt độ lên V1
  Blynk.virtualWrite(V2, do_am);     // Gửi độ ẩm lên V2
  
  Serial.print("🌡 Nhiệt độ: ");
  Serial.print(nhiet_do);
  Serial.print("°C, 💧 Độ ẩm: ");
  Serial.print(do_am);
  Serial.println("%");
}
