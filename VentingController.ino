
#include <TimedAction.h>
#include <DHT22.h>
#include "globalVars.h"
#include "button.h"
#include <Time.h>
#include "cosmComm.h"

//#define SERIAL_OUT

void task50ms();
void task1s();

TimedAction task50msAction = TimedAction(50,task50ms);
TimedAction task1sAction = TimedAction(1000,task1s);
byte counterSec= 57;
byte counterMin= 0;

byte testOut= 0;
byte testTime= 0;


void buttonPushed2s() {
  if(cycleStep[0]!=-1 || cycleStep[1]!=-1 || cycleStep[2]!=-1)
    return;
    
  testOut= 1;
  testTime= 0;
  printTestMsg1();
}

void buttonPushed4s() {
  if(cycleStep[0]!=-1 || cycleStep[1]!=-1 || cycleStep[2]!=-1) {
    testOut= 0;
    return;
  }
  testOut= 2;
  testTime= 0;
  printTestMsg2();
  startVenting(0);
  startVenting(1);
  startVenting(2);
}

void task50ms() {
  buttonTask();
  gasCounter50msTask();
}

void task1s() {
  if(testOut==1) {
    testTime++;
    if(testTime>=3) {
      testOut= 0;
    }    
  }
  
  if(testOut==2) {
    testTime++;
    if(testTime>=60) {
      testOut= 0;
      stopVenting(0);
      stopVenting(1);
      stopVenting(2);
    }
  }
  windowFinishTask();
  reapplyOut();
  lcdTask();
  #ifdef SERIAL_OUT
  incLineOffset();
  //Serial.println(freeRam ());
  #endif
  
  counterSec++;  
  if(counterSec>=60) {
    counterSec= 0;
    counterMin++;
    actionTime++;
    if(counterMin>=60) counterMin= 0;
  }
  if(counterSec==30 || counterSec==0)
    task30s();
  if(counterSec==3 && counterMin==0)
    task1h();
  #ifdef SERIAL_OUT
  if(counterSec==5) {
  #else
  if(counterSec==5 && (counterMin==0 || counterMin==10 || counterMin==20 || counterMin==30 || counterMin==40 || counterMin==50)) {
  #endif
    task10min();
    #ifdef SERIAL_OUT
    writeEEPROM();
    #endif
  }
}

void task30s() {
  dataTask();
  supervisorTask();
  #ifdef SERIAL_OUT
  Serial.println("Task30");
  #endif
}

void task10min() {
  calcMean();
  actionTask();
  actionPrint();
  cosmCommSend();
  if(cosmState==200) {
    updateTime();
    adjustTime(60*60);
    //#ifdef SERIAL_OUT
    //Serial.println(pDateBuffer);
    //#endif
    gas10minCounter= 0;
    rain10minCounter= 0;
    door10minCounter= 0;
  } else {
    lastCosmError= cosmState;
    lastErrorTime= now();
  }
  
  clearMean();
  #ifdef SERIAL_OUT
  Serial.println("Task10m");
  #endif
  //saveTask();
}

void task1h() {
  updateVentings24();
  writeEEPROM();
}

void setup(void) {
  //Serial.begin(9600);
  lcdTaskInit();
  buttonTaskInit();
  dataTaskInit();
  initAction();
  cosmCommInit();
  initActuators();

  #ifdef SERIAL_OUT
  Serial.begin(9600);
  #endif
  
  if(buttonPressed()) {
    printResetMsg1();
    delay(2000);
    if(buttonPressed()) {
      printResetMsg2();
      writeEEPROM();
    }
  }
  readEEPROM();
  #ifdef SERIAL_OUT
  #endif
  //saveInit();
}

void loop() {
  task50msAction.check();
  task1sAction.check();
}
