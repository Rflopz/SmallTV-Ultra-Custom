#include <ESP8266WebServer.h>
#include "Globals.h"
#include "WebManager.h"

ESP8266WebServer server(80);

void handleRoot() {
  String html = F("<!DOCTYPE html><html><head><title>SmallTV Config</title></head><body>");
  html += F("<h1>SmallTV Ultra</h1>");
  
  // System Info
  html += F("<h3>System Info</h3>");
  html += F("<p>Heap: "); html += String(ESP.getFreeHeap() / (1024.0)); html += F(" KB</p>");
  html += F("<p>Flash: "); html += String(ESP.getFlashChipSize() / (1024.0)); html += F(" KB</p>");

  // Settings Form
  html += F("<h3>Settings</h3>");
  html += F("<form action='/set' method='POST'>");
  html += F("Time Color: <input type='color' name='c' value='");
  html += String(hexColor);
  html += F("'><br><br>");
  html += F("<input type='submit' value='Save Settings'>");
  html += F("</form>");

  html += F("</body></html>");
  server.send(200, "text/html", html);
}

void handleSet() {
  if (server.hasArg("c")) {
    String c = server.arg("c");
    c.toCharArray(hexColor, 8);
    
    // Parse Hex to RGB565
    // Format: #RRGGBB
    long number = strtol(&hexColor[1], NULL, 16);
    int r = number >> 16;
    int g = (number >> 8) & 0xFF;
    int b = number & 0xFF;
    
    timeColor = tft.color565(r, g, b);
  }
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void setupWeb() {
  server.on("/", handleRoot);
  server.on("/set", HTTP_POST, handleSet);
  server.begin();
  Serial.println("HTTP server started");
}

void loopWeb() {
  server.handleClient();
}
