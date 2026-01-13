#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include "Globals.h"
#include "WebManager.h"

ESP8266WebServer server(80);

void handleRoot() {
  String html = F("<!DOCTYPE html><html><head><title>SmallTV Config</title>");
  html += F("<script>");
  html += F("function u(){");
  html += F("var f=document.getElementById('f').files[0];");
  html += F("if(!f)return;");
  html += F("var r=new FileReader();");
  html += F("r.onload=function(e){");
  html += F("var i=new Image();");
  html += F("i.onload=function(){");
  html += F("var c=document.createElement('canvas');");
  html += F("c.width=240;c.height=240;");
  html += F("var x=c.getContext('2d');");
  html += F("x.drawImage(i,0,0,240,240);");
  html += F("c.toBlob(function(b){");
  html += F("var d=new FormData();");
  html += F("d.append('f',b,'bg.jpg');");
  html += F("var xh=new XMLHttpRequest();");
  html += F("xh.open('POST','/upload');");
  html += F("xh.send(d);");
  html += F("xh.onload=function(){setTimeout(function(){alert('Uploaded! Rebooting...');location.reload();}, 1000);};");
  html += F("},'image/jpeg',0.8);");
  html += F("};i.src=e.target.result;");
  html += F("};r.readAsDataURL(f);");
  html += F("}");
  html += F("</script>");
  html += F("</head><body>");
  
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

  // Upload Form
  html += F("<h3>Background Image</h3>");
  html += F("<input type='file' id='f' accept='image/*'><br><br>");
  html += F("<button onclick='u()'>Upload & Resize</button>");

  html += F("</body></html>");
  server.send(200, "text/html", html);
}

void handleSet() {
  if (server.hasArg("c")) {
    String c = server.arg("c");
    c.toCharArray(hexColor, 8);
    
    // Parse Hex to RGB565
    long number = strtol(&hexColor[1], NULL, 16);
    int r = number >> 16;
    int g = (number >> 8) & 0xFF;
    int b = number & 0xFF;
    
    timeColor = tft.color565(r, g, b);
  }
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleUpload() {
  Serial.println("Upload started");
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/background.jpg";
    Serial.print("Upload Start: "); Serial.println(filename);
    File f = LittleFS.open(filename, "w");
    f.close();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (upload.currentSize == 0) return;
    File f = LittleFS.open("/background.jpg", "a");
    if (f) {
      f.write(upload.buf, upload.currentSize);
      f.close();
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.print("Upload Size: "); Serial.println(upload.totalSize);
    server.send(200, "text/plain", "OK");
    delay(500);
    ESP.restart(); // Reboot to reload everything cleanly
  }
}

void setupWeb() {
  LittleFS.begin(); // Make sure FS is started
  server.on("/", handleRoot);
  server.on("/set", HTTP_POST, handleSet);
  
  server.on("/upload", HTTP_POST, [](){ server.send(200); }, handleUpload);

  server.begin();
  Serial.println("HTTP server started");
}

void loopWeb() {
  server.handleClient();
}
