#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

/* ===== CALLBACK TYPES ===== */
typedef void (*OtaStartCallback)();
typedef void (*OtaProgressCallback)(uint8_t percent);
typedef void (*OtaEndCallback)();
typedef void (*OtaErrorCallback)(ota_error_t error);
typedef void (*OtaModeCallback)();

/* ===== CONFIG STRUCT ===== */
struct SmartOtaConfig {
  const char* hostname;
  const char* staSsid;
  const char* staPass;
  uint8_t     buttonPin = 0;     // BOOT
  uint32_t    buttonWindowMs = 2000;
  uint32_t    staTimeoutMs = 15000;
  const char* apSsid = "ESP32-OTA";
  const char* apPass = "12345678";
};

/* ===== API ===== */
class SmartButtonOTA {
public:
  static void begin(const SmartOtaConfig& cfg);

  static void onStart(OtaStartCallback cb);
  static void onProgress(OtaProgressCallback cb);
  static void onEnd(OtaEndCallback cb);
  static void onError(OtaErrorCallback cb);
  static void onOtaMode(OtaModeCallback cb);
  static bool isOtaModeActive();
  static String getOtaSsid();
};