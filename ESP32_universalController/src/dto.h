#ifndef DTO_H
#define DTO_H

extern const char* ntpServers[];

typedef struct{
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
}SystemSetup;

typedef struct
{
    uint8_t source;         //AC - 0, BAT - 1
    uint16_t voltage[3];    //Volt
    uint16_t current[3];    //mAmp
    uint16_t totalPower;    //((L1I+L2I+L3I)/3)*((L1U+L2U+L3U)/3) Average energy, calculated by the server.
    long timestamp;
}energyMeterData;

extern SystemSetup sysSetupStruc;

#endif