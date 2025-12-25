#include <Arduino.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include <ESP32-OTA.h>

#define WIFI_SSID "Rohan"
#define WIFI_PASS "vikki08494"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);

void show(const char* l1, const char* l2 = "") {
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 14, l1);
  oled.drawStr(0, 30, l2);
  oled.sendBuffer();
}

void onOtaMode() {
  Serial.println("[APP] OTA MODE TRIGGERED");
  show("OTA MODE", "Started");

}

void otaStart() {
  Serial.println("[APP] OTA START");
  show("OTA STARTED", "Uploading...");
}

void otaProgress(uint8_t percent) {
  char buf[20];
  sprintf(buf, "%u%%", percent);
  show("OTA UPDATING", buf);
}

void otaEnd() {
  Serial.println("[APP] OTA END");
  show("OTA DONE", "Rebooting...");
}

void otaError(ota_error_t err) {
  Serial.printf("[APP] OTA ERROR %u\n", err);
  show("OTA ERROR", "Check Serial");
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(500);

  /* OLED init */
  oled.begin();
  show("Booting...", "");

  Serial.println("\n=== ESP32 OTA ANYTIME MODE ===");

  /* OTA CONFIG */
  SmartOtaConfig cfg;
  cfg.hostname = "esp32-test-ota";
  cfg.staSsid  = WIFI_SSID;
  cfg.staPass  = WIFI_PASS;
  // cfg.buttonPin = 0;       // optional (default BOOT)
  // cfg.staTimeoutMs = 15000;

  /* REGISTER OTA CALLBACKS */
  SmartButtonOTA::onOtaMode(onOtaMode);
  SmartButtonOTA::onStart(otaStart);
  SmartButtonOTA::onProgress(otaProgress);
  SmartButtonOTA::onEnd(otaEnd);
  SmartButtonOTA::onError(otaError);

  /* START OTA SYSTEM */
  SmartButtonOTA::begin(cfg);

  show("Normal Mode", "Running");
  Serial.println("[APP] Normal mode running");
}

/* ================= LOOP ================= */
void loop() {
  if(!SmartButtonOTA::isOtaModeActive()){
    static uint32_t last = 0;

  if (millis() - last > 3000) {
    last = millis();
    Serial.println("[APP] App running");
    show("Normal Mode", "App running");
  }

  delay(10);
  }   // system friendly
}