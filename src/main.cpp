#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <TFT_eSPI.h>
#include <time.h>

// --- USER SETTINGS ---
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

// Timezone for Hermosillo (Mountain Standard Time, no DST usually)
// UTC offset: -7 hours * 3600 seconds = -25200
#define UTC_OFFSET -25200
#define DST_OFFSET 0

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);

  // 1. Setup Screen
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Backlight ON
  tft.init();
  tft.setRotation(0); // Or 1, 2, 3 depending on how it sits on your desk
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // 2. Connect to WiFi
  tft.println("Connecting to WiFi...");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tft.print(".");
    dots++;
    if (dots > 20) {
      dots = 0;
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.println("Connecting...");
    }
  }

  // 3. WiFi Success!
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(10, 50);
  tft.setTextSize(2);
  tft.println("CONNECTED!");
  tft.setCursor(10, 80);
  tft.setTextSize(1);
  tft.println(WiFi.localIP());

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

  // 4. Init Time (NTP)
  configTime(UTC_OFFSET, DST_OFFSET, "pool.ntp.org", "time.nist.gov");
  tft.println("\nWaiting for time...");
}

void loop() {
  time_t now = time(nullptr);
  struct tm *p_tm = localtime(&now);

  // Only draw if we have a valid time (year > 2000)
  if (p_tm->tm_year + 1900 > 2000) {

    // Simple Digital Clock UI
    // tft.fillScreen(TFT_BLACK); // Removed to prevent blinking

    // Draw Time
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(5);
    tft.setCursor(20, 80);

    // Format: HH:MM:SS
    if (p_tm->tm_hour < 10)
      tft.print("0");
    tft.print(p_tm->tm_hour);
    tft.print(":");
    if (p_tm->tm_min < 10)
      tft.print("0");
    tft.print(p_tm->tm_min);

    // Draw Date below
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(40, 140);
    tft.print(p_tm->tm_mday);
    tft.print("/");
    tft.print(p_tm->tm_mon + 1); // Month is 0-11
    tft.print("/");
    tft.print(p_tm->tm_year + 1900);

    delay(1000); // Update every second
  } else {
    // Still waiting for NTP sync
    Serial.println("Syncing time...");
    delay(500);
  }
}