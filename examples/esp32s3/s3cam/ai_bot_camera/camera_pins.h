/*
 * Copyright (c) 2026 Keyes DIY Robot.
 * Camera pin definitions for Keyestudio MB0184 ESP32-S3 CAM.
 *
 * Mapping adapted from Espressif Arduino Core 3.2.0 CameraWebServer,
 * CAMERA_MODEL_ESP32S3_EYE, and checked against the MB0184 pin-out:
 * https://github.com/espressif/arduino-esp32/blob/3.2.0/libraries/ESP32/examples/Camera/CameraWebServer/camera_pins.h
 * The upstream pin-definition file has no separate copyright header.
 * This is a camera wiring mapping, not an ESP-EYE board/audio/AI profile.
 */
#pragma once

#define CAMERA_MODEL_ESP32S3_EYE

// PWDN/RESET are not specified by the MB0184 pin-out. The official Core
// 3.2.0 ESP32S3_EYE mapping uses -1, meaning no dedicated GPIO is driven.
#define PWDN_GPIO_NUM  (-1)
#define RESET_GPIO_NUM (-1)
#define XCLK_GPIO_NUM  15
#define SIOD_GPIO_NUM  4
#define SIOC_GPIO_NUM  5

// Sensor Y2..Y9 correspond to camera_config_t pin_d0..pin_d7.
#define Y2_GPIO_NUM    11
#define Y3_GPIO_NUM    9
#define Y4_GPIO_NUM    8
#define Y5_GPIO_NUM    10
#define Y6_GPIO_NUM    12
#define Y7_GPIO_NUM    18
#define Y8_GPIO_NUM    17
#define Y9_GPIO_NUM    16
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM  7
#define PCLK_GPIO_NUM  13

// No LED, motor, microphone, speaker or SD pins are configured here.
// MB0184 SD CMD=38, CLK=39, D0=40 are deliberately unused by this example.
