//screen.c
//
#include <Arduino.h>
#include "customBoard.h"
#include <ArduinoJson.h>

#include <LiquidCrystal.h>
#include "screen.h"
#include "FastLED.h"


CRGB leds[LED_STRIP_NUM_LEDS];
SystemSetup sysConfig;



TaskHandle_t secondHwLoop = NULL;
extern tm timeinfo;
uint8_t msgSrc = 0;
String extMessage[6];
energyMeterData energy;

LiquidCrystal My_LCD(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

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

void energyReportJson(String jsonLine){
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, jsonLine);

    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }

    if (doc.containsKey("source") && doc.containsKey("total_power")) {
        const char* source = doc["source"];
        if (strcmp(source, "AC") == 0) {
            energy.source = 0;  // AC - 0
        } else if (strcmp(source, "BAT") == 0) {
            energy.source = 1;  // BAT - 1
        }
        energy.totalPower = doc["total_power"];

        if (doc.containsKey("voltage")) {
            JsonArray voltageArray = doc["voltage"];
            for (uint8_t i = 0; i < 3; i++) {
                energy.voltage[i] = voltageArray[i];
            }
        }
        if (doc.containsKey("current")) {
            JsonArray currentArray = doc["current"];
            for (uint8_t i = 0; i < 3; i++) {
                energy.current[i] = currentArray[i];
            }
        } 
    }
    energy.timestamp = millis();
}

void sysSetupUpdate(SystemSetup data){
    sysConfig = data;
}


void ScreenTaskCreate(void){
    //esp_task_wdt_reset();
    xTaskCreatePinnedToCore(ScreenHandler, "ScreenHandler", 10000, NULL, 10, &secondHwLoop, 1);
}

void ScreenHandler(void *arg){
    My_LCD.begin(20, 2);
    FastLED.addLeds<WS2811, LED_STRIP_PIN, RGB>(leds, LED_STRIP_NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(50);
    pinMode(LED_STRIP_PIN, OUTPUT);
    //leds[0] = CHSV(0, 0, 0);
    //leds[1] = CHSV(0, 0, 0);
    leds[0] = CRGB(0, 0, 0);
    leds[1] = CRGB(0, 0, 0);
    FastLED.show();
    clearScr();
    
while(1){


    if(msgSrc!=0){
        My_LCD.setCursor(0, 0);  //col and row
        My_LCD.print(extMessage[0]);
        My_LCD.setCursor(0, 1);  //col and row
        My_LCD.print(extMessage[1]);
    }else{
        char lcdTimeLine[41];
        char energyLine[41];
      
        if (getLocalTime(&timeinfo)) {
            sprintf(lcdTimeLine, "%02d:%02d:%02d   %02d/%0s", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, monthNames[timeinfo.tm_mon]);
        }else{
            sprintf(lcdTimeLine, "RTC ERR");
        }
        sprintf(energyLine, "%03d,%03d,%03d %04d;%0s  ", energy.voltage[0], energy.voltage[1], energy.voltage[2], energy.totalPower, energy.source == 0 ? "AC" : "BAT");

        My_LCD.setCursor(0, 0);  //col and row
        My_LCD.print(lcdTimeLine);
        My_LCD.setCursor(0, 1);  //col and row
        My_LCD.print(energyLine);

        if((millis() - energy.timestamp) < 10000){
             if(energy.source == 0){
            leds[0] = CRGB(0,sysConfig.ambLightBrightness,0);
        }else{
            leds[0] = CRGB(sysConfig.ambLightBrightness,0,0);
        }
        }else{
            leds[0] = CRGB(sysConfig.ambLightBrightness,sysConfig.ambLightBrightness,0);
        }
       
        leds[1] = CRGB(sysConfig.ambLightColr[0],sysConfig.ambLightColr[1],sysConfig.ambLightColr[2]);
        FastLED.show();
        
    }
        
    //digitalWrite(Server_LED, !digitalRead(Server_LED));
    delay(100);

}
}