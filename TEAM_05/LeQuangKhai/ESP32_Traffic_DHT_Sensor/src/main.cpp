#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6tI4ECcgM"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic DHT Sensor"
#define BLYNK_AUTH_TOKEN "0ghl27b4FrOswOyGnFJb5GJPRv9_UVd3"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  // Tên mạng WiFi
char pass[] = "";             // Mật khẩu mạng WiFi

// Định nghĩa các chân kết nối
#define btnBLED  23 // Chân kết nối nút bấm
#define pinBLED  5 // Chân kết nối đèn xanh
#define CLK 18      // Chân kết nối CLK của TM1637
#define DIO 19      // Chân kết nối DIO của TM1637
#define DHTPIN 16   // Chân kết nối DHT22
#define DHTTYPE DHT22 // Loại cảm biến DHT

// Biến toàn cục
ulong currentMiliseconds = 0; // Thời gian hiện tại - miliseconds
bool blueButtonON = true;     // Trạng thái của nút bấm ON -> đèn Xanh sáng và hiển thị LED TM1637
float temperature = 0;        // Nhiệt độ
float humidity = 0;           // Độ ẩm

// Khởi tạo màn hình TM1637 và cảm biến DHT22
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

// Hàm kiểm tra thời gian
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT22();

void setup() {
  // Khởi tạo Serial
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  // Khởi tạo màn hình TM1637 và cảm biến DHT22
  display.setBrightness(0x0f);
  dht.begin();

  // Kết nối WiFi và Blynk
  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Kết nối đến mạng WiFi

  Serial.println();
  Serial.println("WiFi connected");

  // Đồng bộ trạng thái đèn với Blynk
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  Serial.println("== START ==>");
  digitalWrite(pinBLED, HIGH);

}

void loop() {
  Blynk.run();  // Chạy Blynk để cập nhật trạng thái từ Blynk Cloud

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  readDHT22();
}

// Hàm kiểm tra thời gian
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Hàm cập nhật trạng thái nút bấm
void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return; // Hạn chế bấm nút quá nhanh - 50ms mỗi lần bấm
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON); // Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk

  if (blueButtonON) {
    Serial.println("Blue Light ON");
  } else {
    Serial.println("Blue Light OFF");
    display.clear();
  }
}

// Hàm cập nhật thời gian hoạt động (uptime)
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; // Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);  // Gửi giá trị lên chân ảo V0 trên ứng dụng Blynk
  if (blueButtonON) {
    display.showNumberDec(value);
  }
}

// Hàm đọc dữ liệu từ cảm biến DHT22
void readDHT22() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return; // Đọc dữ liệu mỗi 2 giây

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  temperature = t;
  humidity = h;

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V2, temperature); // Nhiệt độ
  Blynk.virtualWrite(V3, humidity);    // Độ ẩm

  Serial.print("Nhiệt độ: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Độ ẩm: "); Serial.print(humidity); Serial.println(" %");
}

// Hàm xử lý dữ liệu từ Blynk (điều khiển đèn LED)
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();  // Lấy giá trị từ ứng dụng Blynk
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

  if (blueButtonON) {
    Serial.println("Blynk -> Đèn xanh BẬT");
  } else {
    Serial.println("Blynk -> Đèn xanh TẮT");
    display.clear();
  }
}