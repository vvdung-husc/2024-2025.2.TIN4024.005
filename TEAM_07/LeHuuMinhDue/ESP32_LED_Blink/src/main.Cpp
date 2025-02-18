#include <Arduino.h>

// put function declarations here:
int red=5;
bool isON = false;    //su dung cho NonBlocking
ulong timeStart = 0;  //su dung cho NonBlocking

void setup() {
  Serial.begin(115200);
  pinMode(red, OUTPUT);
} 

//true: biến iTimer vượt quá thời gian milisecond
bool IsReady(ulong& iTimer, uint32_t milisecond){
  ulong t = millis();
  if ( t - iTimer < (ulong)milisecond) return false;
  iTimer = t;
  return true;
}

//Non-Blocking: không chặn chương trình chính
void Use_Non_Blocking(){
  if (IsReady(timeStart,1000)){
    if (!isON){ 
      digitalWrite(red, HIGH); // Bật LED
      Serial.println("Non-Blocking LED ON");
    }
    else{
       digitalWrite(red, LOW);  // Tắt LED
       Serial.println("Non-Blocking LED OFF");
    }
    isON = !isON;
  }
}

//lặp chương trình chính
void loop() {
    Use_Non_Blocking(); //Gọi hàm Non-Blocking đếm thời gian
}

