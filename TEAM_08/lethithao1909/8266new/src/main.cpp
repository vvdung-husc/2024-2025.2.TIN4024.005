#include <Arduino.h>

#define LED1 15  // Chân LED 1
#define LED2 16  // Chân LED 2
#define LED3 17  // Chân LED 3
#define TEMP_SENSOR_PIN 34  // Chân cảm biến nhiệt độ (ESP32 ADC)

// Thiết lập
void setup() {
    Serial.begin(115200); // Bật Serial Monitor
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
}

// Vòng lặp chính
void loop() {
    // Đọc nhiệt độ
    int rawValue = analogRead(TEMP_SENSOR_PIN);
    float voltage = rawValue * (3.3 / 4095.0); // Chuyển đổi giá trị ADC (ESP32)
    float temperature = voltage * 100.0; // LM35: 10mV = 1°C

    // Hiển thị nhiệt độ lên Serial Monitor
    Serial.print("Nhiệt độ: ");
    Serial.print(temperature);
    Serial.println(" °C");

    // Nháy đèn lần lượt
    digitalWrite(LED1, HIGH);
    delay(500);
    digitalWrite(LED1, LOW);

    digitalWrite(LED2, HIGH);
    delay(500);
    digitalWrite(LED2, LOW);

    digitalWrite(LED3, HIGH);
    delay(500);
    digitalWrite(LED3, LOW);
}
