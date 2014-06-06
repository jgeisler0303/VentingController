uint16_t gas10minCounter= 0;
uint32_t gasTotalCounter= 0;
byte gasCounterState= 0; // 0= H1, 1= H2, 2= L1, 3= L2
uint8_t doorPin;

uint16_t rain10minCounter= 0;
uint16_t door10minCounter= 0;

void gasCounter50msTask() {
  uint8_t gasPin= ioExp.digitalRead(15);
  //Serial.println(gasPin);
  
  if(gasPin!=0) {
    if(gasCounterState==0 || gasCounterState==1) gasCounterState= 1;
    else gasCounterState= 0;
  } else {
    if(gasCounterState==2) { 
      gasCounterState= 3;
      gas10minCounter++;
      gasTotalCounter++;
    } else 
      if(gasCounterState!=3)
        gasCounterState= 2;
  }
  
  uint8_t rainPin= ioExp.digitalRead(14);
  if(rainPin==0)
    rain10minCounter++;
    
  doorPin= ioExp.digitalRead(13);
  ioExp.digitalWrite(11, doorPin);  
  if(doorPin!=0)
    door10minCounter++;
}
