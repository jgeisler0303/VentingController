// DONE

#include <Ethernet.h>
#include <EthernetClient.h>
#include <SPI.h>
#include "config.h"
#include "XivelyKey.h"
#include <Time.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};


#define SECTOR_ID_OFS 100

int cosmState;
int cosmLength;
int lastCosmError;
time_t lastErrorTime;

void cosmCommInit() {
  Ethernet.begin(mac, ip);
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void cosmCommSend() {
  if(cosmState= cosmUpdateStart()) return;
  
  int n= 0;
  int idOfs= 0;
  for(int i= 0; i<N_SENSORS; i++, idOfs+= 10) {
    if(dhtPins[i]<0) continue;
    
    if(dht22ErrorCode[i]==DHT_ERROR_NONE) {
      n+= sendCosmData(1+idOfs, meanTemp[i], 1); // temp -10.0
      n+= sendCosmData(2+idOfs, meanHumidity[i], 1); // humidity 100.0
    } else {
      n+= sendCosmData(1+idOfs, (float)dht22ErrorCode[i], 1); // temp -10.0
      n+= sendCosmData(2+idOfs, -13.0, 1); // humidity 100.0
    }
    // max 8*2*(3+5+2)=16/160
  }

  idOfs= SECTOR_ID_OFS;
  for(int i= 0; i<N_SECTORS; i++, idOfs+= 10) {
    if(!activeSector[i]) continue;
    
    float thr= max(MAX_THRESHOLD*thresholdFactor[i], MIN_THRESHOLD);
    n+= sendCosmData(0+idOfs, sectorHDiff[i], 1); // diff 10.1
    n+= sendCosmData(1+idOfs, thr, 1); // threshold 10.1
    n+= sendCosmData(2+idOfs, tempPenalty[i]); // temp limiter
    n+= sendCosmData(3+idOfs, ventings24[i]); // 24h max limit
    if(sectorError[i])
      n+= sendCosmData(4+idOfs, sectorError[i]+100); // actor on -1
    else 
      n+= sendCosmData(4+idOfs, cycleStep[i]); // actor on -1
    n+= sendCosmData(5+idOfs, curTempPenalty[i]); // 24h max limit

//    n+= sendCosmData(1000, freeRam()); 

    // max 3*(4+4+2+3+2+2+5*(3+2))=15/126
  }
  n+= sendCosmData(1000, gas10minCounter);
  n+= sendCosmData(1001, rain10minCounter);
  n+= sendCosmData(1002, door10minCounter);
  
  cosmLength= n;
  cosmState= cosmUpdateFinish(n);
}

#define HTTP_WAITING_TIMEOUT 4000 // milliseconds, 

byte remoteServer[] = { 173,203,98,29 };            // api.pachube.com
EthernetClient localClient;

//#define bufferSIZE 64
//char pDataBuffer[bufferSIZE];
char pDateBuffer[]= "XXXXXXXXXXXXXXXXXXXX";

char pHttpBegin[]  = "HTTP/1.1";
char pDateBegin[]  = "Date: ";

#define http_get_header F("GET /v2/feeds/")
#define http_host_key_header F(".csv HTTP/1.1\nHost: api.pachube.com\nX-PachubeApiKey: ")
#define http_put_header F("PUT /v2/feeds/")
#define http_content_header F("\nContent-Type: text/csv\nContent-Length: ")
#define http_connection_header F("\nConnection: close\n\n")

int cosmUpdateStart() {
  if(!(localClient.connect(remoteServer, 80) > 0))
    return 1;

// Send PUT request. API v2
  localClient.print(http_put_header/*"PUT /v2/feeds/"*/);
  localClient.print(COSM_FEED_ID);
  localClient.print(http_host_key_header/*".csv HTTP/1.1\nHost: api.pachube.com\nX-PachubeApiKey: "*/);
  localClient.print(COSM_API_KEY);
  //sendToServer(("\nUser-Agent: Arduino (Pachube Out)"));
  localClient.print(http_content_header/*"\nContent-Type: text/csv\nContent-Length: "*/);
  localClient.print(COSM_CONTENT_LENGHT);
  localClient.print(http_connection_header/*"\nConnection: close\n\n"*/);
  return 0;
}

int cosmUpdateFinish(int bytesSent) {
  if(bytesSent>COSM_CONTENT_LENGHT) {
    localClient.stop();
    return 5;
  }
  for(int i= bytesSent; i<COSM_CONTENT_LENGHT;)
    if(++i==COSM_CONTENT_LENGHT)
      localClient.print('\n');
    else
      localClient.print(' ');
  
  int httpStatusCode = waitForRepsonse();
  localClient.stop();

  // Return the result.
  return httpStatusCode;
}

byte sendCosmData(uint16_t id, int v) {
  byte n= 0;
  n+= localClient.print(id);
  n+= localClient.print(',');
  n+= localClient.print(v);
  n+= localClient.print('\n');
  return n;
}

byte sendCosmData(uint16_t id, float v, byte prec) {
  byte n= 0;
  n+= localClient.print(id);
  n+= localClient.print(',');
  n+= localClient.print(v, prec);
  n+= localClient.print('\n');
  return n;
}

// Return the http status code
int waitForRepsonse() {
  // Wait for response.
  int i= 0;
  // DEBUG_PRINT_LN_p(debug_waitForRepsonse/*"Waiting for response."*/);
  while(!localClient.available()) {
    /*if(i%10 == 0) {
      DEBUG_PRINT(i);
      if(i%100 == 0)
        DEBUG_PRINT('\n');
      else
        DEBUG_PRINT(',');
    }*/
    delay(1);
    i++;
    if(i > HTTP_WAITING_TIMEOUT) {
      //DEBUG_PRINT_LN_p(debug_timeout/*"time out"*/);
      return 2;
    }
  }
  //DEBUG_PRINT('\n');

  unsigned int bufferSize= 50;
  char pBuffer[bufferSize];

  int httpStatusCode= 0; // No http status code.
  boolean dateFound= false;

  while(localClient.available() && !dateFound) {
    unsigned int length= getline(pBuffer, bufferSize);
    //DEBUG_PRINT(pBuffer);
    if(length < 8)
      continue;

    if(0 == httpStatusCode) {
      httpStatusCode = getHttpStatusCodeFromHeader(pBuffer);
//      if(200 == httpStatusCode) 
//        DEBUG_PRINT_LN_p(debug_success/*"SUCCESS"*/);        
//      else
//        DEBUG_PRINT_LN_p(debug_fail/*"FAIL"*/);
    } else
        dateFound= getDateFromHeader(pBuffer);
  } 

  return (dateFound)? httpStatusCode: 4;
}

int getHttpStatusCodeFromHeader(char* pFirstLineOfHttpHeader) {
  //DEBUG_PRINT_LN_p(debug_getHttpStatusCodeFromHeader/*"getHttpStatusCodeFromHeader"*/);

  char* pReferenceChar = pHttpBegin;    
  char* pIter = pFirstLineOfHttpHeader;

  for( ; *pReferenceChar != '\0' && *pIter != '\0'; ++pIter, ++pReferenceChar) {
    if(*pReferenceChar!=*pIter)
      return 0; // Not match
  }

  if('\0'==*pIter)
    return 0; // The input string is too short.

  while(*pIter==' ') // Remove the leading space.
    pIter++;

  int httpStatusCode= atoi(pIter);  // Get the first integer.
  return httpStatusCode;    
}

boolean getDateFromHeader(char* pDateLineOfHttpHeader) {
  char* pReferenceChar= pDateBegin;    
  char* pIter= pDateLineOfHttpHeader;

  for( ; *pReferenceChar!='\0' && *pIter!='\0'; ++pIter, ++pReferenceChar) {
    if(*pReferenceChar!=*pIter)
      return false; // Not match
  }
    
  byte i= 0;
  do {
    if('\0'==*pIter) return false; // The input string is too short.
    pIter++;
    i++;
  } while(i<6); // skip space, three letter week day, comma and space

  i= 0;
  do {
    if('\0'==*pIter) return false; // The input string is too short.
    pDateBuffer[i]= *pIter;
    pIter++;
    i++;
  } while(i<20); // date is 20 char long

  return true;    
}

unsigned int getline(char *buffer, unsigned int bufsize) {
  unsigned int length = 0;
  char c;
  while (localClient.available()) {
    c= localClient.read();
    if(length < bufsize - 1){ // The last one is for '\0'
      buffer[length]= c;
      length++;
    } else {
      break; // No buffer available
    }

    if(length>=2) {
      // Note: we can NOT use && here. In some cases, there is only an '\n'.
      if(buffer[length-2]=='\r' || buffer[length-1]=='\n') 
        break; // Line finish
    }
  }

  buffer[length]= '\0';
  return length;
}

