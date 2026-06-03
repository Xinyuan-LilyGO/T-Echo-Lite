/*
 * @Description: aw86224
 * @Author: LILYGO_L
 * @Date: 2024-12-25 10:33:25
 * @LastEditTime: 2026-06-03 13:36:56
 * @License: GPL 3.0
 */
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <Wire.h>

#include "cpp_bus_driver_library.h"
#include "t_echo_lite_keyshield_config.h"

using Aw862xx = cpp_bus_driver::Aw862xx;

static constexpr uint8_t kGainLevels[] = {
    16, 32, 48, 64, 80, 96, 112, 128,
    144, 160, 176, 192, 208, 224, 240, 255};
static constexpr uint8_t kLoopCount = 15;
static constexpr uint32_t kPlayMs = 220;
static constexpr uint32_t kStopMs = 180;
static constexpr uint32_t kRetryDelayMs = 1000;
static constexpr uint32_t kCycleDelayMs = 1500;
static constexpr Aw862xx::RamWaveformLibrary kRamWaveformLibrary =
    Aw862xx::RamWaveformLibrary::kRam12k041230_235;

std::shared_ptr<cpp_bus_driver::HardwareI2c2>& Aw86224I2cBus() {
  static auto aw86224_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c2>(
          AW86224_SDA, AW86224_SCL, &Wire);
  return aw86224_i2c_bus;
}

Aw862xx& Aw86224() {
  static auto aw86224 = std::make_unique<Aw862xx>(
      Aw86224I2cBus(), AW86224_IIC_ADDRESS);
  return *aw86224;
}

void setup() {
  Serial.begin(115200);
  uint8_t serial_init_count = 0;
  while (!Serial) {
    delay(100);  // wait for native usb
    serial_init_count++;
    if (serial_init_count > 30) {
      break;
    }
  }
  printf("Ciallo\n");

  // 3.3V Power ON
  pinMode(RT9080_EN, OUTPUT);
  digitalWrite(RT9080_EN, HIGH);
  delay(100);
  digitalWrite(RT9080_EN, LOW);
  delay(100);
  digitalWrite(RT9080_EN, HIGH);
  delay(1000);

  auto& aw86224 = Aw86224();
  if (!aw86224.Init(500000)) {
    printf("Aw86224 init failed\n");
    return;
  }

  const uint32_t detected_f0 = aw86224.GetF0Detection();
  if (detected_f0 == 0 || detected_f0 == static_cast<uint32_t>(-1)) {
    printf("Aw86224 F0 reference read failed\n");
  } else {
    printf("Aw86224 F0 reference: %u.%uHz\n",
        static_cast<unsigned int>(detected_f0 / 10),
        static_cast<unsigned int>(detected_f0 % 10));
  }
}

void loop() {
  auto& aw86224 = Aw86224();

  const auto info = Aw862xx::GetRamWaveformInfo(kRamWaveformLibrary);
  if (!aw86224.InitRamMode(kRamWaveformLibrary)) {
    printf("Aw86224 RAM waveform init failed\n");
    delay(kRetryDelayMs);
    return;
  }

  printf(
      "Aw86224 selected library: %s, sequences: %u, rated f0: %uHz\n",
      info.name, static_cast<unsigned int>(info.waveform_count),
      static_cast<unsigned int>(info.rated_f0_hz));

  printf("Aw86224 gain test levels:");
  for (uint8_t gain : kGainLevels) {
    printf(" %u", static_cast<unsigned int>(gain));
  }
  printf("\n");

  printf("Aw86224 GetInputVoltage: %.06f v\n", aw86224.GetInputVoltage());

  for (uint8_t gain : kGainLevels) {
    printf("Aw86224 gain level: %u\n", static_cast<unsigned int>(gain));

    for (uint8_t sequence = 1; sequence <= info.waveform_count; sequence++) {
      printf("Play %s sequence %u gain %u\n", info.name,
          static_cast<unsigned int>(sequence),
          static_cast<unsigned int>(gain));
      aw86224.PlayRamWaveform(sequence, kLoopCount, gain);
      delay(kPlayMs);
      aw86224.StopRamPlaybackWaveform();
      delay(kStopMs);
    }
  }

  delay(kCycleDelayMs);
}
