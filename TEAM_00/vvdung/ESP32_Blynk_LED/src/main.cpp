#include <Arduino.h>
#include <TM1637Display.h>

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL64YL8fJrk"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "S9-UuqRP6ItPoUGPZYbtSWknol03FF-0"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Network settings
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

//Pin
#define btnBLED  23
#define pinBLED  21

// Module connection pins (Digital Pins)
#define CLK 18
#define DIO 19


ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
bool blueButtonON = true;

TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
    
  display.setBrightness(0x0f);
  
  // Start the WiFi connection
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass);

  Serial.println();
  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON);
  
  Serial.println("== START ==>");
}

void loop() {  
  Blynk.run();

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
}

// put function definitions here:
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}
void updateBlueButton(){
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  }
  else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
    display.clear();
  }    
}

void uptimeBlynk(){
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  if (blueButtonON){
    display.showNumberDec(value);
  }
}
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();  
  if (blueButtonON){
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    
  }
  else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);   
    display.clear(); 
  }
}