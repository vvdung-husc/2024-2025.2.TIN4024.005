#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6_Q_9TLMv"
#define BLYNK_TEMPLATE_NAME "Blynk"
#define BLYNK_AUTH_TOKEN "PkKAXTTQbku8E_HPhHa_NmGFfdj_zCNL"
// WiFi
char ssid[] = "Wokwi-GUEST";  
char pass[] = "";            

// Chân kết nối
#define btnBLED  23 
#define pinBLED  21 
#define CLK 18  
#define DIO 19  
#define DHT_PIN 16 
#define DHT_TYPE DHT22  

// Biến toàn cục
ulong currentMiliseconds = 0;
bool blueButtonON = true;

// Khởi tạo module
TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void updateTemperature();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  
  display.setBrightness(0x0f);
  dht.begin();

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
  updateTemperature();
}

// Hạn chế bấm nút quá nhanh
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Xử lý nút bấm
void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  if (!blueButtonON) {
    display.clear();
  }
}

// Hiển thị thời gian chạy lên Blynk & TM1637
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  if (blueButtonON) {
    display.showNumberDec(value);
  }
}

// Đọc và hiển thị nhiệt độ từ DHT22
void updateTemperature() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  float temperature = dht.readTemperature();
  if (!isnan(temperature)) {
    Blynk.virtualWrite(V2, temperature);
    Serial.print("Temperature: ");
    Serial.println(temperature);
    if (!blueButtonON) {
      display.showNumberDec((int)temperature);
    }
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }
}

// Xử lý điều khiển LED từ Blynk
BLYNK_WRITE(V1) { 
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (!blueButtonON) {
    display.clear();
  }
}
