#include <ESP8266WiFi.h>
#include "Globals.h"
#include "WifiManager.h"

// --- USER SETTINGS ---
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

void setupWiFi() {
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

  // Success
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(10, 50);
  tft.setTextSize(2);
  tft.println("CONNECTED!");
//   tft.setCursor(10, 80);
//   tft.setTextSize(1);
//   tft.println(WiFi.localIP());

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());
}
