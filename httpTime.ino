#include <Time.h>

void updateTime() {
  int hr;
  int mn;
  int sec;
  int day;
  int month= 13;
  int yr;
  
  char *c= pDateBuffer;
  day= *c++ - '0';
  if(*c!=' ') {
    day*= 10;
    day+= *c++ - '0';
  } else
    c++;
  
  c++;
  char m2= *c++;
  switch(*c++) {
    case 'b':
      month= 2;
      break;
    case 'c':
      month= 12;
      break;
    case 'g':
      month= 8;
      break;
    case 'l':
      month= 7;
      break;
    case 'n':
      if(m2=='a')
        month= 1;
      else
        month= 6;
      break;
    case 'p':
      month= 9;
      break;
    case 'r':
      if(m2=='p')
        month= 4;
      else
        month= 3;
      break;
    case 't':
      month= 10;
      break;
    case 'v':
      month= 11;
      break;
    case 'y':
      month= 5;
      break;
  }
  
  c++;
  yr= 0;
  for(byte i= 0; i<4; i++) {
    yr*= 10;
    yr+= *c++ - '0';
  }
  
  c++;
  hr= *c++ - '0'; hr*= 10; hr+= *c++ - '0';  
  c++;
  mn= *c++ - '0'; mn*= 10; mn+= *c++ - '0';  
  c++;
  sec= *c++ - '0'; sec*= 10; sec+= *c++ - '0';
  
  #ifdef SERIAL_OUT
  Serial.print(day);
  Serial.print('.');
  Serial.print(month);
  Serial.print('.');
  Serial.print(yr);
  Serial.print(' ');
  Serial.print(hr);
  Serial.print(':');
  Serial.print(mn);
  Serial.print(':');
  Serial.println(sec);
  #endif
  
  setTime(hr,mn,sec,day,month,yr);
}
