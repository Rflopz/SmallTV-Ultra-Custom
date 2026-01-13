#include <Arduino.h>
#include "Globals.h"
#include "WifiManager.h"
#include "WebManager.h"
#include "DisplayManager.h"

void setup() {
  Serial.begin(115200);

  // 1. Setup Screen & Time
  setupDisplay();

  // 2. Connect to WiFi
  setupWiFi();

  // 3. Start Web Server
  setupWeb();
}

void loop() {
  loopWeb();
  drawClock();
}