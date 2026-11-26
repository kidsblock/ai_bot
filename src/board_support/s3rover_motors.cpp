// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Keyes DIY Robot
#include "s3rover_motors.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Arduino.h>
#include <esp_arduino_version.h>
#endif

// The timer-allocation guard below is deliberately pinned to the verified
// core implementation. Revalidate esp32-hal-ledc.c before widening this.
#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3) && \
    ESP_ARDUINO_VERSION_MAJOR == 3 && ESP_ARDUINO_VERSION_MINOR == 2 && \
    ESP_ARDUINO_VERSION_PATCH == 0
#define AI_BOT_S3ROVER_MOTOR_SUPPORTED 1
#include <esp32-hal-periman.h>
#else
#define AI_BOT_S3ROVER_MOTOR_SUPPORTED 0
#endif

namespace ai_bot {

#if AI_BOT_S3ROVER_MOTOR_SUPPORTED
namespace {
constexpr uint8_t kDirPins[4] = {10, 12, 17, 46};
constexpr uint8_t kEnablePins[4] = {11, 13, 18, 38};
constexpr uint32_t kPwmHz = 20000;
constexpr uint8_t kPwmBits = 8;
constexpr uint32_t kMaxDuty = 255;
// ledcWrite duty updates on a PWM boundary. 100us is two 20kHz cycles,
// not a promise that inductive motor current has completely decayed.
constexpr unsigned int kBrakeBeforeDirectionUs = 100;
constexpr unsigned int kDirectionSettleUs = 10;
S3roverMotors* g_owner = nullptr;

void ForceEnablesLow() {
  for (uint8_t pin : kEnablePins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
}

bool FourFreeChannelsHaveUnsharedTimers() {
  static_assert(SOC_LEDC_CHANNEL_NUM == 8, "Expected ESP32-S3 LEDC layout");
  uint32_t occupied = 0;
  for (uint8_t pin = 0; pin < SOC_GPIO_PIN_COUNT; ++pin) {
    if (!perimanPinIsValid(pin) ||
        perimanGetPinBusType(pin) != ESP32_BUS_TYPE_LEDC) {
      continue;
    }
    const int8_t channel = perimanGetPinBusChannel(pin);
    if (channel < 0 || channel >= 8) {
      return false;
    }
    occupied |= (uint32_t{1} << channel);
  }
  // Core 3.2.0 allocates the lowest free channel and maps each pair of
  // channels to one timer. Do not reconfigure any external user's timer.
  uint8_t selected = 0;
  for (uint8_t channel = 0; channel < 8 && selected < 4; ++channel) {
    const uint32_t bit = uint32_t{1} << channel;
    if ((occupied & bit) != 0) {
      continue;
    }
    const uint32_t timer_pair = uint32_t{3} << (channel & ~uint8_t{1});
    if ((occupied & timer_pair) != 0) {
      return false;
    }
    ++selected;
  }
  return selected == 4;
}

bool DeadlineReached(uint32_t now, uint32_t deadline) {
  // Pure unsigned arithmetic: no implementation-defined signed cast.
  // Valid for intervals < 2^31 ms; our maximum is only 10000 ms.
  return static_cast<uint32_t>(now - deadline) < uint32_t{0x80000000};
}
}  // namespace

S3roverMotors::~S3roverMotors() {
  if (g_owner == this) {
    FailSafe();
    g_owner = nullptr;
  }
}

bool S3roverMotors::Begin() {
  if (faulted_) {
    return false;
  }
  if (g_owner != nullptr && g_owner != this) {
    faulted_ = true;
    return false;  // Do not alter the legitimate owner's hardware.
  }
  if (ready_) {
    StopAll();
    return ready_;
  }
  g_owner = this;
  ForceEnablesLow();  // ALL four ENABLEs low before any PWM is attached.
  for (uint8_t pin : kDirPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  if (!FourFreeChannelsHaveUnsharedTimers()) {
    FailSafe();
    return false;
  }
  for (uint8_t i = 0; i < 4; ++i) {
    if (!ledcAttach(kEnablePins[i], kPwmHz, kPwmBits)) {
      FailSafe();
      return false;
    }
    ports_[i].attached = true;
    if (!ledcWrite(kEnablePins[i], 0)) {
      FailSafe();
      return false;
    }
  }
  ready_ = true;
  return true;
}

bool S3roverMotors::StopPort(uint8_t index) {
  if (!ledcWrite(kEnablePins[index], 0)) {
    return false;
  }
  ports_[index].active = false;
  ports_[index].deadline_ms = 0;
  return true;
}

bool S3roverMotors::SetMotor(uint8_t port, int speed_percent,
                            uint32_t duration_ms) {
  if (!ready_ || faulted_) {
    return false;
  }
  Update();
  if (!ready_ || port < 1 || port > 4 || speed_percent < -100 ||
      speed_percent > 100 || duration_ms < 1 || duration_ms > 10000) {
    return false;
  }
  const uint8_t index = port - 1;
  if (speed_percent == 0) {
    if (!StopPort(index)) {
      FailSafe();
      return false;
    }
    return true;
  }
  const bool reverse = speed_percent < 0;
  if (reverse != ports_[index].reverse) {
    if (!StopPort(index)) {
      FailSafe();
      return false;
    }
    delayMicroseconds(kBrakeBeforeDirectionUs);
    digitalWrite(kDirPins[index], reverse ? HIGH : LOW);
    ports_[index].reverse = reverse;
    delayMicroseconds(kDirectionSettleUs);
  }
  // Range was checked before negation, so even INT_MIN is rejected safely.
  const uint32_t magnitude = static_cast<uint32_t>(
      speed_percent < 0 ? -speed_percent : speed_percent);
  const uint32_t duty = (magnitude * kMaxDuty + 50) / 100;
  ports_[index].deadline_ms = static_cast<uint32_t>(millis()) + duration_ms;
  if (!ledcWrite(kEnablePins[index], duty)) {
    FailSafe();
    return false;
  }
  ports_[index].active = true;
  Update();  // Also service other ports after the brief direction guard.
  return ready_;
}

void S3roverMotors::StopAll() {
  if (!ready_ || g_owner != this) {
    return;
  }
  bool stopped = true;
  for (uint8_t i = 0; i < 4; ++i) {
    // Do not short-circuit: attempt to brake every port even on failure.
    if (!StopPort(i)) {
      stopped = false;
    }
  }
  if (!stopped) {
    FailSafe();
  }
}

void S3roverMotors::Update() {
  if (!ready_ || faulted_) {
    return;
  }
  const uint32_t now = static_cast<uint32_t>(millis());
  for (uint8_t i = 0; i < 4; ++i) {
    if (ports_[i].active && DeadlineReached(now, ports_[i].deadline_ms)) {
      if (!StopPort(i)) {
        FailSafe();
        return;
      }
    }
  }
}

void S3roverMotors::FailSafe() {
  ready_ = false;
  faulted_ = true;
  for (uint8_t i = 0; i < 4; ++i) {
    if (ports_[i].attached) {
      (void)ledcWrite(kEnablePins[i], 0);
    }
  }
  for (uint8_t i = 0; i < 4; ++i) {
    if (ports_[i].attached) {
      (void)ledcDetach(kEnablePins[i]);
    }
    ports_[i] = PortState{};
  }
  // GPIO mode is the final fallback, even if LEDC write/detach failed.
  // Arduino's void GPIO APIs cannot report electrical/hardware failures.
  ForceEnablesLow();
}

#else

S3roverMotors::~S3roverMotors() = default;
bool S3roverMotors::Begin() {
  faulted_ = true;
  return false;
}
bool S3roverMotors::SetMotor(uint8_t, int, uint32_t) {
  return false;
}
void S3roverMotors::StopAll() {}
void S3roverMotors::Update() {}
bool S3roverMotors::StopPort(uint8_t) {
  return false;
}
void S3roverMotors::FailSafe() {
  ready_ = false;
  faulted_ = true;
}

#endif

}  // namespace ai_bot
