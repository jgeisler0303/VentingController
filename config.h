#ifndef CONFIG_H
#define CONFIG_H

#define COSM_CONTENT_LENGHT                     256

#define N_SECTORS 3
#define N_SENSORS 5

#define VENT_CYCLES 1
#define SETTLE_CYCLES 2

#define TEMP_PENALTY_THRESHOLD 60 
#define TEMP_PENALTY_THRESHOLD 60 
#define PENALTY_AT_90 0.0
#define PENALTY_AT_70 (5.0*1.3)

#define MAX_THRESHOLD 2.5
#define MIN_THRESHOLD 0.1

#define THRESHOLD_TIME_CONST 0.9
#define MAX_VENTINGS24 15
#define N_ACTORS_PER_SECTION 3

const int8_t dhtPins[]= {A0, A1, A2, A3, 6};
const int8_t dhtInside[]= {1, 0, 1, 1, 0};
const int8_t ventPins[3][3]= {{8, 9, -1}, {-1, -1, -1}, {-1, -1, -1}};
const int8_t windowOpenPins[3][3]= {{0, 2, 4}, {-1, -1, -1}, {-1, -1, -1}};
const int8_t windowClosePins[3][3]= {{1, 3, 5}, {-1, -1, -1}, {-1, -1, -1}};

#endif
