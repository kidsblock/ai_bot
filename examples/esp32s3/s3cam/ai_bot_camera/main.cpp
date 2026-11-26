/*
 * Copyright (c) 2026 Keyes DIY Robot.
 * S3cam camera-only local photo test; no audio/LCD/AI initialization.
 * The camera driver and HTTP viewer are shared in board_support/s3cam_camera.h.
 */

#include <Arduino.h>
#include <WiFi.h>

#include "ai_bot_camera.h"
#include "camera_pins.h"  // Local pin-map reference; shared class uses the same mapping.

#if !defined(CONFIG_IDF_TARGET_ESP32S3) || !CONFIG_IDF_TARGET_ESP32S3
#error "Select ESP32S3 Dev Module (16 MB Flash, 8 MB OPI PSRAM)."
#endif

// Edit the defaults OR supply quoted string macros in build flags.
// Example: #define WIFI_SSID "your_wifi_ssid"
// Example: #define WIFI_PASSWORD "your_wifi_password"
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

namespace {
constexpr uint32_t kWiFiTimeoutMs = 20000;
ai_bot::S3camCamera g_camera;  // Default XCLK uses LEDC timer 3, channel 7.
bool g_running = false;

bool setupWiFi() {
  if (WIFI_SSID[0] == '\0') {
    Serial.println("Wi-Fi SSID is empty. No camera/network/server will be started.");
    Serial.println("Set WIFI_SSID and WIFI_PASSWORD in main.cpp or quoted build macros.");
    Serial.println("Upload again and reset. Empty password is only for an open network.");
    return false;
  }
  WiFi.persistent(false);  // Do not save credentials to flash.
  WiFi.setAutoReconnect(false);
  WiFi.mode(WIFI_STA);    // No AP, SmartConfig or provisioning server.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to configured Wi-Fi (20 second timeout)");
  const uint32_t startedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         static_cast<uint32_t>(millis() - startedAt) < kWiFiTimeoutMs) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ERROR: Wi-Fi timed out. Check credentials and 2.4 GHz Wi-Fi.");
    return false;
  }
  Serial.println("Wi-Fi connected. Password is never printed.");
  return true;
}

void enterIdle() {
  g_running = false;
  g_camera.End();
  WiFi.disconnect(false, false);
  WiFi.mode(WIFI_OFF);
  Serial.println("Safe idle. Correct the configuration/problem and reset to retry.");
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);  // Never wait indefinitely for a serial monitor.
  Serial.println("\nKeyes DIY Robot - S3cam camera-only local photo example");
  if (WIFI_SSID[0] == '\0') {
    setupWiFi();  // Print the configuration instructions, without starting Wi-Fi.
    enterIdle();
    return;
  }
  // The shared camera may initialize early, before Wi-Fi or audio/LCD setup.
  if (!g_camera.Begin() || !setupWiFi()) {
    enterIdle();
    return;
  }
  g_camera.StartHttpServer();
  const std::string captureUrl = g_camera.CaptureUrl();
  if (captureUrl.empty()) {
    enterIdle();
    return;
  }
  g_running = true;
  Serial.print("JPEG endpoint (local network only): ");
  Serial.println(captureUrl.c_str());
  Serial.println("Open the printed photo page on the same trusted Wi-Fi and press Capture photo.");
}

void loop() {
  if (!g_running) {
    delay(100);
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost; stopping the standalone camera example.");
    enterIdle();
    return;
  }
  g_camera.Update();
  delay(2);
}
