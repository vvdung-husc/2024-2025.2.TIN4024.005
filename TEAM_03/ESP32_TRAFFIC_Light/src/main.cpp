#include <Arduino.h>
#include <TM1637Display.h>

//Pin
#define rLED  5
#define yLED  17
#define gLED  16

#define CLK   15
#define DIO   2

#define ldrPIN  13

//1000 ms = 1 seconds
#define rTIME  5000   //5 seconds
#define yTIME  3000
#define gTIME  10000

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
int currentLED = rLED;
int tmCounter = rTIME / 1000;
ulong counterTime = 0;

int darkThreshold = 1000;

TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();
bool isDark();
void YellowLED_Blink();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);

  pinMode(ldrPIN,INPUT);

  tmCounter = (rTIME / 1000) - 1;
  display.setBrightness(7);
  
  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  currentLED = rLED;
  nextTimeTotal += rTIME;
  display.showNumberDec(tmCounter--, true, 2, 2);  
  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  "); Serial.print((nextTimeTotal/1000)%60);Serial.println(" (ms)");
}

void loop() {
  // put your main code here, to run repeatedly:
  currentMiliseconds = millis();
  //NonBlocking_Traffic_Light();

  if (!isDark()) NonBlocking_Traffic_Light_TM1637();
  else YellowLED_Blink();
  
}

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}
void NonBlocking_Traffic_Light(){
  switch (currentLED) {
    case rLED: // Đèn đỏ: 5 giây
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        nextTimeTotal += gTIME;
        Serial.print("2. GREEN\t => Next "); Serial.println(nextTimeTotal);        
      } 
      break;

    case gLED: // Đèn xanh: 7 giây
      if (IsReady(ledTimeStart,gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        Serial.print("3. YELLOW\t => Next "); Serial.println(nextTimeTotal);        
      }
      break;

    case yLED: // Đèn vàng: 2 giây
      if (IsReady(ledTimeStart,yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        Serial.print("1. RED \t\t => Next "); Serial.println(nextTimeTotal);        
      }
      break;
  }  
}

void NonBlocking_Traffic_Light_TM1637(){
  bool bShowCounter = false;
  switch (currentLED) {
    case rLED: // Đèn đỏ: 5 giây
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        nextTimeTotal += gTIME;
        tmCounter = (gTIME / 1000) - 1 ; 
        bShowCounter = true;  
        counterTime = currentMiliseconds;        
        Serial.print("2. GREEN  => YELLOW "); Serial.print((nextTimeTotal/1000)%60);Serial.println(" (ms)");       
      } 
      break;

    case gLED: // Đèn xanh: 7 giây
      if (IsReady(ledTimeStart,gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        tmCounter = (yTIME / 1000) - 1; 
        bShowCounter = true;   
        counterTime = currentMiliseconds;    
        Serial.print("3. YELLOW => RED    "); Serial.print((nextTimeTotal/1000)%60);Serial.println(" (ms)");      
      }
      break;

    case yLED: // Đèn vàng: 2 giây
      if (IsReady(ledTimeStart,yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        tmCounter = (rTIME / 1000) - 1; 
        bShowCounter = true;       
        counterTime = currentMiliseconds;        
        Serial.print("1. RED    => GREEN  "); Serial.print((nextTimeTotal/1000)%60);Serial.println(" (ms)");       
      }
      break;
  }
  if (!bShowCounter) bShowCounter = IsReady(counterTime, 1000);
  if (bShowCounter) {
    display.showNumberDec(tmCounter--, true, 2, 2);
  }
}

bool isDark(){
  static ulong darkTimeStart = 0; //lưu thời gian của việc đọc cảm biến
  static uint16_t lastValue = 0;  //lưu giá trị gần nhất của cảm biến
  static bool bDark = false;      //true: value > darkThreshold

  if (!IsReady(darkTimeStart, 50)) return bDark;//50ms đọc cảm biến 1 lầnlần
  uint16_t value = analogRead(ldrPIN);          //đọc cảm biến theo chế đố tương tựtự
  if (value == lastValue) return bDark;         //vẫn bằng giá trị củcủ

  if (value < darkThreshold){
    if (!bDark){
      digitalWrite(currentLED, LOW);
      Serial.print("DARK  value: ");Serial.println(value);
    }   
    bDark = true;   
  }
  else {
    if (bDark){
      digitalWrite(currentLED, LOW);
      Serial.print("LIGHT value: ");Serial.println(value);
    }
    bDark = false;
  }
  
  lastValue = value;  
  return bDark;
}

void YellowLED_Blink(){
  static ulong yLedStart = 0;
  static bool isON = false;

  if (!IsReady(yLedStart,1000)) return;
  if (!isON) digitalWrite(yLED, HIGH);
  else digitalWrite(yLED, LOW);
  isON = !isON;

}
