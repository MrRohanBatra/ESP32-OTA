#include "ESP32-OTA.h"

/* ================= INTERNAL STATE ================= */
static SmartOtaConfig config;

static TaskHandle_t otaTaskHandle  = nullptr;
static TaskHandle_t bootTaskHandle = nullptr;

static bool otaActive  = false;
static bool otaStarted = false;
static String ssid = "";

/* ================= CALLBACKS ================= */
static OtaStartCallback     cbStart    = nullptr;
static OtaProgressCallback  cbProgress = nullptr;
static OtaEndCallback       cbEnd      = nullptr;
static OtaErrorCallback     cbError    = nullptr;
static OtaModeCallback      cbOtaMode  = nullptr;

/* ================= OTA TASK ================= */
static void otaTask(void*) {
  while (true) {
    if (otaActive) {
      ArduinoOTA.handle();
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

/* ================= START OTA ================= */
static void startOta() {
  if (otaActive) return;

  ArduinoOTA.begin();
  otaActive = true;

  xTaskCreatePinnedToCore(
    otaTask,
    "OTA_TASK",
    4096,
    nullptr,
    1,
    &otaTaskHandle,
    0   // WiFi core
  );
}

/* ================= WIFI STA → AP ================= */
static void connectStaOrAp() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.staSsid, config.staPass);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < config.staTimeoutMs) {
    delay(200);
  }

  if (WiFi.status() == WL_CONNECTED) {
    startOta();
    ssid = config.staSsid;
    return;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(config.apSsid, config.apPass);
  ssid = config.apSsid;
  startOta();
}

/* ================= BOOT BUTTON WATCHER ================= */
static void bootButtonTask(void*) {
  pinMode(config.buttonPin, INPUT_PULLUP);

  while (true) {
    if (!otaStarted && digitalRead(config.buttonPin) == LOW) {

      uint32_t t0 = millis();

      while (digitalRead(config.buttonPin) == LOW) {
        if (millis() - t0 >= 2000) {   // 2s long press
          otaStarted = true;

          // 🔔 OTA MODE TRIGGERED
          if (cbOtaMode) cbOtaMode();

          connectStaOrAp();

          vTaskDelete(nullptr);  // stop button task
        }
        vTaskDelay(pdMS_TO_TICKS(20));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

/* ================= PUBLIC API ================= */
void SmartButtonOTA::begin(const SmartOtaConfig& cfg) {
  config = cfg;

  /* ===== ArduinoOTA CONFIG ===== */
  ArduinoOTA.setHostname(config.hostname);

  ArduinoOTA.onStart([]() {
    if (cbStart) cbStart();
  });

  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) {
    if (cbProgress) cbProgress((p * 100) / t);
  });

  ArduinoOTA.onEnd([]() {
    if (cbEnd) cbEnd();
  });

  ArduinoOTA.onError([](ota_error_t e) {
    if (cbError) cbError(e);
  });

  /* ===== START BOOT BUTTON MONITOR ===== */
  xTaskCreatePinnedToCore(
    bootButtonTask,
    "BOOT_BTN_TASK",
    8192,
    nullptr,
    1,
    &bootTaskHandle,
    1   
  );
}

/* ================= CALLBACK SETTERS ================= */
void SmartButtonOTA::onStart(OtaStartCallback cb) {
  cbStart = cb;
}

void SmartButtonOTA::onProgress(OtaProgressCallback cb) {
  cbProgress = cb;
}

void SmartButtonOTA::onEnd(OtaEndCallback cb) {
  cbEnd = cb;
}

void SmartButtonOTA::onError(OtaErrorCallback cb) {
  cbError = cb;
}

void SmartButtonOTA::onOtaMode(OtaModeCallback cb) {
  cbOtaMode = cb;
}
bool SmartButtonOTA::isOtaModeActive() {
  return otaActive;
}
String SmartButtonOTA::getOtaSsid() {
  return ssid;
}