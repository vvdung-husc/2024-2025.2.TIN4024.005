#define BLYNK_TEMPLATE_ID "TMPL6G-_1Se-p"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "J1RzJuboKtIHo0ZCcegBupFpHC00XLDC"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <BlynkSimpleEsp8266.h>



#define gPIN 15
#define yPIN 2
#define rPIN 5

#define OLED_SDA 13
#define OLED_SCL 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
bool yellowBlinkMode = false; // Chế độ đèn vàng nhấp nháy

// Kết nối WiFi
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

void setup() {
  Serial.begin(115200);
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  Blynk.begin(auth, ssid, pass);

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT");
  oled.sendBuffer();
}

// Blynk switch để bật chế độ đèn vàng nhấp nháy
BLYNK_WRITE(V1) {
  yellowBlinkMode = param.asInt();
}

void ThreeLedBlink() {
  static ulong lastTimer = 0;
  static int currentLed = 0;
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (millis() - lastTimer < 1000) return;
  lastTimer = millis();

  if (yellowBlinkMode) {
    digitalWrite(gPIN, LOW);
    digitalWrite(rPIN, LOW);
    digitalWrite(yPIN, !digitalRead(yPIN));
  } else {
    digitalWrite(ledPin[(currentLed + 2) % 3], LOW);
    digitalWrite(ledPin[currentLed], HIGH);
    currentLed = (currentLed + 1) % 3;
  }
}

void updateDHT() {
  static ulong lastTimer = 0;
  if (millis() - lastTimer < 2000) return;
  lastTimer = millis();

  float t = random(-400, 800) / 10.0; // Giả lập nhiệt độ (-40.0 đến 80.0)
  float h = random(0, 1000) / 10.0;   // Giả lập độ ẩm (0.0% đến 100.0%)

  Serial.printf("Temperature: %.2f °C, Humidity: %.2f %%\n", t, h);

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);
  oled.setCursor(0, 20);
  oled.printf("Nhiet do: %.2f °C", t);
  oled.setCursor(0, 40);
  oled.printf("Do am: %.2f %%", h);
  oled.sendBuffer();

  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);
}

void loop() {
  Blynk.run();
  ThreeLedBlink();
  updateDHT();
}
