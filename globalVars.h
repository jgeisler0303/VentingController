#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include <DHT22.h>
#include "config.h"
#include <Time.h>

extern int8_t cycleStep[N_SECTORS];
extern bool cycleAbort[N_SECTORS];
extern float meanTemp[];
extern float meanHumidity[];
extern float currentHumidity[N_SENSORS];
extern float meanAbsHumidity[];
extern float limitHumidity[];
extern DHT22_ERROR_t dht22ErrorCode[];
extern float targetTempVal;

extern int actionTime;
extern byte counterSec;
extern  byte ventings1[N_SECTORS][24];
extern float thresholdFactor[N_SECTORS];
extern byte tempPenalty[N_SECTORS];
extern byte ventings1Idx;
extern byte ventings24[N_SECTORS];

extern char pDateBuffer[];

extern uint16_t gas10minCounter;
extern uint32_t gasTotalCounter;

extern uint16_t rain10minCounter;
extern uint16_t door10minCounter;

extern int lastCosmError;
extern time_t lastErrorTime;


#endif

