#include <Arduino.h>
#include <TM1637Display.h>


#define GREEN_LED 32
#define YELLOW_LED 25
#define RED_LED 26


#define BUTTON_PIN 18


#define LDR_PIN 13


#define TM1637_CLK 4
#define TM1637_DIO 16

// Khởi tạo màn hình TM1637
TM1637Display display(TM1637_CLK, TM1637_DIO);


int trafficLights[] = {GREEN_LED, YELLOW_LED, RED_LED};  
int lightDurations[] = {3, 1, 2};  


unsigned long lastUpdateTime = 0;
unsigned long lastButtonPress = 0; 
unsigned long lastLDRRead = 0; 
int countdown = 0;
int currentLightIndex = 0;
int lastDisplayedCountdown = -1; 
int lastBrightness = -1; 


bool isDisplayOn = false;


void IRAM_ATTR toggleDisplay() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonPress > 300) { 
    isDisplayOn = !isDisplayOn;
    lastButtonPress = currentTime;
    
   
    Serial.print("Đồng hồ đếm giây: ");
    Serial.println(isDisplayOn ? "BẬT" : "TẮT");
  }
}

void setup() {
  // Cấu hình các đèn LED làm OUTPUT và PWM
  for (int i = 0; i < 3; i++) {
    pinMode(trafficLights[i], OUTPUT);
    digitalWrite(trafficLights[i], LOW);  
    ledcSetup(i, 5000, 8); 
    ledcAttachPin(trafficLights[i], i); 
  }

  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), toggleDisplay, FALLING);

 
  pinMode(LDR_PIN, INPUT);

  
  Serial.begin(115200);

  
  display.setBrightness(7);
  display.showNumberDec(0, false);

 
  ledcWrite(currentLightIndex, 255);  
  countdown = lightDurations[currentLightIndex];
}

void loop() {
  unsigned long currentTime = millis();


  if (currentTime - lastUpdateTime >= 1000) {
    lastUpdateTime = currentTime;

    if (countdown == 0) {
      
      for (int i = 0; i < 3; i++) {
        ledcWrite(i, 0); 
      }

      
      currentLightIndex = (currentLightIndex + 1) % 3;
      countdown = lightDurations[currentLightIndex];

     
      ledcWrite(currentLightIndex, 255); 
    } else {
      countdown--;  
    }

    // Chỉ cập nhật màn hình TM1637 nếu giá trị thay đổi
    if (isDisplayOn && countdown != lastDisplayedCountdown) {
      display.showNumberDec(countdown, false);
      lastDisplayedCountdown = countdown;
    } else if (!isDisplayOn) {
      display.clear();
    }

    
    Serial.print("Đèn còn: ");
    Serial.print(countdown);
    Serial.println(" giây");
  }

  // Đọc giá trị cảm biến ánh sáng mỗi 200ms (tránh làm chậm loop)
  if (currentTime - lastLDRRead >= 200) {
    lastLDRRead = currentTime;
    
    int ldrValue = analogRead(LDR_PIN);
    
    //  Đảo ngược độ sáng (Lúc sáng -> Đèn mờ, Lúc tối -> Đèn sáng)
    int ledBrightness = map(ldrValue, 0, 4095, 255, 50);

    // Chỉ cập nhật độ sáng nếu giá trị thay đổi để tránh spam Serial
    if (ledBrightness != lastBrightness) {
      Serial.print("Ánh sáng môi trường: ");
      Serial.print(ldrValue);
      Serial.print(" | Độ sáng LED: ");
      Serial.println(ledBrightness);
      lastBrightness = ledBrightness;
    }

    // Điều chỉnh độ sáng của LED theo ánh sáng môi trường
    ledcWrite(currentLightIndex, ledBrightness);
  }
}
