#include "WebManager.h"
#include "Globals.h"
#include <ESP8266WebServer.h>
#include <LittleFS.h>

ESP8266WebServer server(80);

void handleRoot() {
  if (!LittleFS.exists("/index.html")) {
    server.send(200, "text/plain",
                "Error: /index.html not found. Please upload filesystem!");
    return;
  }

  File f = LittleFS.open("/index.html", "r");
  if (!f) {
    server.send(500, "text/plain", "Error opening /index.html");
    return;
  }

  String html = f.readString();
  f.close();

  // Inject Dynamic Data
  html.replace("%HEAP%", String(ESP.getFreeHeap() / 1024.0));
  html.replace("%FLASH%", String(ESP.getFlashChipSize() / 1024.0));
  html.replace("%COLOR%", String(hexColor));

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

// Global file handle for upload
File uploadFile;

void handleUpload() {
  Serial.println("Upload started");
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (filename.endsWith(".mjpeg")) {
      filename = "/anim.mjpeg";
      if (LittleFS.exists("/background.jpg"))
        LittleFS.remove("/background.jpg");
    } else {
      filename = "/background.jpg";
      if (LittleFS.exists("/anim.mjpeg"))
        LittleFS.remove("/anim.mjpeg");
    }
    Serial.print("Upload Start: ");
    Serial.println(filename);

    // Open file
    uploadFile = LittleFS.open(filename, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile)
      uploadFile.close();
    Serial.print("Upload Size: ");
    Serial.println(upload.totalSize);
    server.send(200, "text/plain", "OK");
    delay(500);
    ESP.restart();
  }
}

void setupWeb() {
  LittleFS.begin(); // Make sure FS is started
  server.on("/", handleRoot);
  server.on("/set", HTTP_POST, handleSet);

  server.on("/upload", HTTP_POST, []() { server.send(200); }, handleUpload);

  server.begin();
  Serial.println("HTTP server started");
}

void loopWeb() { server.handleClient(); }
