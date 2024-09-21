//main.cpp
#include <Arduino.h>
#include <esp_task_wdt.h>
#include "dto.h"
#include <EEPROM.h>

#include <Wire.h>

#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <time.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>


#include "screen.h"
#include "setup.h"
#include "html_page.h"


const char* ntpServers[] = {"pool.ntp.org", "time.google.com", "time.windows.com"};

const long utcOffsetInSeconds = 3600*3;  // Зміщення часу для вашого часового поясу (3600 = +1 година)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServers[0], utcOffsetInSeconds);

WiFiUDP udp;
const char* udpAddress = "192.168.1.255";  // IP-адреса отримувача

const int udpRxPort = 5006;  // UDP RX port
char incomingPacket[1024];
SystemSetup sysSetupStruc;
HTTPClient client;

String sysStatus;
tm timeinfo;

WiFiServer serverConfigured(80);

uint16_t jobPrescaler;
uint16_t lcdPrescaler;




void setup() {
  
  pinMode(Wifi_LED, OUTPUT);
  pinMode(Server_LED, OUTPUT);
  pinMode(Relay, OUTPUT);
  pinMode(Dio, OUTPUT);
  pinMode(in1, INPUT);
  pinMode(in2, INPUT);
  pinMode(userBtn, INPUT);

  ScreenTaskCreate();
  pullMsg(1, "Wait power stable", 0);
  delay(2000);
  
  Serial.begin(115200);

  //EEPROM
  EEPROM.begin(sizeof(sysSetupStruc)+1);
  EEPROM.get(0, sysSetupStruc);
  //EEPROM clear if first start or user request
  if(digitalRead(userBtn) == 0 ) { // Checking if is it a first start or manual reset request
    for(int i = 0; i < 20; i++) sysSetupStruc.ssid[i] = 0;
    for(int i = 0; i < 20; i++) sysSetupStruc.pass[i] = 0;
    sysSetupStruc.FirstStart = 0;   
    EEPROM.put(0, sysSetupStruc);             
    EEPROM.commit();   
  }
   
  Serial.println(sysSetupStruc.FirstStart);
  
  if (sysSetupStruc.FirstStart == 0){
    
    pullMsg(1, "* Config mode  *", 0);
    pullMsg(1, " IP:192.168.4.1 ", 1);
    ClientSetup();
  }
    
    pullMsg(1, "Connecting to:", 0);
    pullMsg(1, sysSetupStruc.ssid, 1);

    Serial.print("Connect to: ");
    Serial.println(sysSetupStruc.ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(sysSetupStruc.ssid, sysSetupStruc.pass);

    for (byte cnonectionCNT = 0; WiFi.status() != WL_CONNECTED; cnonectionCNT ++) { // пробуємо підключитися до wi-fi
      vTaskDelay(500);
      Serial.print(".");
      if(cnonectionCNT > 40){
        pullMsg(1, "Connection Fail ", 0);
        sysStatus = "Connection Fail";
        break;
      }
    }
    Serial.println("");

    if(WiFi.status() == WL_CONNECTED){
      sysStatus = "Connected";
      clearScr();
      IPAddress ip = WiFi.localIP();      
      char charBuff[41];
      sprintf(charBuff, "WiFi: %s", sysSetupStruc.ssid);
      pullMsg(1, charBuff, 0);
      sprintf(charBuff, "IP: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      pullMsg(1, charBuff, 1);
   
      Serial.println(WiFi.localIP());
      udp.begin(udpRxPort);
    }

    delay(2000);
    pullMsg(0, "", 0);
    udp.begin(udpRxPort);

    serverConfigured.begin();

}

void loop() {

 WiFiClient client = serverConfigured.available();  // Очікуємо нових клієнтів
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    String header = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            if (header.indexOf("GET /settings") >= 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:application/json");
              client.println("Connection: close");
              client.println();

              StaticJsonDocument<500> jsonDoc;
              jsonDoc["ntpServer"] = sysSetupStruc.ntpServerIndex;
              jsonDoc["timezone"] = sysSetupStruc.ntpTimeZone;
              jsonDoc["manualTime"] = sysSetupStruc.ntpEN ? "on" : "off";
              jsonDoc["datetime"] = ""; 
              char colorBuffer[10];  
              sprintf(colorBuffer, "#%02X%02X%02X", sysSetupStruc.ambLightColr[0], sysSetupStruc.ambLightColr[1], sysSetupStruc.ambLightColr[2]);
              jsonDoc["ambiLightColor"] = colorBuffer;
              jsonDoc["brightness"] = sysSetupStruc.ambLightBrightness;
              jsonDoc["ambiLight"] = sysSetupStruc.ambLightEnable ? "on" : "off";
              String jsonResponse;
              serializeJson(jsonDoc, jsonResponse);
              client.println(jsonResponse);
            } else if (header.indexOf("POST /submit") >= 0) {

              String requestBody = "";
              while (client.available()) {
                char c = client.read();
                requestBody += c;
              }
              Serial.println(requestBody);
              StaticJsonDocument<500> jsonDoc;
              deserializeJson(jsonDoc, requestBody);

              if (jsonDoc.containsKey("ntpServer")) {
                sysSetupStruc.ntpServerIndex = jsonDoc["ntpServer"];
              }
              if (jsonDoc.containsKey("timezone")) {
                sysSetupStruc.ntpTimeZone = jsonDoc["timezone"];
              }
              if (jsonDoc.containsKey("manualTime")) {
                sysSetupStruc.ntpEN = (jsonDoc["manualTime"] == "on") ? 1 : 0;
              }
              if (jsonDoc.containsKey("ambiLightColor")) {
                String color = jsonDoc["ambiLightColor"];
                sysSetupStruc.ambLightColr[0] = strtol(color.substring(1, 3).c_str(), NULL, 16);
                sysSetupStruc.ambLightColr[1] = strtol(color.substring(3, 5).c_str(), NULL, 16);
                sysSetupStruc.ambLightColr[2] = strtol(color.substring(5, 7).c_str(), NULL, 16);
              }
              if (jsonDoc.containsKey("brightness")) {
                sysSetupStruc.ambLightBrightness = jsonDoc["brightness"];
              }
              if (jsonDoc.containsKey("ambiLight")) {
                sysSetupStruc.ambLightEnable = (jsonDoc["ambiLight"] == "on") ? 1 : 0;
              }
              EEPROM.put(0, sysSetupStruc);
              EEPROM.commit();

              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/plain");
              client.println("Connection: close");
              client.println();
              client.println("Settings updated");
            } else {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              client.println(index_html); 
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected.");
  }
  
  if(jobPrescaler == 0){
    if (WiFi.status() == WL_CONNECTED) {  
      
      digitalWrite(Wifi_LED, HIGH);
      Serial.println("Connected");
      timeClient.update();
      Serial.print("Current time (NTP): ");
      Serial.println(timeClient.getFormattedTime());
      configTime(utcOffsetInSeconds, 0, ntpServers[0]);
      
      if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
      }
        Serial.println(&timeinfo, "Current time (RTC): %H:%M:%S");
            
        //const char *message = "Hello, UDP!";
        //udp.beginPacket(udpAddress, udpTxPort);
        //udp.write((const uint8_t*)message, strlen(message));
        //udp.endPacket();
      }else{
        digitalWrite(Wifi_LED, LOW);
        Serial.println("Connection FAIL");
      }
  }


  jobPrescaler++;
  if(jobPrescaler >= 1500){
    jobPrescaler=0;
  }

  if(WiFi.status() == WL_CONNECTED){
    int packetSize = udp.parsePacket();  
    if (packetSize) {
      //Serial.printf("Received packet of size %d\n", packetSize);
      int len = udp.read(incomingPacket, 255); 
      if (len > 0) {
        incomingPacket[len] = 0; 
      }
      Serial.printf("Received: %s\n", incomingPacket);

    
    }
  }
 
  delay(1);
}

