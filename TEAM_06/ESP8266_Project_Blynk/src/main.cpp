#include <Arduino.h>
#include "utils.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <U8g2lib.h>

// Nguyễn Thị Diệu Anh
//  #define BLYNK_TEMPLATE_ID "TMPL65KeiW37P"
//  #define BLYNK_TEMPLATE_NAME "ESP8266 Blynk"
//  #define BLYNK_AUTH_TOKEN "gng5vfv6VX3INESnXUG2NR--HZGJoFzF"
//  Lê Thị Thanh Nhàn
//  #define BLYNK_TEMPLATE_ID "TMPL62XUJKqGx"
//  #define BLYNK_TEMPLATE_NAME "ESP8266"
//  #define BLYNK_AUTH_TOKEN "gPBpuIA0Xv0Xvcboh5ScnjV4-I0qXuNp"

// Phan Duy An
// #define BLYNK_TEMPLATE_ID "TMPL6N0zfQq82"
// #define BLYNK_TEMPLATE_NAME "ESP8266ProjectBlynk"
// #define BLYNK_AUTH_TOKEN "yQZI9rRQXSkvvlHXDl2Q3mYfliVw7DEz"

// Lại Văn Minh
// #define BLYNK_TEMPLATE_ID "TMPL6i2MGfRU1"
// #define BLYNK_TEMPLATE_NAME "ESP8266 Blynk"
// #define BLYNK_AUTH_TOKEN "BLFlZ2Sxwakztxri0uOfXWB_-vxHxGm5"
//Phạm Ngọc Cảm
#define BLYNK_TEMPLATE_ID "TMPL6sN3PjWNN"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "i3mR0CEiA6tRyQiIZ83H8Na5uGpQbqkT"

#include <Esp8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define dhtPIN 16     // Digital pin connected to the DHT sensor
#define dhtTYPE DHT11 // DHT 22 (AM2302)

#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

DHT dht(D0, dhtTYPE);

// WiFi credentials
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

bool buttonState = false;

bool WelcomeDisplayTimeout(uint msSleep = 5000)
{
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone)
    return true;
  if (!IsReady(lastTimer, msSleep))
    return false;
  bDone = true;
  return bDone;
}

void setup()
{
  Serial.begin(115200);
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);

  dht.begin();

  Wire.begin(OLED_SDA, OLED_SCL); // SDA, SCL
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("❌ WiFi connection failed!");
  }
  else
  {
    Serial.println("✅ WiFi connected!");
  }
  Blynk.virtualWrite(V3, buttonState);
  oled.begin();
  oled.clearBuffer();

  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT");

  oled.sendBuffer();
  randomSeed(analogRead(0));
}

void ThreeLedBlink()
{
  static ulong lastTimer = 0;
  static int currentLed = 0;
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (!IsReady(lastTimer, 1000))
    return;
  int prevLed = (currentLed + 2) % 3;
  digitalWrite(ledPin[prevLed], LOW);
  digitalWrite(ledPin[currentLed], HIGH);
  currentLed = (currentLed + 1) % 3;
}

float fHumidity = 0.0;
float fTemperature = 0.0;

void updateDHT()
{
  static ulong lastTimer = 0;
  if (!IsReady(lastTimer, 2000))
    return;

  // float h = dht.readHumidity();
  // float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  float h = random(0, 10001) / 100.0;    // [0.00, 100.00]
  float t = random(-4000, 8001) / 100.0; // [-40.00, 80.00]

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  bool bDraw = false;

  if (fTemperature != t)
  {
    bDraw = true;
    fTemperature = t;
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
  }

  if (fHumidity != h)
  {
    bDraw = true;
    fHumidity = h;
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
  }
  if (bDraw)
  {
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);

    String s = StringFormat("Nhiet do: %.2f °C", t);
    oled.drawUTF8(0, 14, s.c_str());

    s = StringFormat("Do am: %.2f %%", h);
    oled.drawUTF8(0, 42, s.c_str());

    oled.sendBuffer();
  }
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
}

void DrawCounter()
{
  static uint counter = 0; // Biến đếm
  static ulong lastTimer = 0;
  if (!IsReady(lastTimer, 2000))
    return;

  // Bắt đầu vẽ màn hình
  oled.clearBuffer();
  oled.setFont(u8g2_font_logisoso32_tf); // Chọn font lớn để hiển thị số
  oled.setCursor(30, 40);                // Đặt vị trí chữ
  oled.print(counter);                   // Hiển thị số đếm
  oled.sendBuffer();                     // Gửi dữ liệu lên màn hình

  counter++; // Tăng giá trị đếm
}
void BlinkingYellowLight()
{
  static bool yellowState = false;
  static unsigned long previousBlinkMillis = 0;
  if (IsReady(previousBlinkMillis, 500))
  {
    yellowState = !yellowState;
    digitalWrite(yPIN, yellowState);
  }

  digitalWrite(gPIN, LOW);
  digitalWrite(rPIN, LOW);
}
void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return; // Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
}
BLYNK_WRITE(V3)
{
  buttonState = param.asInt();
}
void loop()
{
  if (!WelcomeDisplayTimeout())
    return;
  ThreeLedBlink();
  updateDHT();
  uptimeBlynk();
  if (buttonState)
  {
    BlinkingYellowLight();
  }
}