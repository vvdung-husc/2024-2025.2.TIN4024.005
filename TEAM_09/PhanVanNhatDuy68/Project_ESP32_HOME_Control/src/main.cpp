#define BLYNK_TEMPLATE_ID "TMPL6hxMhlQwQ"
#define BLYNK_TEMPLATE_NAME "ESP32 HOME CONTROL"
#define BLYNK_AUTH_TOKEN "7VA3jiWeydomtvtV0DW0KufdbPS6WJ83"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>

// Wi-Fi
char ssid[] = "Wokwi-GUEST";
char password[] = "";

// Telegram
#define BOT_TOKEN "8044446031:AAGghQhARa6eiLzS9WLTyvBXoxVFlcY5oEc"
#define CHAT_ID "-1002619419433"
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// DHT
#define DHTPIN 15  // ⚠️ Đúng theo sơ đồ là chân 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// GPIO (chân theo sơ đồ thực tế ESP32 trên Wokwi)
#define LED_PIN 22        // LED đỏ (đèn)
#define FAN_PIN 23        // LED xanh (quạt)
#define BTN_LED_PIN 19    // Nút nhấn điều khiển đèn
#define BTN_FAN_PIN 18    // Nút nhấn điều khiển quạt
#define LDR_PIN 34        // Cảm biến ánh sáng (photoresistor)

bool ledState = false;
bool fanState = false;

// Virtual Pins
#define VPIN_LIGHT_CONTROL V0
#define VPIN_FAN_CONTROL V1
#define VPIN_LIGHT_SENSOR V2
#define VPIN_TEMP_SENSOR V3
#define VPIN_STATUS V4

BlynkTimer timer;

// 📡 Gửi dữ liệu lên Blynk + Telegram
void sendStatus() {
  float temp = dht.readTemperature();
  int ldrValue = analogRead(LDR_PIN);
  int brightness = map(ldrValue, 0, 4095, 0, 100);

  Blynk.virtualWrite(VPIN_TEMP_SENSOR, temp);
  Blynk.virtualWrite(VPIN_LIGHT_SENSOR, brightness);

  String status = "🌡 Nhiệt độ: " + String(temp, 1) + "°C\n" +
                  "☀️ Ánh sáng: " + String(brightness) + "%\n" +
                  "💡 Đèn: " + String(ledState ? "BẬT" : "TẮT") + "\n" +
                  "🌀 Quạt: " + String(fanState ? "BẬT" : "TẮT");

  Blynk.virtualWrite(VPIN_STATUS, status);
  bot.sendMessage(CHAT_ID, status, "");
  Serial.println("[LOG] " + status);
}

// 📲 Xử lý lệnh Telegram
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(VPIN_LIGHT_CONTROL, HIGH);
      bot.sendMessage(CHAT_ID, "💡 Đèn đã BẬT từ Telegram.");
    } else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(VPIN_LIGHT_CONTROL, LOW);
      bot.sendMessage(CHAT_ID, "💡 Đèn đã TẮT từ Telegram.");
    } else if (text == "/fan_on") {
      fanState = true;
      digitalWrite(FAN_PIN, HIGH);
      Blynk.virtualWrite(VPIN_FAN_CONTROL, HIGH);
      bot.sendMessage(CHAT_ID, "🌀 Quạt đã BẬT từ Telegram.");
    } else if (text == "/fan_off") {
      fanState = false;
      digitalWrite(FAN_PIN, LOW);
      Blynk.virtualWrite(VPIN_FAN_CONTROL, LOW);
      bot.sendMessage(CHAT_ID, "🌀 Quạt đã TẮT từ Telegram.");
    } else if (text == "/status") {
      sendStatus();
    }
  }
}

// 🧠 Xử lý nút nhấn vật lý
void checkButtons() {
  if (digitalRead(BTN_LED_PIN) == LOW) {
    delay(200);
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Blynk.virtualWrite(VPIN_LIGHT_CONTROL, ledState);
    bot.sendMessage(CHAT_ID, "💡 Đèn đã " + String(ledState ? "BẬT" : "TẮT") + " bằng nút.");
  }

  if (digitalRead(BTN_FAN_PIN) == LOW) {
    delay(200);
    fanState = !fanState;
    digitalWrite(FAN_PIN, fanState);
    Blynk.virtualWrite(VPIN_FAN_CONTROL, fanState);
    bot.sendMessage(CHAT_ID, "🌀 Quạt đã " + String(fanState ? "BẬT" : "TẮT") + " bằng nút.");
  }
}

// 🚀 setup()
void setup() {
  Serial.begin(115200);
  delay(1000);
  dht.begin();

  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BTN_LED_PIN, INPUT_PULLUP);
  pinMode(BTN_FAN_PIN, INPUT_PULLUP);

  secured_client.setInsecure(); // Cho phép HTTPS không kiểm chứng

  WiFi.begin(ssid, password);
  Serial.print("🔌 Kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Đã kết nối WiFi!");

  configTime(0, 0, "pool.ntp.org"); // Đồng bộ thời gian cho Telegram

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  delay(2000);

  timer.setInterval(5000L, sendStatus);

  timer.setInterval(10000L, []() {
    if (!Blynk.connected()) {
      Serial.println("[⚠️] Mất kết nối Blynk. Đang kết nối lại...");
      Blynk.connect();
    }
  });
}

// 🔁 loop()
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[⚠️] Mất WiFi! Đang kết nối lại...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }

  Blynk.run();
  timer.run();
  checkButtons();
  handleTelegram();
}
