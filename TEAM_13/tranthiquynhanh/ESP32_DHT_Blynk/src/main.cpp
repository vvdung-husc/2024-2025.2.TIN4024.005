#include <Arduino.h>
#include <TM1637Display.h>

#define BLYNK_TEMPLATE_ID "TMPL6p5PEAxZE"
#define BLYNK_TEMPLATE_NAME "Blynk DHT"
#define BLYNK_AUTH_TOKEN "2q23K58TEgWhL-dgSHygbEB_OmY7NHMr"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

/* Thông tin kết nối WiFi */
char ssid[] = "Wokwi-GUEST";  
char pass[] = "";            

// Chân kết nối phần cứng
#define BTN_PIN  23  // Nút nhấn
#define LED_PIN  21  // LED xanh
#define CLK 18       // TM1637 CLK
#define DIO 19       // TM1637 DIO
#define DHT_PIN 16   // Cảm biến DHT22
#define DHT_TYPE DHT22  

// Biến toàn cục
ulong currentMillis = 0;
bool ledState = true;

// Khởi tạo module
TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

bool isReady(ulong &lastTime, uint32_t interval);
void handleButton();
void updateUptime();
void updateTemperatureHumidity();

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  display.setBrightness(0x0f);
  dht.begin();

  Serial.print("Đang kết nối WiFi: ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); 

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Kết nối WiFi thất bại!");
  } else {
    Serial.println("✅ WiFi đã kết nối!");
  }
  
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  Blynk.virtualWrite(V1, ledState);
}

void loop() {  
  Blynk.run();  // Duy trì kết nối với Blynk

  currentMillis = millis();
  updateUptime();
  handleButton();
  updateTemperatureHumidity();
}

// Hạn chế spam tín hiệu
bool isReady(ulong &lastTime, uint32_t interval) {
  if (currentMillis - lastTime < interval) return false;
  lastTime = currentMillis;
  return true;
}

// Xử lý nút nhấn
void handleButton() {
  static ulong lastPress = 0;
  static int lastState = HIGH;
  if (!isReady(lastPress, 50)) return;
  
  int buttonState = digitalRead(BTN_PIN);
  if (buttonState == lastState) return;
  lastState = buttonState;
  if (buttonState == LOW) return;

  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  Blynk.virtualWrite(V1, ledState);

  if (!ledState) {
    display.clear();
  }
}

// Hiển thị thời gian chạy
void updateUptime() {
  static ulong lastUpdate = 0;
  if (!isReady(lastUpdate, 1000)) return;

  ulong seconds = currentMillis / 1000;  
  Blynk.virtualWrite(V0, seconds);
  
  if (ledState) {
    display.showNumberDec(seconds);
  }
}


void updateTemperatureHumidity() {
  static ulong lastUpdate = 0;
  if (!isReady(lastUpdate, 2000)) return;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (!isnan(temp) && !isnan(hum)) {
    Blynk.virtualWrite(V2, temp);  // Gửi nhiệt độ lên Blynk
    Blynk.virtualWrite(V3, hum);   // Gửi độ ẩm lên Blynk

    Serial.print("🌡 Nhiệt độ: ");
    Serial.print(temp);
    Serial.print(" °C | 💧 Độ ẩm: ");
    Serial.print(hum);
    Serial.println(" %");
  } else {
    Serial.println("⚠️ Lỗi đọc cảm biến DHT22! Thử lại...");
  }
}

// Điều khiển LED từ Blynk
BLYNK_WRITE(V1) { 
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  if (!ledState) {
    display.clear();
  }
}
