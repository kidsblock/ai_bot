/*
 * Copyright (c) 2026 Keyes DIY Robot.
 * Header-only S3cam JPEG camera and local HTTP photo viewer.
 * Camera pin mapping/API references: Espressif Arduino ESP32 Core 3.2.0
 * CameraWebServer, and the user-provided S3cam config.h. Upstream library
 * copyrights remain in their original Core files. No AI/cloud/audio pipeline.
 */
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_camera.h>
#include <errno.h>
#include <lwip/sockets.h>
#include <string>

namespace ai_bot {

// Use one active instance, only from the Arduino setup/loop task. The camera
// driver is global and WebServer is synchronous; these methods are not thread
// safe. This header introduces no global object and no separately compiled cpp.
class S3camCamera final {
 public:
  explicit S3camCamera(ledc_timer_t timer = LEDC_TIMER_3,
                       ledc_channel_t channel = LEDC_CHANNEL_7)
      : timer_(timer), channel_(channel) {}

  ~S3camCamera() { End(); }
  S3camCamera(const S3camCamera&) = delete;
  S3camCamera& operator=(const S3camCamera&) = delete;

  // Safe to call before Wi-Fi connects. Requires real OPI PSRAM; no DRAM or
  // fake initialization fallback. Failure never starts the HTTP service.
  bool Begin() {
    if (ready_) {
      return true;
    }
    if (!psramFound()) {
      Serial.println("S3cam ERROR: PSRAM not detected. Select 8 MB OPI PSRAM.");
      return false;
    }

    camera_config_t config = {};
    config.ledc_timer = timer_;
    config.ledc_channel = channel_;
    config.pin_d0 = 11;
    config.pin_d1 = 9;
    config.pin_d2 = 8;
    config.pin_d3 = 10;
    config.pin_d4 = 12;
    config.pin_d5 = 18;
    config.pin_d6 = 17;
    config.pin_d7 = 16;
    config.pin_xclk = 15;
    config.pin_pclk = 13;
    config.pin_vsync = 6;
    config.pin_href = 7;
    config.pin_sccb_sda = 4;
    config.pin_sccb_scl = 5;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    // esp_camera probes the actual sensor; no hard-coded OV3660 requirement.
    const esp_err_t error = esp_camera_init(&config);
    if (error != ESP_OK) {
      Serial.printf("S3cam ERROR: esp_camera_init failed: 0x%x. Check power/cable.\n",
                    static_cast<unsigned int>(error));
      return false;
    }
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
      Serial.println("S3cam ERROR: Sensor unavailable after camera initialization.");
      esp_camera_deinit();
      return false;
    }
    const char* model = "driver-detected sensor";
    if (sensor->id.PID == OV2640_PID) {
      model = "OV2640";
    } else if (sensor->id.PID == OV3660_PID) {
      model = "OV3660";
    }
    Serial.printf("S3cam camera PID=0x%04x (%s); JPEG QVGA, PSRAM, XCLK=20MHz.\n",
                  static_cast<unsigned int>(sensor->id.PID), model);
    if (sensor->set_vflip(sensor, 1) != 0) {
      Serial.println("S3cam warning: vertical-flip setting failed; orientation may differ.");
    }
    ready_ = true;
    return true;
  }

  // Call after Wi-Fi connects, from the same task that calls Update(). This
  // class never supplies credentials or changes the caller's Wi-Fi settings.
  void StartHttpServer() {
    if (httpStarted_) {
      return;
    }
    if (!ready_ || WiFi.status() != WL_CONNECTED) {
      Serial.println("S3cam HTTP not started: initialize camera and connect Wi-Fi first.");
      return;
    }
    if (!routesRegistered_) {
      server_.on("/", HTTP_GET, [this]() { HandleIndex(); });
      server_.on("/capture", HTTP_GET, [this]() { HandleCapture(); });
      server_.onNotFound([this]() { SendText(404, "Not found. Use / or /capture.\n"); });
      routesRegistered_ = true;
    }
    server_.begin();
    httpStarted_ = true;
    Serial.print("S3cam local photo page: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/ (same trusted Wi-Fi; no cloud upload)");
  }

  void Update() {
    if (!httpStarted_) {
      return;
    }
    if (WiFi.status() != WL_CONNECTED) {
      server_.stop();
      httpStarted_ = false;
      Serial.println("S3cam HTTP stopped: Wi-Fi disconnected. Restart after reconnection.");
      return;
    }
    server_.handleClient();
  }

  // Ready refers to the camera, not to Wi-Fi or the HTTP listener.
  bool Ready() const { return ready_; }

  // This URL is only a LAN JPEG endpoint, NOT a cloud/AI vision URL. Empty
  // means the camera/server/network is not currently available.
  std::string CaptureUrl() const {
    if (!ready_ || !httpStarted_ || WiFi.status() != WL_CONNECTED) {
      return std::string();
    }
    return std::string("http://") + WiFi.localIP().toString().c_str() + "/capture";
  }

  // Optional explicit cleanup, used by the standalone example on failure.
  // Does not disconnect Wi-Fi owned by the caller.
  void End() {
    if (httpStarted_) {
      server_.stop();
      httpStarted_ = false;
    }
    if (ready_) {
      esp_camera_deinit();
      ready_ = false;
    }
  }

 private:
  // WebServer::send() is void. This hook checks both headers and body and
  // handles short writes. Non-blocking send avoids NetworkClient::write()'s
  // own retries extending our shared 5-second response deadline. It does not
  // impose a separate timeout on the driver's esp_camera_fb_get().
  class CheckedWebServer final : public WebServer {
   public:
    CheckedWebServer() : WebServer(80) {}
    void BeginResponse() {
      responseOk_ = true;
      startedAt_ = millis();
    }
    bool ResponseOk() const { return responseOk_; }

   protected:
    size_t _currentClientWrite(const char* data, size_t length) override {
      if (!responseOk_) {
        return 0;
      }
      size_t written = 0;
      while (written < length) {
        if (!client().connected() || static_cast<uint32_t>(millis() - startedAt_) >= 5000) {
          break;
        }
        const size_t remaining = length - written;
        const size_t chunk = remaining < 1460 ? remaining : 1460;
        const int sent = ::send(client().fd(), data + written, chunk, MSG_DONTWAIT);
        if (sent > 0) {
          written += static_cast<size_t>(sent);
        } else if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
          delay(1);
          continue;
        } else {
          break;
        }
        delay(1);
      }
      if (written != length) {
        responseOk_ = false;
        client().stop();
      }
      return written;
    }

   private:
    bool responseOk_ = true;
    uint32_t startedAt_ = 0;
  };

  class CameraFrame final {
   public:
    explicit CameraFrame(camera_fb_t* frame) : frame_(frame) {}
    ~CameraFrame() {
      if (frame_ != nullptr) {
        esp_camera_fb_return(frame_);
      }
    }
    CameraFrame(const CameraFrame&) = delete;
    CameraFrame& operator=(const CameraFrame&) = delete;
    camera_fb_t* Get() const { return frame_; }

   private:
    camera_fb_t* frame_;
  };

  void SendText(int code, const char* text) {
    server_.BeginResponse();
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(code, "text/plain; charset=utf-8", text);
    if (!server_.ResponseOk()) {
      Serial.println("S3cam HTTP text response failed or timed out.");
    }
  }

  void HandleIndex() {
    const char* html = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>S3cam local camera</title></head><body>
<h1>S3cam local camera</h1>
<p>One JPEG photo at 320 x 240. No streaming, SD storage or cloud upload.</p>
<p>This camera page does not perform audio or AI processing.</p>
<p>Use only on your trusted local Wi-Fi network.</p>
<button id="capture" type="button">Capture photo</button>
<p id="status">Press the button to request one photo.</p>
<img id="photo" alt="Captured photo appears here" style="max-width:100%;height:auto" hidden>
<script>
const button = document.getElementById('capture');
const photo = document.getElementById('photo');
const status = document.getElementById('status');
button.onclick = () => {
  button.disabled = true;
  status.textContent = 'Capturing...';
  photo.onload = () => {
    photo.hidden = false;
    status.textContent = 'Photo received. Press again for another photo.';
    button.disabled = false;
  };
  photo.onerror = () => {
    photo.hidden = true;
    status.textContent = 'Capture failed. Check the serial monitor and Wi-Fi connection.';
    button.disabled = false;
  };
  photo.src = '/capture?t=' + Date.now();
};
</script></body></html>)HTML";
    server_.BeginResponse();
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(200, "text/html; charset=utf-8", html);
    if (!server_.ResponseOk()) {
      Serial.println("S3cam HTTP page response failed or timed out.");
    }
  }

  void HandleCapture() {
    if (!ready_) {
      SendText(503, "Camera is not ready. Check the serial monitor.\n");
      return;
    }
    const CameraFrame frame(esp_camera_fb_get());
    camera_fb_t* fb = frame.Get();
    if (fb == nullptr) {
      Serial.println("S3cam capture failed: no frame buffer.");
      SendText(503, "Camera capture failed. Try again or check the camera cable.\n");
      return;
    }
    if (fb->format != PIXFORMAT_JPEG || fb->buf == nullptr || fb->len == 0) {
      Serial.println("S3cam returned an invalid/non-JPEG frame.");
      SendText(500, "Invalid JPEG frame.\n");
      return;
    }
    server_.BeginResponse();
    server_.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
    server_.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
    server_.setContentLength(fb->len);
    server_.send(200, "image/jpeg", "");
    if (server_.ResponseOk()) {
      server_.sendContent(reinterpret_cast<const char*>(fb->buf), fb->len);
    }
    if (!server_.ResponseOk()) {
      Serial.println("S3cam JPEG response incomplete: disconnect or send timeout.");
    } else {
      Serial.printf("S3cam JPEG sent: %u bytes.\n", static_cast<unsigned int>(fb->len));
    }
    // CameraFrame returns fb on every exit, including a failed write.
  }

  ledc_timer_t timer_;
  ledc_channel_t channel_;
  CheckedWebServer server_;
  bool ready_ = false;
  bool httpStarted_ = false;
  bool routesRegistered_ = false;
};

}  // namespace ai_bot
