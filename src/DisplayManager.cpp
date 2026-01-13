#include <Arduino.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <TJpg_Decoder.h>
#include "Globals.h"
#include "DisplayManager.h"

// Timezone for Hermosillo
#define UTC_OFFSET -25200
#define DST_OFFSET 0

// Callback for TJpg_Decoder
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void drawBackground() {
  if (LittleFS.exists("/background.jpg")) {
    File f = LittleFS.open("/background.jpg", "r");
    Serial.print("Found background.jpg, size: ");
    Serial.println(f.size());
    f.close();

    Serial.println("Drawing background...");
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);

    uint16_t w = 0, h = 0;
    // Use the *Fs* variants of the functions to pass the filesystem
    TJpgDec.getFsJpgSize(&w, &h, "/background.jpg", LittleFS);
    Serial.printf("JPG Size: %dx%d\n", w, h);
    
    TJpgDec.drawFsJpg(0, 0, "/background.jpg", LittleFS);
  } else {
    Serial.println("No background.jpg found");
    tft.fillScreen(TFT_BLACK);
  }
}

void setupDisplay() {
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Backlight ON
  tft.init();
  tft.setRotation(0); 
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Init LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
  } else {
    drawBackground();
  }

  // Init Time (NTP) - Moved to main.cpp after WiFi
  // configTime(UTC_OFFSET, DST_OFFSET, "pool.ntp.org", "time.nist.gov");
  
  // --- SYSTEM INFO ---
  Serial.println("\n\n--- SYSTEM INFO ---");
  Serial.printf("Heap: %.2f KB\n", ESP.getFreeHeap() / (1024.0));
  Serial.printf("Flash: %.2f KB\n", ESP.getFlashChipSize() / (1024.0));
  Serial.printf("Screen: %dx%d\n", tft.width(), tft.height());
  Serial.println("-------------------\n");
}

// State for minute-based updates
int lastMinute = -1;

void drawClock() {
  time_t now = time(nullptr);
  struct tm *p_tm = localtime(&now);

  // Only draw if we have a valid time (year > 2000)
  if (p_tm->tm_year + 1900 > 2000) {

    // Only update if the minute has changed
    if (p_tm->tm_min != lastMinute) {
      lastMinute = p_tm->tm_min;

      // 1. Clear Screen (Redraw Background)
      drawBackground();

      // 2. Draw Time (Transparent)
      tft.setTextColor(timeColor); // Transparent
      tft.setTextSize(5);
      
      char timeStr[6];
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", p_tm->tm_hour, p_tm->tm_min);
      int timeWidth = tft.textWidth(timeStr);
      tft.setCursor((tft.width() - timeWidth) / 2, 80);
      tft.print(timeStr);

      // 3. Draw Date (Transparent)
      tft.setTextSize(2);
      tft.setTextColor(TFT_YELLOW); // Transparent
      
      char dateStr[16];
      snprintf(dateStr, sizeof(dateStr), "%d/%d/%d", p_tm->tm_mday, p_tm->tm_mon + 1, p_tm->tm_year + 1900);
      int dateWidth = tft.textWidth(dateStr);
      tft.setCursor((tft.width() - dateWidth) / 2, 140);
      tft.print(dateStr);

      // 4. Draw IP (Transparent)
      tft.setTextColor(TFT_WHITE); // Transparent
      tft.setTextSize(1);
      tft.setCursor(20, 20);
      tft.println(WiFi.localIP());
    }

    delay(200); // Check 5 times a second
  } else {
    // Still waiting for NTP sync
    Serial.println("Syncing time...");
    delay(500);
  }
}
