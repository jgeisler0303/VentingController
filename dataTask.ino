// DONE
// rain sensor

#include <DHT22.h>
#include "config.h"

DHT22 dht22;
//DHT22* dht22= static_cast<DHT22*>( ::operator new ( sizeof(DHT22) * N_SENSORS ) );
float currentTemp[N_SENSORS];
float currentHumidity[N_SENSORS];
float currentAbsHumidity[N_SENSORS];
float meanTemp[N_SENSORS];
float meanHumidity[N_SENSORS];
float meanAbsHumidity[N_SENSORS];
int meanCount[N_SENSORS];
DHT22_ERROR_t dht22CurrentErrorCode[N_SENSORS];
DHT22_ERROR_t dht22ErrorCode[N_SENSORS];

void dataTaskInit() {
  for(int i= 0; i<N_SENSORS; i++) {
    dht22CurrentErrorCode[i]= DHT_ERROR_TOOQUICK;
    dht22ErrorCode[i]= DHT_ERROR_NOT_PRESENT;
    //if(dhtPins[i]<0) continue;
    
    //dht22[i]= DHT22(dhtPins[i]);
    //new(static_cast<void*>(dht22+i)) DHT22(dhtPins[i]);
  }
}

void dataTask() {
  for(int i= 0; i<N_SENSORS; i++) {
    if(dhtPins[i]<0) continue;
    
    dht22.setPin(dhtPins[i]);
    DHT22_ERROR_t e= dht22.readDataNow();
    if (e==DHT_ERROR_NONE) {
      currentTemp[i]= dht22.getTemperatureC();
      currentHumidity[i]=dht22.getHumidity();
      if(dhtInside[i])
        currentAbsHumidity[i]= massOfWater(currentTemp[i]-1.0, min(90.0, currentHumidity[i]-5.0));
      else
        currentAbsHumidity[i]= massOfWater(currentTemp[i]+2.0, max(50.0, currentHumidity[i]+5.0));
            
      meanTemp[i]+= currentTemp[i];
      meanHumidity[i]+=currentHumidity[i];
      meanAbsHumidity[i]+= currentAbsHumidity[i];
      meanCount[i]++;
    }
    dht22CurrentErrorCode[i]= min(dht22CurrentErrorCode[i], e);
  }
}

void calcMean() {
  for(int i= 0; i<N_SENSORS; i++) {
    if(meanCount[i]==0) {
      meanTemp[i]= 0.0;
      meanHumidity[i]= 0.0;
      meanAbsHumidity[i]= 0.0;
    } else {
      meanTemp[i]/= meanCount[i];
      meanHumidity[i]/= meanCount[i];
      meanAbsHumidity[i]/= meanCount[i];
    }
    dht22ErrorCode[i]= dht22CurrentErrorCode[i];
    dht22CurrentErrorCode[i]= DHT_ERROR_TOOQUICK;
  }  
}

void clearMean() {
  for(int i= 0; i<N_SENSORS; i++) {
    meanTemp[i]= 0.0;
    meanHumidity[i]= 0.0;
    meanAbsHumidity[i]= 0.0;
    meanCount[i]= 0;
  }  
}
