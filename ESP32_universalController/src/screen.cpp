//screen.c
//
#include <Arduino.h>
#include <screen.h>
#include <LiquidCrystal.h>
#include "dto.h"

#define NUM_LEDS 2
#include "FastLED.h"
#define LEDS_PIN 13
CRGB leds[NUM_LEDS];



TaskHandle_t secondHwLoop = NULL;
extern tm timeinfo;
uint8_t msgSrc = 0;
String extMessage[6];

//Create An LCD Object. Signals: [ RS, EN, D4, D5, D6, D7 ]
//LiquidCrystal My_LCD(15, 27, 26, 32, 33, 25);
LiquidCrystal My_LCD(27, 14, 26, 32, 33, 25);


const char *monthNames[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

void pullMsg(uint8_t src, String msg, uint8_t line){
    msgSrc = src;
    extMessage[line] = msg;
}
void clearScr(void){
    My_LCD.clear();
}


void ScreenTaskCreate(void){
    //esp_task_wdt_reset();
    xTaskCreatePinnedToCore(ScreenHandler, "ScreenHandler", 10000, NULL, 10, &secondHwLoop, 1);
}

void ScreenHandler(void *arg){
    My_LCD.begin(20, 2);
    FastLED.addLeds<WS2811, LEDS_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(50);
    pinMode(LEDS_PIN, OUTPUT);
    leds[0] = CHSV(100, 255, 100);
    leds[1] = CHSV(200, 255, 100);
    FastLED.show();
    clearScr();
    
while(1){


    if(msgSrc!=0){
        My_LCD.setCursor(0, 0);  //col and row
        My_LCD.print(extMessage[0]);
        My_LCD.setCursor(0, 1);  //col and row
        My_LCD.print(extMessage[1]);
    }else{
        char lcdTimeLine[21];
      
        if (getLocalTime(&timeinfo)) {
            sprintf(lcdTimeLine, "%02d:%02d:%02d   %02d/%0s", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, monthNames[timeinfo.tm_mon]);
        }else{
            sprintf(lcdTimeLine, "RTC ERR");
        }

        My_LCD.setCursor(0, 1);  //col and row
        My_LCD.print(lcdTimeLine);
    }
        
    //digitalWrite(Server_LED, !digitalRead(Server_LED));
    delay(100);

}
}