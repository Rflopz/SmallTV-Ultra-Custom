#include <Arduino.h>
#include <time.h>
#include <ESP8266WiFi.h> // For IP display
#include "Globals.h"
#include "DisplayManager.h"

// Timezone for Hermosillo
#define UTC_OFFSET -25200
#define DST_OFFSET 0

void setupDisplay() {
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Backlight ON
  tft.init();
  tft.setRotation(0); 
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Init Time (NTP)
  configTime(UTC_OFFSET, DST_OFFSET, "pool.ntp.org", "time.nist.gov");
  
  // --- SYSTEM INFO ---
  Serial.println("\n\n--- SYSTEM INFO ---");
  Serial.printf("Heap: %.2f KB\n", ESP.getFreeHeap() / (1024.0));
  Serial.printf("Flash: %.2f KB\n", ESP.getFlashChipSize() / (1024.0));
  Serial.printf("Screen: %dx%d\n", tft.width(), tft.height());
  Serial.println("-------------------\n");
}

void drawClock() {
  time_t now = time(nullptr);
  struct tm *p_tm = localtime(&now);

  // Only draw if we have a valid time (year > 2000)
  if (p_tm->tm_year + 1900 > 2000) {

    // Draw Time
    tft.setTextColor(timeColor, TFT_BLACK);
    tft.setTextSize(5);
    
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", p_tm->tm_hour, p_tm->tm_min);
    int timeWidth = tft.textWidth(timeStr);
    tft.setCursor((tft.width() - timeWidth) / 2, 80);
    tft.print(timeStr);

    // Draw Date below
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    
    char dateStr[16];
    snprintf(dateStr, sizeof(dateStr), "%d/%d/%d", p_tm->tm_mday, p_tm->tm_mon + 1, p_tm->tm_year + 1900);
    int dateWidth = tft.textWidth(dateStr);
    tft.setCursor((tft.width() - dateWidth) / 2, 140);
    tft.print(dateStr);

    // Draw IP
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(20, 20);
    tft.println(WiFi.localIP());

    delay(1000); // Update every second
  } else {
    // Still waiting for NTP sync
    Serial.println("Syncing time...");
    delay(500);
  }
}
