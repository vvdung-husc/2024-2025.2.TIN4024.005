#include <Arduino.h>
#include <TM1637Display.h>
/* Thông tin từ Blynk Device Info */
#define BLYNK_TEMPLATE_ID "TMPL6sHrgFVmd"
#define BLYNK_TEMPLATE_NAME "blynk"
#define BLYNK_AUTH_TOKEN "CWpJtebw71oCAiDE8MqynsSO8qzDd0oE"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>


// Thông tin WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Định nghĩa chân kết nối cho nút bấm và LED
#define btnBLED 23   // Nút bấm nối tới GPIO23
#define pinBLED 21   // LED nối tới GPIO21

// Định nghĩa chân cho màn hình TM1637
#define CLK 18       // CLK của TM1637 nối tới GPIO18
#define DIO 19       // DIO của TM1637 nối tới GPIO19

// Định nghĩa chân cho cảm biến DHT22 (theo sơ đồ: dht1:SDA nối tới GPIO16)
#define DHTPIN 16    
#define DHTTYPE DHT22  // Sử dụng cảm biến DHT22

// Khởi tạo đối tượng cảm biến DHT và màn hình TM1637
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

// Biến toàn cục
ulong currentMiliseconds = 0;   // Thời gian hiện tại (millis)
bool blueButtonON = true;       // Trạng thái LED xanh

// Nguyên mẫu các hàm
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void sendSensorData();

void setup()
{
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");

  // Cập nhật trạng thái ban đầu của LED và gửi lên Blynk (V1)
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  Serial.println("== START ==");
}

void loop()
{
  Blynk.run();
  currentMiliseconds = millis();

  uptimeBlynk();      // Gửi thời gian hoạt động lên Blynk (V0) và hiển thị trên TM1637
  updateBlueButton(); // Kiểm tra nút bấm để bật/tắt LED xanh và đồng bộ lên Blynk (V1)
  sendSensorData();   // Đọc nhiệt độ & độ ẩm từ DHT22 và gửi lên Blynk (V2, V3)
}

// Kiểm tra xem đã đủ khoảng thời gian hay chưa
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Hàm cập nhật trạng thái nút bấm và LED xanh
void updateBlueButton()
{
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  
  if (!IsReady(lastTime, 50))
    return;
  
  int v = digitalRead(btnBLED);
  if (v == lastValue)
    return;
  lastValue = v;
  
  // Chỉ xử lý khi nút được thả (LOW -> HIGH)
  if (v == LOW)
    return;

  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  if (blueButtonON)
    Serial.println("Blue Light ON");
  else {
    Serial.println("Blue Light OFF");
    display.clear();
  }
}

// Hàm gửi thời gian hoạt động lên Blynk và hiển thị trên màn hình 7-seg
void uptimeBlynk()
{
  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000))
    return;
  
  ulong value = lastTime / 1000;   // Chuyển đổi sang giây
  Blynk.virtualWrite(V0, value);
  
  if (blueButtonON)
    display.showNumberDec(value);
}

// Hàm đọc dữ liệu nhiệt độ và độ ẩm từ DHT22 và gửi lên Blynk
void sendSensorData()
{
  static ulong lastTimeDHT = 0;
  
  if (!IsReady(lastTimeDHT, 2000)) // Đọc mỗi 2 giây
    return;

  float nhietdo = dht.readTemperature();
  float doam    = dht.readHumidity();

  if (isnan(nhietdo) || isnan(doam))
  {
    Serial.println("Lỗi: Không đọc được dữ liệu từ cảm biến DHT22!");
    return;
  }

  Blynk.virtualWrite(V2, nhietdo); // Gửi nhiệt độ lên V2
  Blynk.virtualWrite(V3, doam);    // Gửi độ ẩm lên V3

  Serial.print("Nhiệt độ: ");
  Serial.print(nhietdo);
  Serial.print(" °C | Độ ẩm: ");
  Serial.print(doam);
  Serial.println(" %");
}

// Hàm xử lý dữ liệu nhận từ Blynk trên chân ảo V1 (điều khiển LED xanh)
BLYNK_WRITE(V1)
{
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (blueButtonON)
    Serial.println("Blynk -> Blue Light ON");
  else {
    Serial.println("Blynk -> Blue Light OFF");
    display.clear();
  }
}
