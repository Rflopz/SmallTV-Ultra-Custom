#include "DisplayManager.h"
#include "Globals.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <TJpg_Decoder.h>
#include <time.h>

// Timezone for Hermosillo
#define UTC_OFFSET -25200
#define DST_OFFSET 0

// Clipping variables for partial updates
int16_t clipY_start = 0;
int16_t clipY_end = 240;

// Callback for TJpg_Decoder
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h,
                uint16_t *bitmap) {
  if (y >= tft.height())
    return 0;

  // Simple vertical clipping optimization
  // If the block is completely outside our vertical range, skip drawing it.
  if (y + h < clipY_start || y > clipY_end)
    return 1;

  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void drawBackground() {
  if (LittleFS.exists("/background.jpg")) {
    File f = LittleFS.open("/background.jpg", "r");
    f.close();

    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);

    // Pass LittleFS as the last argument
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

  // --- SYSTEM INFO ---
  Serial.println("\n\n--- SYSTEM INFO ---");
  Serial.printf("Heap: %.2f KB\n", ESP.getFreeHeap() / (1024.0));
  Serial.printf("Flash: %.2f KB\n", ESP.getFlashChipSize() / (1024.0));
  Serial.printf("Screen: %dx%d\n", tft.width(), tft.height());
  Serial.println("-------------------\n");
}

int lastMinute = -1;

void drawClock() {
  time_t now = time(nullptr);
  struct tm *p_tm = localtime(&now);

  // Only draw if we have a valid time (year > 2000)
  if (p_tm->tm_year + 1900 > 2000) {

    // Only update if the minute has changed
    if (p_tm->tm_min != lastMinute) {
      lastMinute = p_tm->tm_min;
      clipY_start = 70;
      clipY_end = 160;
      drawBackground();
      clipY_start = 0;
      clipY_end = 240;

      tft.setTextColor(timeColor);
      tft.setTextSize(5);
      char timeStr[6];
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", p_tm->tm_hour,
               p_tm->tm_min);
      int timeWidth = tft.textWidth(timeStr);
      tft.setCursor((tft.width() - timeWidth) / 2, 80);
      tft.print(timeStr);

      tft.setTextSize(2);
      tft.setTextColor(TFT_YELLOW);
      char dateStr[16];
      snprintf(dateStr, sizeof(dateStr), "%d/%d/%d", p_tm->tm_mday,
               p_tm->tm_mon + 1, p_tm->tm_year + 1900);
      int dateWidth = tft.textWidth(dateStr);
      tft.setCursor((tft.width() - dateWidth) / 2, 140);
      tft.print(dateStr);

      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(1);
      tft.setCursor(20, 20);
      tft.println(WiFi.localIP());
    }
    delay(200);
  } else {
    Serial.println("Syncing time...");
    delay(500);
  }
}

File animFile;
uint32_t nextFrameTime = 0;
uint8_t *frameBuf = NULL;
size_t frameBufSize = 0;

void loopDisplay() {
  // Check for animation file
  if (LittleFS.exists("/anim.mjpeg")) {
    if (!animFile) {
      animFile = LittleFS.open("/anim.mjpeg", "r");
      if (!animFile) {
        Serial.println("Failed to open anim.mjpeg");
        LittleFS.remove("/anim.mjpeg");
        return;
      }
      tft.fillScreen(TFT_BLACK);
    }

    if (millis() >= nextFrameTime) {
      if (animFile.available() >= 4) {
        // Read Frame Size
        uint32_t size = 0;
        animFile.read((uint8_t *)&size, 4);

        // Sanity check size
        if (size > 30000 || size == 0) {
          Serial.println("Invalid frame size, resetting");
          animFile.seek(0);
          return;
        }

        // Alloc buffer if needed
        if (size > frameBufSize) {
          if (frameBuf)
            free(frameBuf);
          frameBuf = (uint8_t *)malloc(size);
          frameBufSize = size;
        }

        if (frameBuf) {
          if (animFile.read(frameBuf, size) == (int)size) {
            TJpgDec.drawJpg(0, 0, frameBuf, size);
          }
        }

        // Simple delay control - could be embedded in file later
        nextFrameTime = millis() + 50;
      } else {
        // Loop
        animFile.seek(0);
      }
    }
  } else {
    if (animFile) {
      animFile.close();
      if (frameBuf) {
        free(frameBuf);
        frameBuf = NULL;
        frameBufSize = 0;
      }
      drawBackground();
    }
    drawClock();
  }
}
