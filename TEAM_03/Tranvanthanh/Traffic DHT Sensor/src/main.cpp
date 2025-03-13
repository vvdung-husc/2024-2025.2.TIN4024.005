
#define BLYNK_TEMPLATE_ID "TMPL6lbv2NwWh"
#define BLYNK_TEMPLATE_NAME "Traffic DHT Sensor"
#define BLYNK_AUTH_TOKEN "NF60oMrAbcJ2JMuQkNEKRmRt2VS_O3X-"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

// Thông tin WiFi và Blynk
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Định nghĩa chân kết nối
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define CLK_PIN 18
#define DIO_PIN 19

#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK_PIN, DIO_PIN);

// Biến toàn cục
bool displayOn = true;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long sensorLastReadTime = 0;
const unsigned long sensorInterval = 2000;

double temperature = 0.0;
double humidity = 0.0;

unsigned long lastUptimeSendTime = 0;
const unsigned long uptimeInterval = 1000;

BlynkTimer timer;

// Callback khi thay đổi trạng thái hiển thị từ Blynk
BLYNK_WRITE(V2) {
  displayOn = param.asInt();
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
}

// Gửi nhiệt độ và độ ẩm lên Blynk
void sendSensorData() {
  double temp = dht.readTemperature();
  double hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    Blynk.virtualWrite(V4, temp);
    Blynk.virtualWrite(V5, hum);
    Serial.print("Temp: "); Serial.print(temp);
    Serial.print(" | Hum: "); Serial.println(hum);
  } else {
    Serial.println("Lỗi đọc dữ liệu từ DHT22!");
  }
}

// Hiển thị thời gian lên màn hình LED
void displayTime(unsigned long seconds) {
  int minutes = seconds / 60;
  int secs = seconds % 60;
  int timeToShow = (minutes * 100) + secs; // Hiển thị MM:SS
  display.showNumberDecEx(timeToShow, 0x40, true);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  display.setBrightness(7);
  display.showNumberDec(0, true);
  
  dht.begin();
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected to Blynk");
  
  digitalWrite(LED_PIN, displayOn ? HIGH : LOW);
  Blynk.virtualWrite(V1, displayOn);

  // Đọc cảm biến định kỳ
  timer.setInterval(sensorInterval, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
  unsigned long currentMillis = millis();

  // Xử lý debounce nút nhấn
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

  // Gửi thời gian hoạt động lên Blynk và hiển thị trên LED
  if (currentMillis - lastUptimeSendTime >= uptimeInterval) {
    lastUptimeSendTime = currentMillis;
    unsigned long uptimeSeconds = currentMillis / 1000;
    Blynk.virtualWrite(V1, uptimeSeconds);
    
    if (displayOn) {
      displayTime(uptimeSeconds);
    } else {
      display.clear();
    }
  }
}
