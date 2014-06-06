// DONE
#include "button.h"

byte buttonState= 0;         // 0: Lo2, 1: Hi1, 2: Hi2, 3: Lo1
uint16_t buttonCounter= 0;


void buttonTaskInit() {
  pinMode(buttonPin, INPUT);     
  digitalWrite(buttonPin, HIGH);    
}

void buttonTask() {
  if(buttonPressed()) { // pushed
    if(buttonCounter<(20*10))
      buttonCounter++;
    if(buttonCounter==(20*2))
      buttonPushed2s();
    if(buttonCounter==(20*4))
      buttonPushed4s();
      
    if(buttonState==0) buttonState= 1;
    if(buttonState==3) buttonState= 2;
    if(buttonState==1) {
      buttonPushed();
      buttonState= 2;
    }
  } else {
    buttonCounter= 0;
    if(buttonState==1) buttonState= 0;
    if(buttonState==2) buttonState= 3;
    if(buttonState==3) buttonState= 0;
  }
}

