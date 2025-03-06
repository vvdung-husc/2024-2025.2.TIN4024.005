#include <Arduino.h>

int ledPin = 5;
bool isON = false;    //su dung cho NonBlocking
ulong timeStart = 0;  //su dung cho NonBlocking

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void Use_Blocking(){
  digitalWrite(ledPin, HIGH); // Bật LED
  Serial.println("LED ON");
  delay(1000); // Đợi 1 giây
  digitalWrite(ledPin, LOW);  // Tắt LED
  Serial.println("LED OFF");
  delay(1000); // Đợi 1 giây
}

//true: biến iTimer vượt quá thời gian milisecond
bool IsReady(ulong& iTimer, uint32_t milisecond){
  ulong t = millis();
  if ( t - iTimer < (ulong)milisecond) return false;
  iTimer = t;
  return true;
}

void Use_Non_Blocking(){
  if (IsReady(timeStart,1000)){
    if (!isON){ 
      digitalWrite(ledPin, HIGH); // Bật LED
      Serial.println("Non-Blocking LED ON");
    }
    else{
       digitalWrite(ledPin, LOW);  // Tắt LED
       Serial.println("Non-Blocking LED OFF");
    }
    isON = !isON;
  }
}
void loop() {
  //Use_Blocking();
  Use_Non_Blocking();  
  ulong t = millis();
  Serial.print("\nTimer: ");
  Serial.print(t);
  Serial.print("\n");
}