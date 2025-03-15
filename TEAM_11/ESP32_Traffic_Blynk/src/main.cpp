
#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>


// Lê Si Thuận
#define BLYNK_TEMPLATE_ID "TMPL6FwaegGhO"
#define BLYNK_TEMPLATE_NAME "ThuanBlynkLed"
#define BLYNK_AUTH_TOKEN "mRckA9umZwNmIlFYk-3X3pei9hyJgbsO"


char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Khai báo các chân
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define DHT_TYPE DHT22
#define CLK_PIN 18
#define DIO_PIN 19
#define LIGHT_SENSOR_PIN 34
#define WARNING_LED_PIN 5

// Đối tượng cảm biến và màn hình
DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK_PIN, DIO_PIN);

// Biến trạng thái
bool ledState = false;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
unsigned long sensorLastReadTime = 0;
const unsigned long sensorInterval = 2000;
float temperature = 0.0;
float humidity = 0.0;
unsigned long lastUptimeSendTime = 0;
const unsigned long uptimeInterval = 1000;
int lightThreshold = 1000;

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(WARNING_LED_PIN, OUTPUT);
    pinMode(LIGHT_SENSOR_PIN, INPUT);
    
    display.setBrightness(7);
    display.showNumberDec(0, true);
    
    dht.begin();
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("Connected to Blynk");

    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    Blynk.virtualWrite(V1, ledState);
    Blynk.virtualWrite(V4, lightThreshold);
}

// Cập nhật thời gian hoạt động lên màn hình LED
void displayTime(unsigned long seconds) {
    int minutes = seconds / 60;
    int secs = seconds % 60;
    int timeToShow = (minutes * 100) + secs;
    display.showNumberDecEx(timeToShow, 0x40, true);
}

void loop() {
    Blynk.run();
    unsigned long currentMillis = millis();
    
    // Xử lý nút nhấn bật/tắt đèn
    int reading = digitalRead(BUTTON_PIN);
    if (reading != lastButtonState) {
        lastDebounceTime = currentMillis;
    }
    if ((currentMillis - lastDebounceTime) > debounceDelay) {
        if (reading == LOW && lastButtonState == HIGH) {
            ledState = !ledState;  
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            Blynk.virtualWrite(V1, ledState);
        }
    }
    lastButtonState = reading;

    // Đọc cảm biến và gửi dữ liệu lên Blynk
    if (currentMillis - sensorLastReadTime >= sensorInterval) {
        sensorLastReadTime = currentMillis;
        float newTemperature = dht.readTemperature();
        float newHumidity = dht.readHumidity();
        int lightLevel = analogRead(LIGHT_SENSOR_PIN);
        
        if (!isnan(newTemperature) && !isnan(newHumidity)) {
            temperature = newTemperature;
            humidity = newHumidity;
            Serial.print("Nhiệt độ: ");
            Serial.print(temperature);
            Serial.print(" °C, Độ ẩm: ");
            Serial.print(humidity);
            Serial.println(" %");
            
            Blynk.virtualWrite(V0, temperature);
            Blynk.virtualWrite(V2, humidity);
        } else {
            Serial.println("Lỗi đọc dữ liệu từ DHT22!");
        }
        
        Serial.print("Mức ánh sáng: ");
        Serial.println(lightLevel);
        Blynk.virtualWrite(V4, lightLevel);

        // Điều khiển đèn cảnh báo nếu trời tối
        if (lightLevel < lightThreshold) {
            digitalWrite(WARNING_LED_PIN, HIGH);
        } else {
            digitalWrite(WARNING_LED_PIN, LOW);
        }
    }

    // Hiển thị thời gian hoạt động
    if (currentMillis - lastUptimeSendTime >= uptimeInterval) {
        lastUptimeSendTime = currentMillis;
        unsigned long uptimeSeconds = currentMillis / 1000;
        Blynk.virtualWrite(V1, uptimeSeconds);
        displayTime(uptimeSeconds);
    }
}

// Cập nhật trạng thái LED từ Blynk
BLYNK_WRITE(V1) {
    ledState = param.asInt();
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    Serial.print("Cập nhật LED từ Blynk: ");
    Serial.println(ledState ? "ON" : "OFF");
}

// Cập nhật ngưỡng trời tối từ Blynk
BLYNK_WRITE(V4) {
    lightThreshold = param.asInt();
    Serial.print("Ngưỡng trời tối mới: ");
    Serial.println(lightThreshold);
}
