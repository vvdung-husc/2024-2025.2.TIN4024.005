#include <Arduino.h>
#include <TM1637Display.h>

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6ynMLHQcL"
#define BLYNK_TEMPLATE_NAME "Blink an LED on ESP32"
#define BLYNK_AUTH_TOKEN "Ldlqq0grRPlr44NUWp6WVv9mgJ6C_RUV"
// Phải để trước khai báo sử dụng thư viện Blynk

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
// Định nghĩa các chân kết nối
#define BTN_PIN 26  // Chân kết nối nút bấm
#define LED_PIN 21  // Chân điều khiển đèn LED
#define CLK 32      // Chân CLK của màn hình TM1637
#define DIO 33      // Chân DIO của màn hình TM1637

const char* ssid = "Wokwi-GUEST";  // Tên mạng WiFi
const char* pass = "";            // Mật khẩu mạng WiFi (Wokwi-GUEST không cần)

TM1637Display display(CLK, DIO);   // Khởi tạo màn hình TM1637
bool displayEnabled = true;        // Trạng thái hiển thị màn hình
bool buttonPressed = false;        // Trạng thái nút bấm
unsigned long lastMillis = 0;
unsigned long startTime = 0;       // Biến lưu thời gian bắt đầu kết nối

// Xử lý khi có tín hiệu từ Blynk (chân ảo V1)
BLYNK_WRITE(V1) {
    displayEnabled = param.asInt();  // Cập nhật trạng thái hiển thị từ Blynk
}

// Hàm ngắt khi nhấn nút
void IRAM_ATTR handleButtonPress() {
    static unsigned long lastPressTime = 0;
    if (millis() - lastPressTime > 200) {  // Chống nhiễu (debounce) 200ms
        buttonPressed = !buttonPressed;
        displayEnabled = buttonPressed; // Đồng bộ trạng thái hiển thị với trạng thái nút
        lastPressTime = millis();
    }
}

void setup() {
    Serial.begin(115200);  // Khởi động Serial Monitor
    WiFi.begin(ssid, pass);
    int wifiTimeout = 20;  // Giới hạn thời gian kết nối WiFi (10 giây)
    while (WiFi.status() != WL_CONNECTED && wifiTimeout > 0) {
        delay(500);
        Serial.print(".");
        wifiTimeout--;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
    } else {
        Serial.println("\nFailed to connect to WiFi");
    }
    
    startTime = millis() / 1000;  // Lưu thời gian bắt đầu kết nối
    Serial.print("Connected time (seconds): ");
    Serial.println(startTime);
    
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Kết nối với Blynk
    
    pinMode(LED_PIN, OUTPUT);      // Cấu hình chân LED là OUTPUT
    pinMode(BTN_PIN, INPUT_PULLUP);// Cấu hình chân nút bấm là INPUT_PULLUP
    attachInterrupt(BTN_PIN, handleButtonPress, FALLING); // Gán ngắt khi nhấn nút
    
    display.setBrightness(7); // Đặt độ sáng cho màn hình TM1637
}

void loop() {
    Blynk.run();  // Chạy Blynk để cập nhật trạng thái từ đám mây
    
    unsigned long currentMillis = (millis() / 1000) - startTime; // Tính thời gian chạy
    Serial.print("Uptime (seconds): ");
    Serial.println(currentMillis);
    
    // Gửi thời gian hoạt động lên Blynk (chân ảo V0)
    Blynk.virtualWrite(V0, currentMillis);
    
    if (displayEnabled) {
        display.showNumberDec(currentMillis, true); // Hiển thị thời gian chạy trên màn hình TM1637
    } else {
        display.clear(); // Tắt màn hình nếu không bật
    }
    
    digitalWrite(LED_PIN, displayEnabled ? HIGH : LOW); // Bật/tắt LED theo trạng thái hiển thị
    delay(1000);  // Cập nhật mỗi giây
}