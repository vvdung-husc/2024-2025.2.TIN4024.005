#define BLYNK_TEMPLATE_ID "TMPL6QbA18q0b"
#define BLYNK_TEMPLATE_NAME "ESP32trafficBlynk"
#define BLYNK_AUTH_TOKEN "1cD7lZoj2GEvbVJizPJxEa04uVMtTRfY"

#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <WiFi.h>            // Thư viện WiFi cho ESP32
#include <BlynkSimpleEsp32.h> // Thư viện Blynk cho ESP32

// Thông tin kết nối WiFi và Blynk
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST"; // Nếu chạy trên Wokwi
char pass[] = ""; // Không có mật khẩu

// Chân kết nối phần cứng
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define CLK 18
#define DIO 19

// Khởi tạo đối tượng
TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT22);

// Biến trạng thái
bool ledState = false;
unsigned long startTime = 0;
unsigned long elapsedTime = 0;
bool buttonPressed = false;

void setup() {
    Serial.begin(115200);
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    
    display.setBrightness(7);
    dht.begin();

    // Kết nối WiFi
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Kết nối Blynk
    Blynk.begin(auth, ssid, pass);
}

// Xử lý nút nhấn từ ứng dụng Blynk
BLYNK_WRITE(V2) {
    int buttonValue = param.asInt(); // Nhận giá trị từ ứng dụng
    if (buttonValue == 1) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        Serial.println(ledState ? "LED ON" : "LED OFF");

        if (ledState) {
            startTime = millis(); // Ghi lại thời gian bật LED
            display.showNumberDec(0); // Hiển thị "0" khi bật
        } else {
            elapsedTime = millis() - startTime;
            display.clear();
        }
    }
}

void loop() {
    Blynk.run(); // Chạy Blynk

    // Xử lý nút nhấn vật lý
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (!buttonPressed) {
            ledState = !ledState;
            buttonPressed = true;
            digitalWrite(LED_PIN, ledState);

            if (ledState) {
                startTime = millis();
                Serial.println("LED ON");
                display.showNumberDec(0);
            } else {
                elapsedTime = millis() - startTime;
                Serial.println("LED OFF");
                display.clear();
            }
        }
    } else {
        buttonPressed = false;
    }

    // Hiển thị thời gian LED sáng trên màn hình 7 đoạn
    if (ledState) {
        static unsigned long lastUpdate = 0;
        if (millis() - lastUpdate >= 1000) {
            lastUpdate = millis();
            unsigned long seconds = (millis() - startTime) / 1000;
            display.showNumberDec(seconds);
            Blynk.virtualWrite(V6, seconds); // Gửi thời gian sáng LED lên Blynk
        }
    }

    // Đọc cảm biến DHT22 mỗi 2 giây và gửi lên Blynk
    static unsigned long lastDHTRead = 0;
    if (millis() - lastDHTRead > 2000) {
        lastDHTRead = millis();
        float temp = dht.readTemperature();
        float hum = dht.readHumidity();

        if (!isnan(temp) && !isnan(hum)) {
            Serial.print("Temp: "); Serial.print(temp); Serial.print("C, ");
            Serial.print("Humidity: "); Serial.print(hum); Serial.println("%");

            Blynk.virtualWrite(V4, temp);
            Blynk.virtualWrite(V5, hum);
        } else {
            Serial.println("Failed to read from DHT sensor!");
        }
    }
}
