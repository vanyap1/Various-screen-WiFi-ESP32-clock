#include <Arduino.h>
#ifndef DTO_H
#define DTO_H



#define Wifi_LED 14
#define Server_LED 12

#define Relay     18
#define Dio       19
#define in1       16
#define in2       17
#define userBtn   4

extern const char* ntpServers[];

struct SystemSetup {
  uint8_t FirstStart;
  bool NodeWasUpdated; 
  char ssid[20];
  char pass[20];
  uint8_t ambLightBrightness;
  uint8_t ambLightEnable;
  uint8_t ambLightColr[3];
  uint8_t ntpEN;
  uint8_t ntpServerIndex;
  uint8_t ntpTimeZone;
  uint8_t screenType;
};

extern SystemSetup sysSetupStruc;

#endif