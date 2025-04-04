// LÊ BÁ NHẬT MINH
#define BLYNK_TEMPLATE_ID "TMPL6N92hnKMZ"
#define BLYNK_TEMPLATE_NAME "ESP8266 Blynk Project"
#define BLYNK_AUTH_TOKEN "JuuTlquqg72SD0zhHGuFA5-eb_vNroz1"

// HỒ XUÂN LÃM
// #define BLYNK_TEMPLATE_ID "TMPL6HwO6Vtka"
// #define BLYNK_TEMPLATE_NAME "IOT"
// #define BLYNK_AUTH_TOKEN "Q1P__3piELCWKs22dLh589vQktApTTKJ"

// Nguyễn Viết Hùng
// #define BLYNK_TEMPLATE_ID "TMPL6J1Jal72W"
// #define BLYNK_TEMPLATE_NAME "esp8266"
// #define BLYNK_AUTH_TOKEN "u1cKK2vyD5FCTZ76up_wKqrc3n5LPGg9"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h> // Thêm thư viện Telegram

// LÊ BÁ NHẬT MINH
#define BOTtoken "7965370700:AAGHB_XS6lNgahXM6ZDbEJj-tV4chtwuiBo" // Bot Token từ BotFather
#define GROUP_ID "-4692915290"                                    // Group ID của bạn
// HỒ XUÂN LÃM
// #define BOTtoken "7984313333:AAEfM0Yox23-5fuM31lZmYBHe7NY2HsjXxA" // Bot Token từ BotFather
// #define GROUP_ID "-4692915290"                                    // Group ID của bạn

// Nguyễn Viết Hùng
// #define BOTtoken "7520744232:AAHTX360kc_5wuJZuZUmTc2Zs9FqD_Zvg44"
// #define GROUP_ID "-4554634970"

#define gPIN 15 // Đèn xanh
#define yPIN 2  // Đèn vàng
#define rPIN 5  // Đèn đỏ

#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo màn hình OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// Kết nối WiFi
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

// Khởi tạo Telegram client
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

BlynkTimer timer;

// -------------------- Biến toàn cục --------------------
bool yellowBlinkMode = false;       // Chế độ đèn vàng nhấp nháy
bool trafficLightOn = true;         // Trạng thái đèn giao thông (true: hoạt động, false: tắt)
unsigned long startMillis;          // Thời gian bắt đầu
unsigned long lastTelegramTime = 0; // Thời gian gửi thông báo Telegram cuối cùng
unsigned long lastBotCheck = 0;     // Thời gian kiểm tra tin nhắn Telegram cuối cùng

float fHumidity = 0.0;
float fTemperature = 0.0;

// -------------------- Hàm kiểm tra thời gian --------------------
bool IsReady(unsigned long &lastTimer, unsigned long interval)
{
    unsigned long currentMillis = millis();
    if (currentMillis - lastTimer >= interval)
    {
        lastTimer = currentMillis;
        return true;
    }
    return false;
}

// -------------------- Hàm xử lý tin nhắn Telegram --------------------
void handleTelegramMessages()
{
    if (!IsReady(lastBotCheck, 1000)) // Kiểm tra tin nhắn mỗi 1 giây
        return;

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
        for (int i = 0; i < numNewMessages; i++)
        {
            String chat_id = String(bot.messages[i].chat_id);
            String text = bot.messages[i].text;

            // Kiểm tra nếu tin nhắn đến từ group đúng
            if (chat_id != GROUP_ID)
                continue;

            // Xử lý lệnh /traffic_off
            if (text == "/traffic_off")
            {
                trafficLightOn = false;
                digitalWrite(gPIN, LOW);
                digitalWrite(yPIN, LOW);
                digitalWrite(rPIN, LOW);
                yellowBlinkMode = false; // Tắt chế độ nhấp nháy nếu đang bật
                bot.sendMessage(GROUP_ID, "🚦 Đèn giao thông đã được TẮT!", "");
            }
            // Xử lý lệnh /traffic_on
            else if (text == "/traffic_on")
            {
                trafficLightOn = true;
                bot.sendMessage(GROUP_ID, "🚦 Đèn giao thông đã được BẬT!", "");
            }
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
}

// -------------------- Hàm gửi cảnh báo qua Telegram --------------------
void sendTelegramAlert()
{
    // Kiểm tra mỗi 5 phút (300000ms)
    if (!IsReady(lastTelegramTime, 300000))
        return;

    // if (!IsReady(lastTelegramTime, 3000))
    //     return;

    String message = "";

    // Kiểm tra nhiệt độ
    if (fTemperature < 10.0)
    {
        message = "⚠️ CẢNH BÁO: Nhiệt độ " + String(fTemperature) + "°C - Nguy cơ hạ thân nhiệt, tê cóng, giảm miễn dịch!";
    }
    else if (fTemperature >= 10.0 && fTemperature < 15.0)
    {
        message = "⚠️ CẢNH BÁO: Nhiệt độ " + String(fTemperature) + "°C - Cảm giác lạnh, tăng nguy cơ mắc bệnh đường hô hấp!";
    }
    else if (fTemperature >= 25.0 && fTemperature <= 30.0)
    {
        message = "✅ Nhiệt độ lý tưởng: " + String(fTemperature) + "°C - Ít ảnh hưởng đến sức khỏe.";
    }
    else if (fTemperature > 30.0 && fTemperature <= 35.0)
    {
        message = "⚠️ CẢNH BÁO: Nhiệt độ " + String(fTemperature) + "°C - Có thể gây mệt mỏi, mất nước!";
    }
    else if (fTemperature > 35.0 && fTemperature <= 40.0)
    {
        message = "⚠️ CẢNH BÁO: Nhiệt độ " + String(fTemperature) + "°C - Nguy cơ sốc nhiệt, chuột rút, say nắng!";
    }
    else if (fTemperature > 40.0)
    {
        message = "🚨 CẢNH BÁO NGHIÊM TRỌNG: Nhiệt độ " + String(fTemperature) + "°C - Cực kỳ nguy hiểm, có thể gây suy nội tạng, đột quỵ nhiệt!";
    }

    // Kiểm tra độ ẩm
    if (fHumidity < 30.0)
    {
        message += "\n⚠️ CẢNH BÁO: Độ ẩm " + String(fHumidity) + "% - Da khô, kích ứng mắt, tăng nguy cơ bệnh hô hấp!";
    }
    else if (fHumidity >= 40.0 && fHumidity <= 60.0)
    {
        if (message == "")
        {
            message = "✅ Độ ẩm lý tưởng: " + String(fHumidity) + "% - Ít ảnh hưởng đến sức khỏe.";
        }
        else
        {
            message += "\n✅ Độ ẩm lý tưởng: " + String(fHumidity) + "% - Ít ảnh hưởng đến sức khỏe.";
        }
    }
    else if (fHumidity > 70.0 && fHumidity <= 80.0)
    {
        message += "\n⚠️ CẢNH BÁO: Độ ẩm " + String(fHumidity) + "% - Tăng nguy cơ nấm mốc, vi khuẩn phát triển, gây bệnh đường hô hấp!";
    }
    else if (fHumidity > 80.0)
    {
        message += "\n🚨 CẢNH BÁO NGHIÊM TRỌNG: Độ ẩm " + String(fHumidity) + "% - Oi bức, khó thở, tăng nguy cơ sốc nhiệt!";
    }

    // Gửi tin nhắn nếu có cảnh báo
    if (message != "")
    {
        bot.sendMessage(GROUP_ID, message, "");
    }
}

// -------------------- Hàm cập nhật dữ liệu --------------------
void updateData()
{
    static unsigned long lastTimer = 0;
    if (!IsReady(lastTimer, 2000))
        return;

    fTemperature = random(-400, 801) / 10.0; // Nhiệt độ từ -40.0°C đến 80.0°C
    fHumidity = random(0, 1001) / 10.0;      // Độ ẩm từ 0.0% đến 100.0%

    Serial.print("Temperature: ");
    Serial.print(fTemperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(fHumidity);
    Serial.println(" %");

    Blynk.virtualWrite(V1, fTemperature);
    Blynk.virtualWrite(V2, fHumidity);

    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese1);
    oled.drawUTF8(0, 14, "Nhiệt độ: ");
    oled.setCursor(80, 14);
    oled.print(fTemperature);
    oled.print("C");

    oled.drawUTF8(0, 42, "Độ ẩm: ");
    oled.setCursor(80, 42);
    oled.print(fHumidity);
    oled.print("%");

    oled.sendBuffer();

    // Gọi hàm gửi cảnh báo Telegram
    sendTelegramAlert();
}

// -------------------- Điều khiển đèn LED --------------------
void controlTrafficLights()
{
    static unsigned long lastTimer = 0;
    static int currentLed = 0;
    static const int ledPin[3] = {gPIN, yPIN, rPIN};

    // Nếu đèn giao thông bị tắt, không thực hiện điều khiển
    if (!trafficLightOn)
        return;

    if (yellowBlinkMode)
    {
        if (!IsReady(lastTimer, 500))
            return;
        digitalWrite(gPIN, LOW);
        digitalWrite(rPIN, LOW);
        digitalWrite(yPIN, !digitalRead(yPIN)); // Nhấp nháy đèn vàng
        return;
    }

    if (!IsReady(lastTimer, 1000))
        return;
    int prevLed = (currentLed + 2) % 3;
    digitalWrite(ledPin[prevLed], LOW);
    digitalWrite(ledPin[currentLed], HIGH);
    currentLed = (currentLed + 1) % 3;
}

// -------------------- Hiển thị thời gian chạy lên Blynk --------------------
void sendUptime()
{
    unsigned long uptime = millis() / 1000;
    Blynk.virtualWrite(V0, uptime);
}

// -------------------- Nhận lệnh từ Blynk --------------------
BLYNK_WRITE(V3)
{
    int value = param.asInt();
    yellowBlinkMode = (value == 1);
    if (yellowBlinkMode)
    {
        digitalWrite(gPIN, LOW);
        digitalWrite(rPIN, LOW);
        digitalWrite(yPIN, HIGH);
    }
    else
    {
        digitalWrite(yPIN, LOW);
    }
}

// -------------------- Setup --------------------
void setup()
{
    Serial.begin(115200);
    pinMode(gPIN, OUTPUT);
    pinMode(yPIN, OUTPUT);
    pinMode(rPIN, OUTPUT);

    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);
    digitalWrite(rPIN, LOW);

    Wire.begin(OLED_SDA, OLED_SCL);
    oled.begin();
    oled.clearBuffer();

    oled.setFont(u8g2_font_unifont_t_vietnamese1);
    oled.drawUTF8(0, 14, "Welcom to chanel");
    oled.sendBuffer();

    // Kết nối WiFi và Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Cấu hình client Telegram
    client.setInsecure(); // Bỏ qua kiểm tra chứng chỉ SSL (dùng cho ESP8266)

    // Thiết lập timer
    timer.setInterval(2000L, updateData);
    timer.setInterval(1000L, sendUptime);

    startMillis = millis();
}

// -------------------- Loop --------------------
void loop()
{
    Blynk.run();
    timer.run();
    controlTrafficLights();
    handleTelegramMessages(); // Gọi hàm xử lý tin nhắn Telegram
}
