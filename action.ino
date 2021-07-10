// DONE
#include <DHT22.h>
#include <TimeX.h>
#include "globalVars.h"
#include "config.h"

const byte activeSector[]= {1, 0, 0};
#define MIN_THRESHOLD_NORM MIN_THRESHOLD/MAX_THRESHOLD

int8_t cycleStep[N_SECTORS];
bool cycleAbort[N_SECTORS];
float limitHumidity[N_SECTORS];
int actionTime=-1;
float thresholdFactor[N_SECTORS];
float sectorHDiff[N_SECTORS];

byte ventings24[N_SECTORS];
byte ventings1[N_SECTORS][24];
byte ventings1Idx;

byte tempPenalty[N_SECTORS];
byte curTempPenalty[N_SECTORS];
float targetTempVal;

DHT22_ERROR_t sectorError[N_SECTORS];

void initAction() {
  for(byte i= 0; i<N_SECTORS; i++) {
    cycleStep[i]= -1;
    cycleAbort[i]= false;
    thresholdFactor[i]= MIN_THRESHOLD_NORM;
    ventings24[i]= 0;
    tempPenalty[i]= 0;
    for(byte j= 0; j<24; j++)
      ventings1[i][j]= 0;
      
    sectorHDiff[i]= 0.0;
    curTempPenalty[i]= 0;    
  }
}  

void actionTask() {
  if(testOut==2) return;
  for(byte i= 0; i<N_SECTORS; i++) {
    sectorAction(i);
  }
}

void supervisorTask() {
  if(testOut==2) return;
  for(byte i= 0; i<N_SECTORS; i++) {
    sectorSupervisor(i);
  }
}

void sectorAction(byte sect) {
  if(cycleStep[sect]==0) { // from last steps activation
    tempPenalty[sect]+= curTempPenalty[sect];
    if(!cycleAbort[sect]) {
        ventings24[sect]++;
        ventings1[sect][ventings1Idx]++;
    }
  }
  
  sectorHDiff[sect]= calcSectorHDiff(sect);
  curTempPenalty[sect]= calcTempPenalty(sect);
  sectorError[sect]= calcSectorError(sect);

  if(cycleStep[sect]==-1) {
    thresholdFactor[sect]*= THRESHOLD_TIME_CONST;
    if(thresholdFactor[sect]<MIN_THRESHOLD_NORM) thresholdFactor[sect]= MIN_THRESHOLD_NORM;
    if(tempPenalty[sect]>0) tempPenalty[sect]--;
  } else { 
    cycleStep[sect]++;
    thresholdFactor[sect]*= THRESHOLD_TIME_CONST;
    thresholdFactor[sect]+= 1.0-THRESHOLD_TIME_CONST;
    
    if(cycleStep[sect]==VENT_CYCLES) {
      if(!cycleAbort[sect])  
        stopVenting(sect);
      actionTime= 0;
    } else if(cycleStep[sect]==(VENT_CYCLES+SETTLE_CYCLES)) {
      cycleStep[sect]= -1;  // venting cycle is finished
      cycleAbort[sect]= false;
      actionTime= 0;
    }
  }
  if(cycleStep[sect]==-1 && ventCondition(sect)) {
    limitHumidity[sect]= getSectorHumidity(sect)+2.0; 
    startVenting(sect);
    cycleAbort[sect]= false;
    cycleStep[sect]= 0;
    actionTime= 0;
  }
}

void sectorSupervisor(byte sect) {
  if(cycleStep[sect]==-1 || cycleStep[sect]>=VENT_CYCLES || cycleAbort[sect])
      return;
  
  if(getSectorCurrentHumidity(sect)>limitHumidity[sect]) {
      cycleAbort[sect]= true;
      stopVenting(sect);
      actionTime= 0;
  }
  limitHumidity[sect]-= 0.1;
}

boolean ventCondition(byte sect) {
  return sectorHDiff[sect]>MAX_THRESHOLD*thresholdFactor[sect]
    && ventings24[sect]<MAX_VENTINGS24
    && (tempPenalty[sect]+curTempPenalty[sect])<=TEMP_PENALTY_THRESHOLD
    && sectorError[sect]==0;
}
float calcSectorHDiff(byte sect) {
    if(sect>=N_SECTORS) return 13.3;
    
    float hi= 0.0;
    float n= 0.0;
    for(int8_t s= 0; s<N_SENSORS; ++s) {
        int8_t idx= sectorSensors[sect][s];
        if(idx<0 || dhtInside[idx]==0) continue;
        
        hi+= meanAbsHumidity[idx];
        n+= 1.0;
    }
    if(n==0.0) return 13.3;
    
    hi/= n;
    
    float ho= 0.0;
    n= 0.0;
    for(int8_t s= 0; s<N_SENSORS; ++s) {
        int8_t idx= sectorSensors[sect][s];
        if(idx<0 || dhtInside[idx]==1) continue;
        
        ho+= meanAbsHumidity[idx];
        n+= 1.0;
    }
    if(n==0.0) return 13.3;
    
    ho/= n;
    
    return hi - ho;
}
float calcSectorTDiff(byte sect) {
  if(sect>=N_SECTORS) return 13.3;
  
  float temp= 0.0;
  float n= 0.0;
  for(int8_t s= 0; s<N_SENSORS; ++s) {
      int8_t idx= sectorSensors[sect][s];
      if(idx<0 || dhtInside[idx]==1) continue;
      
      temp+= meanTemp[idx];
      n+= 1.0;
  }
  if(n==0.0) return 13.3;
  temp/= n;
  
  targetTempVal= targetTemp();
  temp-= targetTempVal;
  if(temp>2.0) temp-= 2.0;
  else if(temp<2.0) temp+= 2.0;
  else temp= 0.0;
  return temp;  
}

float targetTemp() {
  if(timeStatus() == timeNotSet)
    return 12.0;
  else {
    time_t t = now();
    return 14.5 - cos((month(t)-1)/12.0*2*PI)*2.5;
  }
}
float getSectorHumidity(byte sect) {
    if(sect>=N_SECTORS) return 0.0;
    
    float h= 0.0;
    float n= 0.0;
    for(int8_t s= 0; s<N_SENSORS; ++s) {
        int8_t idx= sectorSensors[sect][s];
        if(idx<0 || dhtInside[idx]==0) continue;
        
        h+= meanHumidity[idx];
        n+= 1.0;
    }
    if(n==0.0) return 0.0;
    
    return h/n;
}
float getSectorCurrentHumidity(byte sect) {
    if(sect>=N_SECTORS) return 0.0;
    
    float h= 0.0;
    float n= 0.0;
    for(int8_t s= 0; s<N_SENSORS; ++s) {
        int8_t idx= sectorSensors[sect][s];
        if(idx<0 || dhtInside[idx]==0) continue;
        
        h+= currentHumidity[idx];
        n+= 1.0;
    }
    if(n==0.0) return 0.0;
    
    return h/n;
}
DHT22_ERROR_t calcSectorError(byte sect) {
    if(sect>=N_SECTORS) return DHT_ERROR_NOT_PRESENT;
    
    DHT22_ERROR_t e= DHT_ERROR_NONE;
    bool any= false;
    for(int8_t s= 0; s<N_SENSORS; ++s) {
        int8_t idx= sectorSensors[sect][s];
        if(idx<0) continue;
        
        e= max(e, dht22ErrorCode[idx]);
        any= true;
    }
    if(!any) e= DHT_ERROR_NOT_PRESENT;
    
    return e;
}

byte calcTempPenalty(byte sect) {
  float penFact= (min(getSectorHumidity(sect), 90.0)-70.0)*(PENALTY_AT_90-PENALTY_AT_70)/20 + PENALTY_AT_70;
  float tempPen= (fabs(calcSectorTDiff(sect))+0.5)*penFact;
  float hDiff= max(min(sectorHDiff[sect], MAX_THRESHOLD), MIN_THRESHOLD);
  return lround(tempPen/hDiff);
//  return lround(fabs(calcSectorTDiff(sect))*PENALTY_PER_DEGREE);
}

void updateVentings24() {
  ventings1Idx++;
  if(ventings1Idx>=24) ventings1Idx= 0;
  for(byte i= 0; i<N_SECTORS; i++) {
    ventings24[i]-= ventings1[i][ventings1Idx];
    ventings1[i][ventings1Idx]= 0;
  }
}
