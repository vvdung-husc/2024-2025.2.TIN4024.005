// Thông tin Blynk
#define BLYNK_TEMPLATE_ID "TMPL6yTjMiow8"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "B3du7AXEeav_KcitkbrjYKqZf63Yii-R"


#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Chân kết nối phần cứng
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define CLK_PIN 18
#define DIO_PIN 19
#define DHT_TYPE DHT22

// Khởi tạo đối tượng cảm biến và hiển thị
DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK_PIN, DIO_PIN);

// Biến điều khiển
bool displayOn = true;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long sensorLastReadTime = 0;
const unsigned long sensorInterval = 2000;

float temperature = 0.0;
float humidity = 0.0;

unsigned long lastUptimeSendTime = 0;
const unsigned long uptimeInterval = 1000;

// BLYNK: Nhận dữ liệu từ công tắc Replay
BLYNK_WRITE(V1) {
  displayOn = param.asInt();
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
}

// Kết nối WiFi và Blynk
void connectWiFi() {
  Serial.print("Kết nối WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    Serial.print(".");
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi đã kết nối!");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("Đã kết nối với Blynk");
  } else {
    Serial.println("\nKhông thể kết nối WiFi!");
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  display.setBrightness(7);
  display.showNumberDec(0, true);
  
  dht.begin();
  connectWiFi();
  
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
  Blynk.virtualWrite(V1, displayOn);
}

// Hiển thị thời gian dạng MM:SS trên TM1637
void displayTime(unsigned long seconds) {
  int minutes = seconds / 60;
  int secs = seconds % 60;
  int timeToShow = (minutes * 100) + secs; // MM:SS
  display.showNumberDecEx(timeToShow, 0x40, true);
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();
  
  // Kiểm tra nút nhấn debounce
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && lastButtonState == HIGH) {
      displayOn = !displayOn;
      Blynk.virtualWrite(V1, displayOn);
      Serial.print("Nút nhấn, displayOn: ");
      Serial.println(displayOn);
    }
  }
  lastButtonState = reading;
  
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
  
  // Đọc dữ liệu từ cảm biến DHT22
  if (currentMillis - sensorLastReadTime >= sensorInterval) {
    sensorLastReadTime = currentMillis;
    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();
    
    if (isnan(newTemperature) || isnan(newHumidity)) {
      Serial.println("Lỗi đọc DHT22!");
    } else {
      temperature = newTemperature;
      humidity = newHumidity;
      
      Serial.print("Nhiệt độ: ");
      Serial.print(temperature);
      Serial.print(" °C, Độ ẩm: ");
      Serial.print(humidity);
      Serial.println(" %");

      // Gửi lên Blynk
      Blynk.virtualWrite(V2, temperature);  // Gửi nhiệt độ lên V2
      Blynk.virtualWrite(V3, humidity);     // Gửi độ ẩm lên V3
    }
  }
  
  // Gửi thời gian uptime lên Blynk
  if (currentMillis - lastUptimeSendTime >= uptimeInterval) {
    lastUptimeSendTime = currentMillis;
    unsigned long uptimeSeconds = currentMillis / 1000;
    
    Blynk.virtualWrite(V0, uptimeSeconds); // Gửi uptime lên V0

    if (displayOn) {
      displayTime(uptimeSeconds);
    } else {
      display.clear();
    }
  }
}
