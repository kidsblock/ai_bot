// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Keyes DIY Robot
#pragma once

#include <stdint.h>

namespace ai_bot {

// DRV8835, MODE held HIGH externally: DIR is PHASE; PWM is ENABLE.
// Ports: 1=(10,11), 2=(12,13), 3=(17,18), 4=(46,38), (DIR,PWM).
// Positive speed: PHASE LOW (OUT1 HIGH / OUT2 LOW); negative: PHASE HIGH.
// This is a port electrical direction, NOT a vehicle-forward convention.
// ENABLE LOW is brake, not coast. MODE=5V must not exceed driver VCC.
//
// Requires Arduino-ESP32 3.2.0 on ESP32-S3. Other builds fail closed.
// One hardware owner at a time. Call every API, including Update(), from
// one serialized loop/task; this class is not ISR-safe or thread-safe.
// Serialize LEDC initialization too. Never reuse these GPIOs or their LEDC
// timers in other code. Begin() checks existing paired-timer conflicts.
// Raw ESP-IDF LEDC users bypassing Arduino Peripheral Manager are not visible
// to that check and must not share this peripheral without coordination.
//
// SAFETY: construction and Begin() never request motion. External hardware
// must hold all ENABLE inputs LOW during reset/high-impedance/boot and any
// attachment glitches. Software alone cannot guarantee a reset-time stop.
// The time limit is cooperative: call Update() frequently without blocking.
// A stalled loop/CPU needs an independent hardware cutoff/watchdog.
class S3roverMotors {
 public:
  S3roverMotors() = default;
  ~S3roverMotors();
  S3roverMotors(const S3roverMotors&) = delete;
  S3roverMotors& operator=(const S3roverMotors&) = delete;
  S3roverMotors(S3roverMotors&&) = delete;
  S3roverMotors& operator=(S3roverMotors&&) = delete;

  // Drives all ENABLE pins LOW before attaching four 20kHz/8-bit PWMs.
  // Repeated successful Begin() stops all without reallocating channels.
  // Any attach/write/resource failure stops all and latches this instance
  // disabled. Begin() cannot clear that latch; investigate before restart.
  bool Begin();

  // port: 1..4; speed_percent: -100..100; duration_ms: 1..10000.
  // All arguments are validated, including duration for speed zero.
  // Zero immediately brakes this port; nonzero replaces its deadline.
  // Invalid arguments return false without changing a running command
  // (except that Update() still services already-expired deadlines).
  bool SetMotor(uint8_t port, int speed_percent, uint32_t duration_ms);

  // Brake all ports and cancel deadlines. Safe before Begin() (no-op).
  // A write failure additionally latches the instance disabled.
  void StopAll();

  // Brake expired ports using wrap-safe uint32_t millis deadlines.
  void Update();

 private:
  struct PortState {
    uint32_t deadline_ms = 0;
    bool attached = false;
    bool active = false;
    bool reverse = false;
  };

  bool StopPort(uint8_t index);
  void FailSafe();

  PortState ports_[4]{};
  bool ready_ = false;
  bool faulted_ = false;
};

}  // namespace ai_bot
