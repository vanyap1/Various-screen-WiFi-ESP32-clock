//screen.h
#ifndef MY_SCREEN_H
#define MY_SCREEN_H
#include "dto.h"

void pullMsg(uint8_t src, String msg, uint8_t line);
void clearScr(void);
void energyReportJson(String jsonLine);
void sysSetupUpdate(SystemSetup data);
void ScreenTaskCreate(void);
void ScreenHandler(void *arg);

#endif