#include <Arduino.h>
#include <TM1637Display.h>

// Dương Duy Khanh
//#define BLYNK_TEMPLATE_ID "TMPL6rW4m1S4J"
//#define BLYNK_TEMPLATE_NAME "khanh"
//#define BLYNK_AUTH_TOKEN "ejtyvb4NArA0Ek1L4KVNv5pZDDcf81PB"

<<<<<<< HEAD
//Tôn Huyền Kim Khánh
// #define BLYNK_TEMPLATE_ID "TMPL6Mtg-cw9S"
// #define BLYNK_TEMPLATE_NAME "ESP32 LED"
// #define BLYNK_AUTH_TOKEN "jNyrijhCFQGHv4WNa5VCQxQpFPxd_l8B"

=======
>>>>>>> 48359288a8ded251b72e31671b1306e4375c2320
// Hoàng Thanh Nhã
#define BLYNK_TEMPLATE_ID "TMPL6WdT9pprT"
#define BLYNK_TEMPLATE_NAME "TrafficBlynk"
#define BLYNK_AUTH_TOKEN   "Eg73I3k1TN8KpG9DKqM8dqpWl1ShcgpB"
//Tôn Huyền Kim Khánh
// #define BLYNK_TEMPLATE_ID "TMPL6Mtg-cw9S"
// #define BLYNK_TEMPLATE_NAME "ESP32 LED"
// #define BLYNK_AUTH_TOKEN "jNyrijhCFQGHv4WNa5VCQxQpFPxd_l8B"
// Nguyễn Khánh Linh
// #define BLYNK_TEMPLATE_ID "TMPL6CbYUwJFN"
// #define BLYNK_TEMPLATE_NAME "ESP32 BLYNK LED"
// #define BLYNK_AUTH_TOKEN "C6_0VoXx3puIy5vz3fCEsGo1dBWE7oXF"
#include <WiFi.h>
 #include <WiFiClient.h>
 #include <BlynkSimpleEsp32.h>
 #include <DHT.h>
 
 // Thông tin WiFi
 char ssid[] = "Wokwi-GUEST";
 char pass[] = "";
 
 // Chân kết nối phần cứng
 #define CLK         18
 #define DIO         19
 #define DHT_PIN     16
 #define DHT_TYPE    DHT22  
 #define LDR_PIN     34
 #define RED_LED     27
 #define YELLOW_LED  26
 #define GREEN_LED   25
 #define BUTTON_PIN  23
 #define BLUE_LED    21  // Đèn xanh dương
 
 // Thời gian thay đổi đèn giao thông (ms)
 ulong trafficDurations[] = {10000, 10000, 3000}; // Đỏ 10s, Xanh 10s, Vàng 3s
 
 // Biến toàn cục
 ulong lastChange     = 0; 
 int   trafficState   = 0; 
 bool  isDark         = false;
 int   ldrThreshold   = 2000; 
 bool  isDisplayOn    = true;
 
 TM1637Display display(CLK, DIO);
 DHT dht(DHT_PIN, DHT_TYPE);
 
 // Uptime
 unsigned long startTime = 0;
 
 // Biến lưu thời gian còn lại (ms) khi sang tối
 long timeLeftInState = 0;
 
 /* --------------------------------------------------
    updateTrafficLights
    -------------------------------------------------- */
 void updateTrafficLights() {
   ulong currentMillis = millis();
 
   if (isDark) {
     // Trời tối -> Đèn vàng, không đếm thời gian
     digitalWrite(RED_LED, LOW);
     digitalWrite(GREEN_LED, LOW);
     digitalWrite(YELLOW_LED, HIGH);
     display.clear();
     return;
   }
 
   // Trời sáng -> Tiếp tục đèn giao thông
   if (currentMillis - lastChange >= trafficDurations[trafficState]) {
     lastChange = currentMillis;
     trafficState = (trafficState + 1) % 3;
 
     digitalWrite(RED_LED,    trafficState == 0);
     digitalWrite(GREEN_LED,  trafficState == 1);
     digitalWrite(YELLOW_LED, trafficState == 2);
   }
 
   int remainingTime = (trafficDurations[trafficState] - (currentMillis - lastChange)) / 1000;
 
   if (isDisplayOn) {
     display.showNumberDec(remainingTime);
   } else {
     display.clear();
   }
 }
 
 /* --------------------------------------------------
    updateTemperatureHumidity
    -------------------------------------------------- */
 void updateTemperatureHumidity() {
   static ulong lastUpdate = 0;
   if (millis() - lastUpdate < 2000) return;
   lastUpdate = millis();
 
   float temp = dht.readTemperature();
   float hum  = dht.readHumidity();
   
   if (!isnan(temp) && !isnan(hum)) {
     Blynk.virtualWrite(V2, temp);
     Blynk.virtualWrite(V3, hum);
     Serial.printf("📡 Temp: %.2f°C | Hum: %.2f%%\r", temp, hum);
   } else {
     Serial.println("⚠️ Lỗi đọc cảm biến DHT22!");
   }
 }
 
 /* --------------------------------------------------
    updateLightThreshold (LDR)
    -------------------------------------------------- */
 void updateLightThreshold() {
   static ulong lastUpdate = 0;
   if (millis() - lastUpdate < 500) return;
   lastUpdate = millis();
 
   int lightValue = analogRead(LDR_PIN);
   Blynk.virtualWrite(V4, lightValue);
   Serial.printf("💡 Độ sáng hiện tại: %d\r", lightValue);
 
   if (lightValue < ldrThreshold) {
     // TỐI
     if (!isDark) {
       // Sáng -> Tối
       long leftover = (long)trafficDurations[trafficState] - (millis() - lastChange);
       if (leftover < 0) leftover = 0;
 
       // Thêm đệm 1 giây để tránh leftover = 0
       if (leftover < 1000) {
         leftover = 1000;
       }
 
       timeLeftInState = leftover;
 
       isDark = true;
       Serial.println("🌙 Trời tối -> Bật đèn vàng!");
     }
   } else {
     // SÁNG
     if (isDark) {
       // Tối -> Sáng
       isDark = false;
 
       // Khôi phục
       lastChange = millis() - (trafficDurations[trafficState] - timeLeftInState);
 
       Serial.println("☀️ Trời sáng -> Quay lại đèn giao thông!");
     }
   }
 }
 
 /* --------------------------------------------------
    checkButton
    -------------------------------------------------- */
 void checkButton() {
   static bool lastButtonState = HIGH;
   bool buttonState = digitalRead(BUTTON_PIN);
 
   if (buttonState == LOW && lastButtonState == HIGH) {
     delay(50);
     if (digitalRead(BUTTON_PIN) == LOW) {
       isDisplayOn = !isDisplayOn;
       Serial.printf("🎛 Màn hình: %s\r", isDisplayOn ? "BẬT" : "TẮT");
 
       if (!isDisplayOn) {
         display.clear();
       }
       Blynk.virtualWrite(V1, isDisplayOn);
       digitalWrite(BLUE_LED, isDisplayOn ? HIGH : LOW);
     }
   }
   
   lastButtonState = buttonState;
 }
 
 /* --------------------------------------------------
    BLYNK_WRITE(V1)
    -------------------------------------------------- */
 BLYNK_WRITE(V1) {
   bool newState = param.asInt();
   if (newState != isDisplayOn) {
     isDisplayOn = newState;
     Serial.printf("🎛 Điều khiển từ Blynk - Màn hình: %s\r", isDisplayOn ? "BẬT" : "TẮT");
 
     if (!isDisplayOn) {
       display.clear();
     }
     digitalWrite(BLUE_LED, isDisplayOn ? HIGH : LOW);
   }
 }
 
 /* --------------------------------------------------
    updateUptime
    -------------------------------------------------- */
 void updateUptime() {
   unsigned long uptimeSec = (millis() - startTime) / 1000;
   Blynk.virtualWrite(V0, uptimeSec);
 }
 
 /* --------------------------------------------------
    Setup
    -------------------------------------------------- */
 void setup() {
   Serial.begin(115200);
   
   pinMode(RED_LED, OUTPUT);
   pinMode(YELLOW_LED, OUTPUT);
   pinMode(GREEN_LED, OUTPUT);
   pinMode(BUTTON_PIN, INPUT_PULLUP);
   pinMode(BLUE_LED, OUTPUT);
 
   display.setBrightness(0x0f);
   display.clear();
 
   dht.begin();
 
   startTime = millis();
 
   Serial.printf("🔗 Đang kết nối WiFi: %s\r", ssid);
   Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
   if (WiFi.status() == WL_CONNECTED) {
     Serial.println("✅ WiFi đã kết nối!");
   } else {
     Serial.println("❌ Kết nối WiFi thất bại!");
   }
 
   digitalWrite(RED_LED, LOW);
   digitalWrite(YELLOW_LED, LOW);
   digitalWrite(GREEN_LED, LOW);
   digitalWrite(BLUE_LED, isDisplayOn ? HIGH : LOW);
 }
 
 /* --------------------------------------------------
    Loop
    -------------------------------------------------- */
 void loop() {
   Blynk.run();
   checkButton();
   updateLightThreshold();
   updateTrafficLights();
   updateTemperatureHumidity();
   updateUptime();
 
   if (WiFi.status() != WL_CONNECTED) {
     Serial.println("🚨 Mất kết nối WiFi, thử kết nối lại...");
     WiFi.begin(ssid, pass);
   }
 }