#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

#define BLYNK_TEMPLATE_ID "TMPL60pMYcGkL"
#define BLYNK_TEMPLATE_NAME "Lab3"
#define BLYNK_AUTH_TOKEN "wz1Vwsafs3YS58taQXBeaCc8Y-v2LowA"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  // Tên mạng WiFi
char pass[] = "";             // Mật khẩu mạng WiFi

// Pin - Các đèn LED
#define rLED  27
#define yLED  26
#define gLED  25
#define btnBLED  23 // Chân kết nối nút bấm
#define pinBLED  21 // Chân kết nối đèn xanh
// Pin - TM1637
#define CLK   18
#define DIO   19

#define DHTPIN 16         // Chân DATA của cảm biến DHT22
#define DHTTYPE DHT22    // Loại cảm biến

// Pin - Cảm biến quang trở (thuộc ADC1)
#define ldrPIN  34

// Thời gian chờ cho đèn giao thông (ms)
const uint rTIME = 5000;  // Đèn đỏ: 5 giây
const uint yTIME = 3000;  // Đèn vàng: 3 giây
const uint gTIME = 10000; // Đèn xanh: 10 giây

// Biến ngưỡng ánh sáng (sẽ được cập nhật từ Blynk V4)
int darkThreshold = 500;   // Giá trị mặc định

// Khởi tạo cảm biến và hiển thị
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);  // Điều khiển bảng đếm ngược

// Biến trạng thái
bool blueButtonON = true;
int currentLED = rLED;
ulong ledTimeStart = 0;
int tmCounter = rTIME / 1000; // Biến đếm ngược trên TM1637
ulong uptime = 0;

// Hàm kiểm tra thời gian không chặn
bool IsReady(ulong &timer, uint32_t interval) {
  ulong currentTime = millis();
  if (currentTime - timer >= interval) {
    timer = currentTime;
    return true;
  }
  return false;
}

// Gửi dữ liệu cảm biến lên Blynk
void sendSensor() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Lỗi: Không đọc được dữ liệu từ DHT22!");
    return;
  }

  Serial.print("Nhiệt độ: ");
  Serial.print(temperature);
  Serial.print(" °C  |  Độ ẩm: ");
  Serial.print(humidity);
  Serial.println(" %");

  Blynk.virtualWrite(V1, temperature); // Gửi nhiệt độ lên V1 (0-100°C)
  Blynk.virtualWrite(V2, humidity);    // Gửi độ ẩm lên V2 (0-100%)
}

// Kiểm tra ánh sáng và kích hoạt chế độ tối với debounce
bool isDark() {
  static ulong darkTimeStart = 0;
  static uint16_t lastValue = 0;
  static bool bDark = false;
  static ulong lastChangeTime = 0; // Thêm biến để debounce
  const uint debounceDelay = 2000; // Độ trễ 2 giây để tránh nhấp nháy

  if (!IsReady(darkTimeStart, 50)) return bDark;

  uint16_t value = analogRead(ldrPIN);
  int mappedValue = map(value, 0, 4095, 0, 3000); // Ánh xạ giá trị LDR sang 0-3000
  if (value == lastValue) return bDark;

  bool shouldBeDark = (mappedValue > darkThreshold);
  ulong currentTime = millis();

  if (shouldBeDark != bDark) {
    if (currentTime - lastChangeTime > debounceDelay) {
      if (shouldBeDark) {
        if (!bDark) {
          Serial.print("DARK mode activated, value: ");
          Serial.print(mappedValue);
          Serial.print(", threshold: ");
          Serial.println(darkThreshold);
          digitalWrite(rLED, LOW);
          digitalWrite(gLED, LOW);
          digitalWrite(yLED, HIGH); // Bật đèn vàng ban đầu
          display.showNumberDec(0, true, 3, 0); // Hiển thị "000" trên TM1637
        }
        bDark = true;
      } else {
        if (bDark) {
          Serial.print("LIGHT mode activated, value: ");
          Serial.print(mappedValue);
          Serial.print(", threshold: ");
          Serial.println(darkThreshold);
          digitalWrite(yLED, LOW); // Tắt đèn vàng khi sáng
        }
        bDark = false;
      }
      lastChangeTime = currentTime;
    }
  }

  Blynk.virtualWrite(V4, mappedValue); // Gửi giá trị ánh sáng lên V4 (0-3000)
  lastValue = value;
  return bDark;
}

// Điều khiển đèn vàng nhấp nháy
void YellowLED_Blink() {
  static ulong yLedStart = 0;
  static bool isON = false;

  if (!IsReady(yLedStart, 500)) return; // Nhấp nháy mỗi 0.5 giây
  digitalWrite(yLED, isON ? HIGH : LOW);
  isON = !isON;
}

// Điều khiển đèn giao thông và TM1637
void TrafficLightControl() {
  static ulong lastUpdate = 0;
  if (!IsReady(lastUpdate, 1000)) return;

  ulong elapsedTime = millis() - ledTimeStart;
  int remainingTime = 0;

  switch (currentLED) {
    case rLED:
      remainingTime = (rTIME - elapsedTime) / 1000;
      tmCounter = remainingTime > 0 ? remainingTime : 0; // Cập nhật tmCounter
      if (elapsedTime >= rTIME) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        ledTimeStart = millis();
        tmCounter = gTIME / 1000;
        Serial.println("2. GREEN => YELLOW");
      }
      break;
    case gLED:
      remainingTime = (gTIME - elapsedTime) / 1000;
      tmCounter = remainingTime > 0 ? remainingTime : 0; // Cập nhật tmCounter
      if (elapsedTime >= gTIME) {
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        ledTimeStart = millis();
        tmCounter = yTIME / 1000;
        Serial.println("3. YELLOW => RED");
      }
      break;
    case yLED:
      remainingTime = (yTIME - elapsedTime) / 1000;
      tmCounter = remainingTime > 0 ? remainingTime : 0; // Cập nhật tmCounter
      if (elapsedTime >= yTIME) {
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        ledTimeStart = millis();
        tmCounter = rTIME / 1000;
        Serial.println("1. RED => GREEN");
      }
      break;
  }

  // Cập nhật thời gian đếm ngược trên TM1637 và gửi lên V0
  if (!isDark()) { // Chỉ cập nhật khi không ở chế độ tối
    display.showNumberDec(tmCounter, true, 2, 2);
    Blynk.virtualWrite(V0, tmCounter); // Gửi thời gian đếm ngược lên V0
  }
}

// Cập nhật thời gian hoạt động lên Blynk
void updateUptime() {
  static ulong lastUptime = 0;
  if (!IsReady(lastUptime, 1000)) return;
  uptime = (millis() / 1000);
  if (!blueButtonON && !isDark()) { // Hiển thị thời gian hoạt động khi không ở chế độ button hoặc tối
    display.showNumberDec(uptime % 100); // Hiển thị 2 chữ số cuối
    Blynk.virtualWrite(V0, uptime); // Gửi thời gian hoạt động lên V0
  }
}

// Xử lý nút bấm vật lý
void updateBlueButton() {
  static ulong lastDebounce = 0;
  static int lastState = HIGH;

  if (!IsReady(lastDebounce, 50)) return;

  int currentState = digitalRead(btnBLED);
  if (currentState == lastState) return;

  lastState = currentState;
  if (currentState == LOW) {
    blueButtonON = !blueButtonON;
    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V3, blueButtonON);
    Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
    if (!blueButtonON) display.clear();
  }
}

// Xử lý giá trị ngưỡng tối từ Blynk (V4)
BLYNK_WRITE(V4) {
  int newThreshold = param.asInt(); // Nhận giá trị ngưỡng từ Input Number
  if (newThreshold < 0) newThreshold = 0;
  if (newThreshold > 3000) newThreshold = 3000;
  darkThreshold = newThreshold; // Cập nhật ngưỡng
  Serial.print("Ngưỡng tối mới từ Blynk: ");
  Serial.println(darkThreshold);
}

void setup() {
  Serial.begin(115200);

  // Cấu hình chân
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(ldrPIN, INPUT);

  // Khởi động cảm biến và hiển thị
  dht.begin();
  display.setBrightness(7);

  // Khởi tạo trạng thái ban đầu
  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  ledTimeStart = millis();
  display.showNumberDec(tmCounter, true, 2, 2);

  // Kết nối Blynk
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");
  Serial.println("== START ==>");
  
  // Đồng bộ giá trị V4 từ Blynk
  Blynk.syncVirtual(V4);
}

void loop() {
  Blynk.run();

  if (isDark()) {
    YellowLED_Blink();
  } else {
    TrafficLightControl();
  }

  updateUptime();
  updateBlueButton();

  // Đọc và gửi dữ liệu cảm biến DHT22 mỗi 2 giây
  static ulong lastDHTRead = 0;
  if (IsReady(lastDHTRead, 2000)) {
    sendSensor();
  }
}

// Xử lý lệnh từ nút bấm trên Blynk (V3)
BLYNK_WRITE(V3) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V3, blueButtonON);
  Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
  if (!blueButtonON) display.clear();
}