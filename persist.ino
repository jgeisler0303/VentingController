#include <EEPROM.h>
#include "globalVars.h"


void readEEPROM() {
  uint16_t a= 0;
  byte i, j;
  byte* p= (byte*)(void*)thresholdFactor;
  for (i= 0; i < sizeof(float)*N_SECTORS; i++)
    *p++= EEPROM.read(a++);
    
  #ifdef SERIAL_OUT
  Serial.println(F("EEPROM:"));
  #endif
  for(i= 0; i<N_SECTORS; i++) {
    if(thresholdFactor[i]<0.0)
      thresholdFactor[i]= 0.0;
    if(thresholdFactor[i]>1.0)
      thresholdFactor[i]= 1.0;
    #ifdef SERIAL_OUT
    Serial.println(thresholdFactor[i]);
    #endif
  }
    
  for (i= 0; i < N_SECTORS; i++) {
    tempPenalty[i]= EEPROM.read(a++);
    if(tempPenalty[i]>TEMP_PENALTY_THRESHOLD)
      tempPenalty[i]= TEMP_PENALTY_THRESHOLD;
    #ifdef SERIAL_OUT
    Serial.println(tempPenalty[i]);
    #endif
  }
    
  for (i= 0; i < N_SECTORS; i++)
    ventings24[i]= 0;

  for (j= 0; j < 24; j++) {
    for (i= 0; i < N_SECTORS; i++) {
      byte v= EEPROM.read(a++);
      if(v>4) v= 4;
      ventings1[i][j]= v;
      ventings24[i]+= v;
    }
  }
  ventings1Idx= 0;

  #ifdef SERIAL_OUT
  for (i= 0; i < N_SECTORS; i++) {
    for (j= 0; j < 24; j++) {
      Serial.print(ventings1[i][j]);
      Serial.print(' ');
    }
    Serial.print('=');
    Serial.println(ventings24[i]);
  }
  #endif
}

void writeEEPROM() {
  uint16_t a= 0;
  byte i, j, k;
  const byte* p= (const byte*)(const void*)thresholdFactor;
  for (i= 0; i < sizeof(float)*N_SECTORS; i++)
    EEPROM.write(a++, *p++);
    
  for (i= 0; i < N_SECTORS; i++)
    EEPROM.write(a++, tempPenalty[i]);
    
  k= ventings1Idx;
  for (j= 0; j < 24; j++) {
    for (i= 0; i < N_SECTORS; i++)
      EEPROM.write(a++, ventings1[i][k]);
    k++;
    if(k>=24) k= 0;
  }
}
