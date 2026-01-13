#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  // 1. FORCE SERIAL LOUD AND CLEAR
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n\n!!! SERIAL CONNECTION ESTABLISHED !!!");
  Serial.println("If you can read this, the logs are fixed.");

  // 2. Setup Backlight
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // ON

  // 3. Init Screen
  Serial.println("Initializing Display...");
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  // 4. Draw Initial Text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("BOOTING...");
  Serial.println("Display Init Command Sent.");
}

void loop() {
  // Print heartbeat to logs
  Serial.println("Looping: Attempting to fill screen RED");
  
  // Fill Screen RED
  tft.fillScreen(TFT_RED);
  tft.setCursor(10, 50);
  tft.setTextColor(TFT_WHITE);
  tft.println("RED");
  delay(1000);

  Serial.println("Looping: Attempting to fill screen GREEN");
  // Fill Screen GREEN
  tft.fillScreen(TFT_GREEN);
  tft.setCursor(10, 50);
  tft.setTextColor(TFT_BLACK);
  tft.println("GREEN");
  delay(1000);
}