#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 13
#define OLED_SCL 12
#define DHTPIN 16
#define DHTTYPE DHT22

#define LED_BLUE 15
#define LED_YELLOW 2
#define LED_RED 4

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

void updateDisplay(float temp, float hum);
void updateLED(float temp);

void setup() {
    Serial.begin(9600);
    
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    dht.begin();
    
    Wire.begin(OLED_SDA, OLED_SCL);

    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Failed to start SSD1306 OLED"));
        while (1);
    }

    delay(2000);
    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 2);
    oled.println("   IOT\n Welcome!");
    oled.display();
}

void loop() {
    delay(1000);
    
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    updateDisplay(t, h);
    updateLED(t);
}

void updateDisplay(float temp, float hum) {
    oled.clearDisplay();  // Xóa màn hình đúng cách
    
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print("Temp: ");
    
    if (temp < 0.0) {
        oled.print("Too COOL");
    } else if (temp < 40.0) {
        oled.print("Normal");
    } else {
        oled.print("Too HOT");
    }

    oled.setTextSize(2);
    oled.setCursor(0, 10);
    oled.print(temp);
    oled.print(" C");

    oled.setTextSize(1);
    oled.setCursor(0, 35);
    oled.print("Humidity: ");
    oled.setTextSize(2);
    oled.setCursor(0, 45);
    oled.print(hum);
    oled.print(" %");

    oled.display();
}

void updateLED(float temp) {
    int led = LED_BLUE;

    if (temp < 0.0) {
        led = LED_YELLOW;
    } else if (temp >= 40.0) {
        led = LED_RED;
    }

    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
}
