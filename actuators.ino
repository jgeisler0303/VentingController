// DONE
// window pins
#include "config.h"
#include <Mcp23s17.h>

// Then choose any other free pin as the Slave Select (pin 10 if the default but doesnt have to be)
#define MCP23S17_SLAVE_SELECT_PIN  8 //arduino   <->   SPI Slave Select           -> CS  (Pin 11 on MCP23S17 DIP)

// SINGLE DEVICE
// Instantiate a single Mcp23s17 object
MCP23S17 ioExp = MCP23S17(MCP23S17_SLAVE_SELECT_PIN, true);


#define WINDOW_FINISH_TIME 15
byte windowOpenFinishTimer[N_SECTORS];
byte windowCloseFinishTimer[N_SECTORS];


void initActuators() {
  ioExp.pinMode((uint16_t)0x0FFF);
  ioExp.port(0);
  /*Serial.println(ioExp.pinMode(), BIN);
  Serial.println(ioExp.port(), BIN);
  ioExp.digitalWrite(15, 1);
  Serial.println(ioExp.port(), BIN);*/
  
  for(byte i= 0; i<N_SECTORS; i++) {
    windowOpenFinishTimer[i]= 0;
    windowCloseFinishTimer[i]= 0;
  }
}  

void startVenting(byte sect) {
  for(byte j= 0; j<N_ACTORS_PER_SECTION; j++) {
    if(ventPins[sect][j]>=0) ioExp.digitalWriteD(ventPins[sect][j], HIGH);
    
    if(windowOpenPins[sect][j]>=0) {
      ioExp.digitalWriteD(windowOpenPins[sect][j], HIGH);
      windowOpenFinishTimer[sect]= WINDOW_FINISH_TIME;
    }
  }
  ioExp.applyDigitalWrite();
}

void stopVenting(byte sect) {
  for(byte j= 0; j<N_ACTORS_PER_SECTION; j++) {
    if(ventPins[sect][j]>=0) ioExp.digitalWriteD(ventPins[sect][j], LOW);
    if(windowClosePins[sect][j]>=0) {
      ioExp.digitalWriteD(windowClosePins[sect][j], HIGH);
      windowCloseFinishTimer[sect]= WINDOW_FINISH_TIME;
    }
  }
  ioExp.applyDigitalWrite();
}

void windowFinishTask() {
  for(byte i= 0; i<N_SECTORS; i++) {
    if(windowOpenFinishTimer[i]>0) {
      windowOpenFinishTimer[i]--;
      if(windowOpenFinishTimer[i]==0)      
        for(byte j= 0; j<N_ACTORS_PER_SECTION; j++)
          ioExp.digitalWriteD(windowOpenPins[i][j], LOW);
    }

    if(windowCloseFinishTimer[i]>0) {
      windowCloseFinishTimer[i]--;
      if(windowCloseFinishTimer[i]==0)      
        for(byte j= 0; j<N_ACTORS_PER_SECTION; j++)
          ioExp.digitalWriteD(windowClosePins[i][j], LOW);
    }
  }
  ioExp.applyDigitalWrite();
}

void reapplyOut() {
  ioExp.applyDigitalWrite();
}
