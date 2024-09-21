
#include <Arduino.h>
#include <EEPROM.h>
#include <esp_task_wdt.h>
#include <WiFi.h>
#include "dto.h"

void httpLoop();

WiFiServer server(80);  


void saveLoginData(String ssid, String pass) {
    ssid.toCharArray(sysSetupStruc.ssid, sizeof(sysSetupStruc.ssid));
    pass.toCharArray(sysSetupStruc.pass, sizeof(sysSetupStruc.pass));
    
    sysSetupStruc.FirstStart = 1;  
    
    
    EEPROM.put(0, sysSetupStruc);
    EEPROM.commit();

    Serial.println("Дані збережені:");
    Serial.print("SSID: ");
    Serial.println(sysSetupStruc.ssid);
    Serial.print("Password: ");
    Serial.println(sysSetupStruc.pass);
    ESP.restart();
}


void ClientSetup(void) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MyController");
    esp_task_wdt_reset();
    
    EEPROM.get(0, sysSetupStruc);

    server.begin(); 

    while(true){

        httpLoop(); 
    }


}

void httpLoop() {
    WiFiClient client = server.available();  

    if (client) {
        Serial.println("New Client."); 
        String header = "";
        String currentLine = "";
        String ssid = "";
        String pass = "";

        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                header += c;

                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();
                        
                        client.println("<!DOCTYPE html><html>");
                    client.println("<head><title>MyController</title>");
                    client.println("<style>");
                    client.println("body { background-color: #f0f8ff; font-family: Arial, sans-serif; }");
                    client.println("h1 { color: #4682b4; }");
                    client.println("form { max-width: 400px; margin: auto; padding: 20px; background-color: #e0ffff; border-radius: 10px; }");
                    client.println("input[type='text'] { width: 100%; padding: 10px; margin: 8px 0; border: 1px solid #4682b4; border-radius: 5px; }");
                    client.println("input[type='submit'] { background-color: #32cd32; color: white; padding: 10px 15px; border: none; border-radius: 5px; cursor: pointer; }");
                    client.println("input[type='submit']:hover { background-color: #228b22; }");
                    client.println("</style>");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<h1>WiFi Configuration</h1>");
                    client.println("<form action=\"/submit\" method=\"get\">");
                    client.println("SSID: <input type=\"text\" name=\"ssid\"><br>");
                    client.println("Password: <input type=\"text\" name=\"pass\"><br>");
                    client.println("<input type=\"submit\" value=\"Save\">");
                    client.println("</form>");
                    client.println("</body></html>");

                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }
        if (header.indexOf("GET /submit") >= 0) {
            int ssidIndex = header.indexOf("ssid=") + 5;
            int passIndex = header.indexOf("pass=") + 5;

            ssid = header.substring(ssidIndex, header.indexOf('&', ssidIndex));
            pass = header.substring(passIndex, header.indexOf(' ', passIndex));

            
            saveLoginData(ssid, pass);
        }
        client.stop(); 
        Serial.println("Client Disconnected.");
    }
}