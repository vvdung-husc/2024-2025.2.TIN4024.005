// #include <Arduino.h>
// #include <Adafruit_Sensor.h>
// #include <DHT.h>
// #include <Wire.h>
// #include <U8g2lib.h>

// // Định nghĩa thông tin Blynk
// #define BLYNK_TEMPLATE_ID "TMPL6UtUU_5WZ"
// #define BLYNK_TEMPLATE_NAME "ESP8266Project"
// #define BLYNK_AUTH_TOKEN "p_B7I5wU_xTCi3owpkhbeklgLbr7r-8R"

// #include <ESP8266WiFi.h>
// #include <BlynkSimpleEsp8266.h>
// #include <UniversalTelegramBot.h> // Thêm thư viện Telegram
// #include <WiFiClientSecure.h>

// // Thông tin kết nối WiFi
// char ssid[] = "CNTT-MMT";
// char pass[] = "13572468";

// // Thông tin Telegram Bot
// #define BOT_TOKEN "7499637616:AAGVz985ubbTrkQlWYRrJMTNC70be_QVP7k"
// #define GROUP_ID "-4698442702"

// WiFiClientSecure client;
// UniversalTelegramBot bot(BOT_TOKEN, client);

// // Định nghĩa các chân cho đèn giao thông
// #define gPIN 15  // Đèn xanh (D8/GPIO 15)
// #define yPIN 2   // Đèn vàng (D4/GPIO 2)
// #define rPIN 5   // Đèn đỏ (D1/GPIO 5)

// // Định nghĩa chân và loại cảm biến DHT
// #define dhtPIN 16     // Chân số kết nối với cảm biến DHT (D0/GPIO 16)
// #define dhtTYPE DHT11 // Loại cảm biến DHT 11

// // Định nghĩa chân I2C cho OLED
// #define OLED_SDA 13   // D7/GPIO 13
// #define OLED_SCL 12   // D6/GPIO 12

// // Khởi tạo màn hình OLED SH1106
// U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// // Khởi tạo cảm biến DHT
// DHT dht(dhtPIN, dhtTYPE);

// // Biến cho đèn giao thông
// int currentState = 0;           // Trạng thái hiện tại: 0 - Xanh, 1 - Vàng, 2 - Đỏ
// unsigned long previousMillis = 0; // Thời gian trước đó (dùng để tính thời gian)
// unsigned long remainingTime = 0;  // Thời gian còn lại của trạng thái hiện tại
// unsigned long greenTime = 15000;  // Thời gian đèn xanh: 15 giây
// unsigned long yellowTime = 2000;  // Thời gian đèn vàng: 2 giây
// unsigned long redTime = 5000;     // Thời gian đèn đỏ: 5 giây
// bool yellowBlinkMode = false;     // Chế độ nhấp nháy đèn vàng
// bool trafficOff = false;          // Cờ để kiểm soát trạng thái đèn giao thông
// bool trafficOffByTelegram = false; // Cờ để kiểm tra xem đèn bị tắt bởi Telegram hay không

// float fHumidity = 0.0;    // Biến lưu độ ẩm
// float fTemperature = 0.0; // Biến lưu nhiệt độ

// // Hàm tạo giá trị ngẫu nhiên cho nhiệt độ và độ ẩm
// float randomFloat(float min, float max) {
//   return min + (float)random(0, 1000) / 1000.0 * (max - min);
// }

// // Hàm kiểm tra xem đã đến thời gian thực hiện hành động chưa
// bool IsReady(unsigned long &lastTimer, unsigned long interval) {
//   unsigned long currentTime = millis();
//   if (currentTime - lastTimer >= interval) {
//     lastTimer = currentTime;
//     return true;
//   }
//   return false;
// }

// // Hàm kiểm tra thời gian hiển thị màn hình chào mừng
// bool WelcomeDisplayTimeout(unsigned long msSleep = 5000) {
//   static unsigned long lastTimer = 0;
//   static bool bDone = false;
//   if (bDone) return true;
//   if (!IsReady(lastTimer, msSleep)) return false;
//   bDone = true;
//   return bDone;
// }

// // Hàm cập nhật hiển thị trên màn hình OLED
// void updateDisplay() {
//   static unsigned long lastTimer = 0;
//   if (!IsReady(lastTimer, 1000)) return;  // Cập nhật màn hình mỗi 1 giây

//   oled.clearBuffer();
//   oled.setFont(u8g2_font_unifont_t_vietnamese2);
//   String s;

//   // Hiển thị trạng thái đèn giao thông
//   if (trafficOff) {
//     s = "Den tat: Lenh Telegram"; // Hiển thị nếu đèn bị tắt bởi Telegram
//   } else if (yellowBlinkMode) {
//     s = "Den vang: Nhap nhay"; // Hiển thị thông báo nếu ở chế độ nhấp nháy
//   } else {
//     int secondsLeft = remainingTime / 1000; // Tính thời gian còn lại (giây)
//     if (currentState == 0) s = "Den xanh: " + String(secondsLeft) + "s";
//     else if (currentState == 1) s = "Den vang: " + String(secondsLeft) + "s";
//     else s = "Den do: " + String(secondsLeft) + "s";
//     // Gửi thời gian đếm ngược lên Blynk V0
//     Blynk.virtualWrite(V0, secondsLeft);
//   }
//   oled.drawUTF8(0, 12, s.c_str());

//   // Hiển thị nhiệt độ và độ ẩm
//   s = "Nhiet do: " + String(fTemperature, 1) + "C";
//   oled.drawUTF8(0, 48, s.c_str());
//   s = "Do am: " + String(fHumidity, 1) + "%";
//   oled.drawUTF8(0, 60, s.c_str());
//   oled.sendBuffer();
// }

// // Hàm điều khiển đèn giao thông
// void TrafficLightMode() {
//   if (yellowBlinkMode || trafficOff) return; // Không chạy nếu ở chế độ nhấp nháy hoặc đèn bị tắt

//   unsigned long currentMillis = millis();
//   unsigned long interval = (currentState == 0) ? greenTime : (currentState == 1) ? yellowTime : redTime;

//   // Tính thời gian còn lại
//   remainingTime = interval - (currentMillis - previousMillis);

//   if (currentMillis - previousMillis >= interval) {
//     previousMillis = currentMillis;
//     digitalWrite(rPIN, LOW);
//     digitalWrite(gPIN, LOW);
//     digitalWrite(yPIN, LOW);

//     switch (currentState) {
//       case 0:  // Xanh -> Vàng
//         digitalWrite(yPIN, HIGH);
//         currentState = 1;
//         break;
//       case 1:  // Vàng -> Đỏ
//         digitalWrite(rPIN, HIGH);
//         currentState = 2;
//         break;
//       case 2:  // Đỏ -> Xanh
//         digitalWrite(gPIN, HIGH);
//         currentState = 0;
//         break;
//     }
//     remainingTime = interval;  // Đặt lại thời gian còn lại khi chuyển trạng thái
//     Serial.print("[TRAFFIC] ");
//     Serial.println(currentState == 0 ? "Xanh" : currentState == 1 ? "Vang" : "Do");
//   }
// }

// // Hàm điều khiển chế độ nhấp nháy đèn vàng
// void YellowBlinkMode() {
//   static unsigned long lastTimer = 0;
//   static bool yellowState = false;

//   if (!yellowBlinkMode || trafficOff) return; // Không chạy nếu không ở chế độ nhấp nháy hoặc đèn bị tắt

//   if (!IsReady(lastTimer, 1000)) return;

//   digitalWrite(rPIN, LOW);
//   digitalWrite(gPIN, LOW);
//   yellowState = !yellowState;
//   digitalWrite(yPIN, yellowState ? HIGH : LOW);
// }

// // Hàm cập nhật dữ liệu từ cảm biến DHT và gửi thông báo Telegram
// void updateDHT() {
//   static unsigned long lastTimer = 0;
//   if (!IsReady(lastTimer, 15000)) return;  // Cập nhật mỗi 15 giây (15000ms)

//   // Tạo giá trị ngẫu nhiên cho nhiệt độ và độ ẩm
//   float t = randomFloat(-40.0, 80.0);
//   float h = randomFloat(0.0, 100.0);

//   // Biến để kiểm tra xem có thay đổi không
//   bool hasChanged = false;
//   String message = "Cập nhật dữ liệu:\n";

//   if (fTemperature != t) {
//     fTemperature = t;
//     Serial.print("Nhiet do: ");
//     Serial.print(t);
//     Serial.println(" *C");
//     Blynk.virtualWrite(V1, t);
//     message += "🔥 Nhiệt độ: " + String(fTemperature, 1) + "°C\n";
//     hasChanged = true;
//   }

//   if (fHumidity != h) {
//     fHumidity = h;
//     Serial.print("Do am: ");
//     Serial.print(h);
//     Serial.println(" %");
//     Blynk.virtualWrite(V2, h);
//     message += "💦 Độ ẩm: " + String(fHumidity, 1) + "%\n";
//     hasChanged = true;
//   }

//   // Kiểm tra các ngưỡng nguy hiểm và thêm cảnh báo vào tin nhắn
//   if (hasChanged) {
//     // Kiểm tra nhiệt độ
//     if (fTemperature < 10) {
//       message += "🔥 CẢNH BÁO: Nhiệt độ quá thấp (" + String(fTemperature) + "°C)! Nguy cơ hạ thân nhiệt, tê cóng, giảm miễn dịch.\n";
//     } else if (fTemperature >= 10 && fTemperature <= 15) {
//       message += "🔥 CẢNH BÁO: Nhiệt độ thấp (" + String(fTemperature) + "°C)! Cảm giác lạnh, tăng nguy cơ mắc bệnh đường hô hấp.\n";
//     } else if (fTemperature > 25 && fTemperature <= 30) {
//       // Nhiệt độ lý tưởng, không cần cảnh báo
//     } else if (fTemperature > 30 && fTemperature <= 35) {
//       message += "🔥 CẢNH BÁO: Nhiệt độ cao (" + String(fTemperature) + "°C)! Cơ thể có dấu hiệu mất nước, mệt mỏi.\n";
//     } else if (fTemperature > 35 && fTemperature <= 40) {
//       message += "🔥 CẢNH BÁO: Nhiệt độ rất cao (" + String(fTemperature) + "°C)! Nguy cơ sốc nhiệt, chuột rút, say nắng.\n";
//     } else if (fTemperature > 40) {
//       message += "🔥 CẢNH BÁO: Nhiệt độ cực kỳ nguy hiểm (" + String(fTemperature) + "°C)! Nguy cơ suy nội tạng, đột quỵ nhiệt.\n";
//       // Không đặt trafficOff = true nữa, để đèn giao thông vẫn hoạt động
//     }

//     // Kiểm tra độ ẩm
//     if (fHumidity < 30) {
//       message += "💦 CẢNH BÁO: Độ ẩm quá thấp (" + String(fHumidity) + "%)! Da khô, kích ứng mắt, tăng nguy cơ mắc bệnh về hô hấp.\n";
//     } else if (fHumidity >= 40 && fHumidity <= 60) {
//       // Độ ẩm lý tưởng, không cần cảnh báo
//     } else if (fHumidity > 70 && fHumidity <= 80) {
//       message += "💦 CẢNH BÁO: Độ ẩm cao (" + String(fHumidity) + "%)! Tăng nguy cơ nấm mốc, vi khuẩn phát triển, gây bệnh đường hô hấp.\n";
//     } else if (fHumidity > 80) {
//       message += "💦 CẢNH BÁO: Độ ẩm rất cao (" + String(fHumidity) + "%)! Oi bức, khó thở, tăng nguy cơ sốc nhiệt.\n";
//     }

//     // Gửi thông báo qua Telegram
//     bot.sendMessage(GROUP_ID, message, "");
//   }
// }

// // Hàm xử lý tin nhắn từ Telegram
// void handleTelegramMessages() {
//   int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

//   while (numNewMessages) {
//     for (int i = 0; i < numNewMessages; i++) {
//       String chat_id = String(bot.messages[i].chat_id);
//       String text = bot.messages[i].text;

//       if (text == "/traffic_off") {
//         trafficOff = true;
//         trafficOffByTelegram = true; // Đánh dấu rằng đèn bị tắt bởi Telegram
//         digitalWrite(rPIN, LOW);
//         digitalWrite(gPIN, LOW);
//         digitalWrite(yPIN, LOW);
//         bot.sendMessage(chat_id, "Đèn giao thông đã được tắt.", "");
//       } else if (text == "/traffic_on") {
//         trafficOff = false;
//         trafficOffByTelegram = false; // Xóa đánh dấu tắt bởi Telegram
//         currentState = 0; // Đặt lại trạng thái về đèn xanh
//         previousMillis = millis();
//         digitalWrite(gPIN, HIGH);
//         remainingTime = greenTime;
//         bot.sendMessage(chat_id, "Đèn giao thông đã được bật.", "");
//       } else {
//         bot.sendMessage(chat_id, "Dùng lệnh /traffic_off để tắt đèn hoặc /traffic_on để bật đèn.", "");
//       }
//     }
//     numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//   }
// }

// // Hàm xử lý khi nhận giá trị từ Blynk V3 (chế độ nhấp nháy đèn vàng)
// BLYNK_WRITE(V3) {
//   yellowBlinkMode = param.asInt();
//   if (!yellowBlinkMode) {
//     digitalWrite(rPIN, LOW);
//     digitalWrite(gPIN, LOW);
//     digitalWrite(yPIN, LOW);
//     currentState = 0;  // Đặt lại trạng thái về đèn xanh khi tắt chế độ nhấp nháy
//     previousMillis = millis();  // Đặt lại thời gian
//     digitalWrite(gPIN, HIGH);  // Bật đèn xanh ngay lập tức
//     remainingTime = greenTime;  // Đặt lại thời gian còn lại
//     trafficOff = false; // Bật lại đèn giao thông nếu đang bị tắt
//     trafficOffByTelegram = false; // Xóa đánh dấu tắt bởi Telegram
//   }
// }

// // Hàm xử lý khi nhận giá trị từ Blynk V4 (thời gian đèn xanh)
// BLYNK_WRITE(V4) {
//   greenTime = param.asInt() * 1000;
//   if (greenTime < 1000) greenTime = 1000;
//   if (greenTime > 30000) greenTime = 30000;
// }

// // Hàm xử lý khi nhận giá trị từ Blynk V5 (thời gian đèn vàng)
// BLYNK_WRITE(V5) {
//   yellowTime = param.asInt() * 1000;
//   if (yellowTime < 1000) yellowTime = 1000;
//   if (yellowTime > 10000) yellowTime = 10000;
// }

// // Hàm xử lý khi nhận giá trị từ Blynk V6 (thời gian đèn đỏ)
// BLYNK_WRITE(V6) {
//   redTime = param.asInt() * 1000;
//   if (redTime < 1000) redTime = 1000;
//   if (redTime > 30000) redTime = 30000;
// }

// // Hàm thiết lập ban đầu
// void setup() {
//   Serial.begin(115200);

//   // Thiết lập các chân cho đèn giao thông
//   pinMode(gPIN, OUTPUT);
//   pinMode(yPIN, OUTPUT);
//   pinMode(rPIN, OUTPUT);
//   digitalWrite(gPIN, HIGH);  // Bắt đầu với đèn xanh
//   digitalWrite(yPIN, LOW);
//   digitalWrite(rPIN, LOW);

//   dht.begin(); // Khởi động cảm biến DHT

//   // Khởi động giao tiếp I2C cho OLED
//   Wire.begin(OLED_SDA, OLED_SCL);
//   oled.begin();
//   oled.clearBuffer();

//   // Hiển thị thông tin ban đầu trên OLED
//   oled.setFont(u8g2_font_unifont_t_vietnamese1);
//   oled.drawUTF8(0, 14, "Trường ĐHKH");
//   oled.drawUTF8(0, 28, "Khoa CNTT");
//   oled.drawUTF8(0, 42, "Lập trình IoT");
//   oled.sendBuffer();

//   // Kết nối WiFi
//   WiFi.begin(ssid, pass);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("WiFi da ket noi");

//   // Thiết lập Telegram
//   client.setInsecure(); // Bỏ qua kiểm tra chứng chỉ SSL cho ESP8266
//   bot.sendMessage(GROUP_ID, "ESP8266 đã khởi động!", "");

//   // Kết nối với Blynk
//   Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

//   previousMillis = millis();  // Khởi tạo thời gian ban đầu
//   remainingTime = greenTime;  // Khởi tạo thời gian còn lại

//   randomSeed(analogRead(0));  // Khởi tạo seed cho hàm randomFloat
// }

// // Hàm vòng lặp chính
// void loop() {
//   Blynk.run();

//   // Bỏ điều kiện WelcomeDisplayTimeout để hiển thị ngay lập tức
//   // if (!WelcomeDisplayTimeout()) return;

//   updateDHT();              // Cập nhật dữ liệu từ cảm biến DHT
//   TrafficLightMode();       // Điều khiển đèn giao thông
//   YellowBlinkMode();        // Chế độ nhấp nháy đèn vàng
//   updateDisplay();          // Cập nhật màn hình và V0
//   handleTelegramMessages(); // Xử lý lệnh từ Telegram
// }