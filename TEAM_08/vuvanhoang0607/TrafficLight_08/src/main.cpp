#include <Arduino.h>
#include <TM1637Display.h>

#define RED_LED 27
#define YELLOW_LED 26
#define GREEN_LED 25
#define LDR_PIN 13
#define CLK 18
#define DIO 19
#define BUTTON_PIN 23
#define RIGHT_LED 21 // Đèn bên phải

TM1637Display display(CLK, DIO);
bool isScreenOn = true; // Trạng thái màn hình

void checkButton() {
    if (digitalRead(BUTTON_PIN) == LOW) { // Nút được nhấn
        isScreenOn = false;  // Tắt màn hình
        display.setBrightness(0);
        display.clear(); // Xoá màn hình
        digitalWrite(RIGHT_LED, HIGH); // Bật đèn bên phải
    } else {
        isScreenOn = true;  // Bật lại màn hình
        display.setBrightness(7);
        digitalWrite(RIGHT_LED, LOW); // Tắt đèn bên phải
    }
}

void setLight(int red, int yellow, int green, int time) {
    digitalWrite(RED_LED, red);
    digitalWrite(YELLOW_LED, yellow);
    digitalWrite(GREEN_LED, green);

    unsigned long startTime = millis();
    while (millis() - startTime < time * 1000) {
        checkButton(); // Kiểm tra nút nhấn liên tục
        int lightLevel = analogRead(LDR_PIN);

        // Nếu trời tối thì chuyển sang chế độ nhấp nháy
        if (lightLevel < 1000) return;

        if (isScreenOn) {
            int remainingTime = time - (millis() - startTime) / 1000;
            display.showNumberDec(remainingTime, true);
        } else {
            display.clear(); // Xoá màn hình khi tắt
        }
        delay(200);
    }
}

void nightMode() {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);

    while (analogRead(LDR_PIN) < 1000) { // Lặp khi trời tối
        checkButton(); // Đảm bảo nút vẫn hoạt động

        // Hiển thị số 0 khi đèn vàng nhấp nháy
        if (isScreenOn) {
            display.showNumberDec(0, true);
        } else {
            display.clear();
        }

        digitalWrite(YELLOW_LED, HIGH); // Bật đèn vàng
        delay(500);
        digitalWrite(YELLOW_LED, LOW); // Tắt đèn vàng
        delay(500);
    }

    display.clear(); // Xóa màn hình khi thoát chế độ đêm
}

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RIGHT_LED, OUTPUT);
    display.setBrightness(7);
    Serial.begin(115200); // Debug giá trị LDR
}

void loop() {
    checkButton(); // Kiểm tra nút mỗi vòng lặp

    int lightLevel = analogRead(LDR_PIN);
    Serial.println(lightLevel); // In giá trị LDR để kiểm tra

    if (lightLevel < 1000) {
        nightMode(); // Chế độ đèn vàng nhấp nháy
    } else {
        setLight(1, 0, 0, 10); // Đèn đỏ 10s
        setLight(0, 1, 0, 3);  // Đèn vàng 3s
        setLight(0, 0, 1, 8);  // Đèn xanh 8s
    }
}
