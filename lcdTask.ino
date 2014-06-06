// DONE

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <StopWatch.h>
#include "cosmComm.h"
#include "config.h"
#include "globalVars.h"
#include <Time.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
StopWatch lcdResetTimer;
bool suppressNextTick= false;
byte lcdFlash= 0;
byte lcdToggle1= 0;
byte lcdToggle2= 0;
byte lcdToggle3= 0;

byte lineOffset= 0;
#define LCD_LINES 4
#define LCD_VLINES 9

void lcdTaskInit() {
  lcd.init();
  //lcd.noBacklight();
  lcd.print("Init...");
  lcdIlluminate();
}

void lcdTask() {
  if(testOut!=0) return;
  lcdToggle1++; if(lcdToggle1>5) lcdToggle1= 0;
  lcdToggle2++; if(lcdToggle2>3) lcdToggle2= 0;
  lcdToggle3++; if(lcdToggle3>1) lcdToggle3= 0;
  if(lcdResetTimer.state()==StopWatch::RUNNING && lcdResetTimer.elapsed()>20000) {
    lcd.noBacklight();
    lcdResetTimer.stop();
    lineOffset= 0;
  }
  if(suppressNextTick) {
    suppressNextTick= false;
    return;
  }
  if(lcdFlash==1) lcdFlash++;
  if(lcdFlash>=2) {
    lcdFlash= 0;
    lcd.noBacklight();
  }
    
  printLines();
}

void buttonPushed() {
  if(lcdResetTimer.state()==StopWatch::RUNNING)
    incLineOffset();
  printNow();
  lcdFlash= 0;
}

void actionPrint() {
  if(lcdResetTimer.state()!=StopWatch::RUNNING) {
    lineOffset= 3;
    printNow();
  } else {
    lcdFlash= 1;
    lcd.backlight();
  }
}

void lcdIlluminate() {
  lcd.backlight();
  lcdResetTimer.reset();
  lcdResetTimer.start();
}

void printNow() {
  lcdIlluminate();
  printLines();
  suppressNextTick= true;    
}

void printLines() {
  // lcd.clear();
  for(byte i= 0; i<LCD_LINES; i++) {
    byte line= i+lineOffset;
    if(line>=LCD_VLINES) line-= LCD_VLINES;
    lcd.setCursor(0,i);
    byte n= printLine(line, lcd);
    for(byte j= n; j<20; j++) lcd.print(' ');
    #ifdef SERIAL_OUT
    if(i==0) { printLine(line, Serial); Serial.println(); }
    #endif
  }  
}

byte printLine(byte line, Print& out) {
  byte snr;
  char decimal;
  time_t t;
  switch(line) {
    case 0:
      
      if(timeStatus() == timeNotSet)
        decimal= '?';
      else
        decimal= '.';
      t = now();
      
      return printTime(out, t, decimal);
      // out.print(pDateBuffer);
      break;
    case 1:
      snr= 0;
      if(lcdToggle1>3) snr= 3;
      else if(lcdToggle1>1) snr= 2;
      return printSensor(snr, out);
      break;
    case 2:
      if(lcdToggle2>1) snr= 4;
      else snr= 1;
      return printSensor(snr, out);
      break;
    case 3:
      return printSectorA(0, out);
      break;
    case 4:
      return printSectorB(0, out);
      break;
    case 5:
      return printSectorC(0, out);
      break;
    case 6:
      return printCosm(out, cosmState);
      break;
    case 7:
      return printGas(out);
      break;
    case 8:
      return printLastHTTPError(out);
      break;
  }
}

byte printTime(Print& out, time_t t, char decimal) {
  byte n= 0;

  n+= print2Digit(day(t), out);
  n+= out.print(decimal);
  n+= print2Digit(month(t), out);
  n+= out.print(decimal);
  n+= out.print(year(t));
  n+= out.print(' ');
  
  n+= print2Digit(hour(t), out);
  n+= out.print(':');
  n+= print2Digit(minute(t), out);
  n+= out.print(':');
  n+= print2Digit(second(t), out);
  return n;
}

byte print2Digit(byte d, Print& out) {
  if(d<10) out.print('0');
  out.print(d);
  return 2;
}

byte printCosm(Print& out, int state) {
  byte n= 0;
  n+= out.print(F("Cosm: "));
  
  switch(state) {
    case 0:
      n+= out.print(F("no http state"));
      break;
    case 1:
      n+= out.print(F("no connection"));
      break;      
    case 2:
      n+= out.print(F("timeout"));
      break;      
    case 3:
      n+= out.print(F("no streams"));
      break;
    case 4:
      n+= out.print(F("no datestr"));
      break;
    case 5:
      n+= out.print(F("too long"));
      n+= out.print(cosmLength);
      break;
    case 200:
      n+= out.print(F("http success"));
      break;
    default:
      n+= out.print(F("error "));    
      n+= out.print(state);
  }
  return n;  
}

byte printDHT22Err(DHT22_ERROR_t e, Print& out) {
  byte n= 0;
  switch(e) {
    case DHT_BUS_HUNG:
      n+= out.print(F("bus hung"));
      break;
    case DHT_ERROR_ACK_TOO_LONG:
      n+= out.print(F("ack too long"));
      break;
    case DHT_ERROR_NOT_PRESENT:
      n+= out.print(F("not present"));
      break;
    case DHT_ERROR_SYNC_TIMEOUT:
      n+= out.print(F("sync timeout"));
      break;
    case DHT_ERROR_DATA_TIMEOUT:
      n+= out.print(F("data timeout"));
      break;
    case DHT_ERROR_CHECKSUM:
      n+= out.print(F("checksum"));
      break;
    case DHT_ERROR_TOOQUICK:
      n+= out.print(F("too quick"));
      break;
    default:
      n+= out.print(F("error"));      
  }      
  return n;  
}  

byte printSensor(byte i, Print& out) {
  byte n= 0;
  switch(i) {
    case 0:
    case 2:
    case 3:
      n+= out.print(F("In"));
      if(i==0)
        n+= out.print('1');
      else if(i==3)
        n+= out.print('3');
      else
        n+= out.print('2');
      break;
    case 1:
    case 4:
      n+= out.print(F("Out"));
      if(i==4)
        n+= out.print('2');
      else
        n+= out.print('1');
      break;
  }
  n+= out.print(' ');
  if(dht22CurrentErrorCode[i]==DHT_ERROR_NONE) {
    n+= out.print(currentTemp[i],1); n+= out.print(char(223)); n+= out.print(F("C "));
    n+= out.print(currentHumidity[i],0); n+= out.print(F("% "));
  } else
    n+= printDHT22Err(dht22CurrentErrorCode[i], out);
    
  //out.print();
  // out.print(currentAbsHumidity[i],1); out.print("g/m3"); //179
  return n;
} 

byte printSectorA(byte i, Print& out) {
  byte n= 0;
//  out.print("Diff: "); out.print(currentAbsHumidity[0]-currentAbsHumidity[1]); out.print("g/m3");
  n+= out.print('B'); n+= out.print(i+1, 1); n+= out.print(' ');
  float thr= MAX_THRESHOLD*thresholdFactor[i];
  n+= out.print(sectorHDiff[i], 1);
  n+= out.print(F("thr"));
  n+= out.print(thr, 1);
  n+= out.print(F(" g/m3"));
  return n;
}

byte printSectorB(byte i, Print& out) {
  byte n= 0;
  n+= out.print('B'); n+= out.print(i+1, 1); n+= out.print(' '); // 3
  n+= out.print(F("pen ")); // +4=7
  n+= out.print(curTempPenalty[i]); // +2=9
  n+= out.print(F(" cur "));  // +5=14
  n+= out.print(tempPenalty[i]); // +2= 16
  n+= out.print('(');  // +1=17
  n+= out.print(targetTempVal, 0); // +2= 19
  n+= out.print(')');  // +1=20
  
  //out.print(sectorTDiff[i],1);
  return n;
}

byte printSectorC(byte i, Print& out) {
  byte n= 0;
  n+= out.print('B'); n+= out.print(i+1, 1); n+= out.print(' '); // 3

  // out.print(F("#on "));
  n+= out.print(ventings24[i]); // 5
  n+= out.print('/'); // 6
  if(sectorError[i]==DHT_ERROR_NONE) {
    if(cycleStep[i]==-1) {
      n+= out.print(F("idle")); // 10
    } else if(cycleStep[i]<VENT_CYCLES) {
      n+= out.print(F("vent"));
    } else if(cycleStep[i]<(VENT_CYCLES+SETTLE_CYCLES)) {
      n+= out.print(F("setl"));
    }
    if(windowOpenFinishTimer[i]>0)
      n+= out.print(F("(o)")); // 13
    if(windowCloseFinishTimer[i]>0)
      n+= out.print(F("(c)"));
    n+= out.print(' '); //14
    n+= out.print(actionTime); //17
    n+= out.print(':'); //18
    if(counterSec<10)
      n+= out.print('0'); //19
    n+= out.print(counterSec); //20
  } else
    n+= printDHT22Err(sectorError[i], out);
    
  //out.print();
  return n;
}

byte printGas(Print& out) {
  byte n= 0;
  
  n+= out.print(F("Gas: "));
  n+= out.print(gas10minCounter);
  n+= out.print(',');
  n+= out.print(gasTotalCounter);
  n+= out.print(';');
  n+= out.print(COSM_CONTENT_LENGHT-cosmLength);
  
  return n;
}

byte printLastHTTPError(Print& out) {
  byte n= 0;
  
  n+= out.print('!');
  if(lcdToggle3==0)
    n+= printCosm(out, lastCosmError);
  else
    n+= printTime(out, lastErrorTime, '.');
  
  return n;
}

void incLineOffset() {
  lineOffset++;
  if(lineOffset>=LCD_VLINES) lineOffset= 0;
}

void printResetMsg1() {
  lcd.setCursor(0,1);
  printResetStr1(lcd);
  #ifdef SERIAL_OUT
  printResetStr1(Serial);
  Serial.println();
  #endif
}

void printResetStr1(Print& out) {
  out.print(F("EEPROM reset in 2s"));
}

void printResetMsg2() {
  lcd.setCursor(13,1);
  printResetStr2(lcd);
  #ifdef SERIAL_OUT
  printResetStr2(Serial);
  Serial.println();
  #endif
}

void printResetStr2(Print& out) {
  out.print(F("now"));
}

void printTestMsg1() {
  lcd.clear();
  lcd.setCursor(0,1);
  printTestStr1(lcd);
  #ifdef SERIAL_OUT
  printTestStr1(Serial);
  Serial.println();
  #endif
}

void printTestStr1(Print& out) {
  out.print(F("Test in 2s"));
}

void printTestMsg2() {
  lcd.setCursor(13,1);
  printResetStr2(lcd);
  #ifdef SERIAL_OUT
  printResetStr2(Serial);
  Serial.println();
  #endif
}

