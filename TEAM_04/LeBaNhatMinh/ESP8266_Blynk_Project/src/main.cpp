#define BLYNK_TEMPLATE_ID "TMPL6N92hnKMZ"
#define BLYNK_TEMPLATE_NAME "ESP8266 Blynk Project"
#define BLYNK_AUTH_TOKEN "JuuTlquqg72SD0zhHGuFA5-eb_vNroz1"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define gPIN 15  // Đèn xanh
#define yPIN 2   // Đèn vàng
#define rPIN 5   // Đèn đỏ

#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo màn hình OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Kết nối WiFi
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

BlynkTimer timer;

// -------------------- Biến toàn cục --------------------
bool yellowBlinkMode = false; // Chế độ đèn vàng nhấp nháy
unsigned long startMillis; // Thời gian bắt đầu

float fHumidity = 0.0;
float fTemperature = 0.0;

// -------------------- Hàm kiểm tra thời gian --------------------
bool IsReady(unsigned long &lastTimer, unsigned long interval) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastTimer >= interval) {
        lastTimer = currentMillis;
        return true;
    }
    return false;
}

// -------------------- Hàm cập nhật dữ liệu --------------------
void updateData() {
    static unsigned long lastTimer = 0;
    if (!IsReady(lastTimer, 2000)) return;

    fTemperature = random(-400, 801) / 10.0;  // Nhiệt độ từ -40.0°C đến 80.0°C
    fHumidity = random(0, 1001) / 10.0;       // Độ ẩm từ 0.0% đến 100.0%

    Serial.print("Temperature: ");
    Serial.print(fTemperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(fHumidity);
    Serial.println(" %");

    Blynk.virtualWrite(V1, fTemperature);
    Blynk.virtualWrite(V2, fHumidity);

    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese1);
    oled.drawUTF8(0, 14, "Nhiet do: ");
    oled.setCursor(80, 14);
    oled.print(fTemperature);
    oled.print("°C");

    oled.drawUTF8(0, 42, "Do am: ");
    oled.setCursor(80, 42);
    oled.print(fHumidity);
    oled.print("%");

    oled.sendBuffer();
}

// -------------------- Điều khiển đèn LED --------------------
void controlTrafficLights() {
    static unsigned long lastTimer = 0;
    static int currentLed = 0;
    static const int ledPin[3] = {gPIN, yPIN, rPIN};

    if (yellowBlinkMode) {
        if (!IsReady(lastTimer, 500)) return;
        digitalWrite(gPIN, LOW);
        digitalWrite(rPIN, LOW);
        digitalWrite(yPIN, !digitalRead(yPIN));  // Nhấp nháy đèn vàng
        return;
    }

    if (!IsReady(lastTimer, 1000)) return;
    int prevLed = (currentLed + 2) % 3;
    digitalWrite(ledPin[prevLed], LOW);
    digitalWrite(ledPin[currentLed], HIGH);
    currentLed = (currentLed + 1) % 3;
}

// -------------------- Hiển thị thời gian chạy lên Blynk --------------------
void sendUptime() {
    unsigned long uptime = millis() / 1000;
    Blynk.virtualWrite(V0, uptime);
}

// -------------------- Nhận lệnh từ Blynk --------------------
BLYNK_WRITE(V3) {
    int value = param.asInt();
    yellowBlinkMode = (value == 1);
    if (yellowBlinkMode) {
        digitalWrite(gPIN, LOW);
        digitalWrite(rPIN, LOW);
        digitalWrite(yPIN, HIGH);
    } else {
        digitalWrite(yPIN, LOW);
    }
}

// -------------------- Setup --------------------
void setup() {
    Serial.begin(115200);
    pinMode(gPIN, OUTPUT);
    pinMode(yPIN, OUTPUT);
    pinMode(rPIN, OUTPUT);
    
    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);
    digitalWrite(rPIN, LOW);

    Wire.begin(OLED_SDA, OLED_SCL);
    oled.begin();
    oled.clearBuffer();
    
    oled.setFont(u8g2_font_unifont_t_vietnamese1);
    oled.drawUTF8(0, 14, "Welcom to chanel");
    oled.sendBuffer();

    // Kết nối WiFi và Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Thiết lập timer để gửi dữ liệu lên Blynk mỗi 2 giây
    timer.setInterval(2000L, updateData);
    timer.setInterval(1000L, sendUptime);

    startMillis = millis();
}

// -------------------- Loop --------------------
void loop() {
    Blynk.run();
    timer.run();
    controlTrafficLights();
}
