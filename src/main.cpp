#include "DisplayManager.h"
#include "Globals.h"
#include "WebManager.h"
#include "WifiManager.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);

  // 1. Setup Screen & Time
  setupDisplay();

  // 2. Connect to WiFi
  setupWiFi();

  // 2.1 Init Time (NTP)
  // Timezone for Hermosillo (Mountain Standard Time, no DST usually)
  // UTC offset: -7 hours * 3600 seconds = -25200
  configTime(-25200, 0, "pool.ntp.org", "time.nist.gov");

  // 3. Start Web Server
  setupWeb();

  // 4. Restore Background (overwriting WiFi status)
  drawBackground();
}

void loop() {
  loopWeb();
  loopDisplay();
}