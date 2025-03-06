#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa các chân kết nối
#define LED_RED 19       
#define LED_YELLOW 18    
#define LED_GREEN 5      
#define BUTTON 12        
#define CLK 27           
#define DIO 14           

// Khởi tạo màn hình TM1637
TM1637Display display(CLK, DIO);

// Biến kiểm soát hiển thị màn hình
bool displayActive = true; 

// Biến thời gian để xử lý millis()
unsigned long previousMillis = 0;
const long interval = 1000;  // Đếm lùi mỗi 1 giây

// Thời gian chờ khi chuyển trạng thái (0.5s)
const int transitionDelay = 500; 
bool transitioning = false;  // Đánh dấu trạng thái chuyển đèn
unsigned long transitionStartTime = 0;

// Biến trạng thái đèn giao thông và bộ đếm ngược
int state = 0;      // 0: Đèn xanh, 1: Đèn vàng, 2: Đèn đỏ
int countdown = 5; // Bộ đếm ngược

// Biến debounce để tránh lỗi nhấn nút nhiều lần
volatile unsigned long lastInterruptTime = 0;

// Hàm xử lý nút nhấn bằng ngắt ngoài
void IRAM_ATTR handleButtonPress() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 300) { // Kiểm tra debounce (300ms)
        displayActive = !displayActive;  // Đảo trạng thái hiển thị màn hình
        lastInterruptTime = interruptTime;
    }
}

void setup() {
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);
    
    attachInterrupt(digitalPinToInterrupt(BUTTON), handleButtonPress, FALLING);
    
    display.setBrightness(7);
}

void loop() {
    unsigned long currentMillis = millis();

    // Nếu đang trong giai đoạn chuyển đèn, chờ 0.5s rồi mới bật đèn tiếp theo
    if (transitioning) {
        if (currentMillis - transitionStartTime >= transitionDelay) {
            transitioning = false; // Kết thúc quá trình chuyển
        } else {

            return; // Đang chờ, không làm gì cả
        }
    }

    // Kiểm tra xem đã đến lúc giảm bộ đếm ngược chưa
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;  
        countdown--;  

        if (countdown == 0) {
            transitioning = true; // Bắt đầu giai đoạn chuyển đèn
            transitionStartTime = millis();
            // Tắt màn hình trong khoảng nghỉ 0.5s
            display.clear();
            // Chuyển trạng thái đèn
            state = (state + 1) % 3; 
            countdown = (state == 1) ? 2 : 5; // Đèn vàng: 3s, còn lại: 10s
        }
    }

    // Nếu không trong giai đoạn chuyển đèn, bật đèn theo trạng thái hiện tại
    if (!transitioning) {
        switch (state) {
            case 0: digitalWrite(LED_RED, LOW); digitalWrite(LED_YELLOW, LOW); digitalWrite(LED_GREEN, HIGH); break;
            case 1: digitalWrite(LED_RED, LOW); digitalWrite(LED_YELLOW, HIGH); digitalWrite(LED_GREEN, LOW); break;
            case 2: digitalWrite(LED_RED, HIGH); digitalWrite(LED_YELLOW, LOW); digitalWrite(LED_GREEN, LOW); break; 
        }
    }

    // Kiểm soát hiển thị màn hình
    if (displayActive) {
        display.showNumberDec(countdown); // Hiển thị số đếm
    } else {
        display.clear(); // Tắt màn hình nhưng đèn vẫn chạy
    }
}
