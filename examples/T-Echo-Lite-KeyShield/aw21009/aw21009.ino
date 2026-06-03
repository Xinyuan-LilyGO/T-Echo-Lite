/*
 * @Description: aw21009
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-01 15:48:05
 * @License: GPL 3.0
 */
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <Wire.h>

#include "cpp_bus_driver_library.h"
#include "t_echo_lite_keyshield_config.h"

namespace {
constexpr int16_t kBrightnessMin = 0;
constexpr int16_t kBrightnessMax = 4095;
constexpr int16_t kBrightnessStep = 20;
constexpr uint32_t kBrightnessDelayMs = 10;

cpp_bus_driver::Aw21009& GetAw21009() {
  static auto aw21009_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c2>(
          AW21009_SDA, AW21009_SCL, &Wire);
  static cpp_bus_driver::Aw21009 aw21009(
      aw21009_i2c_bus, AW21009_IIC_ADDRESS);

  return aw21009;
}
}  // namespace

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

  cpp_bus_driver::Aw21009& aw21009 = GetAw21009();
  aw21009.Init();

  // aw21009.SetAutoPowerSave(true);
  // aw21009.SetChipEnable(true);
  // aw21009.SetGlobalCurrentLimit(255);
  // aw21009.SetCurrentLimit(
  //     cpp_bus_driver::Aw21009::LedChannel::kAll, 255);

  aw21009.SetBrightness(
      cpp_bus_driver::Aw21009::LedChannel::kAll, kBrightnessMin);
}

void loop() {
  static int16_t brightness = kBrightnessMin;
  static int16_t brightness_step = kBrightnessStep;

  GetAw21009().SetBrightness(cpp_bus_driver::Aw21009::LedChannel::kAll,
      static_cast<uint16_t>(brightness));

  brightness += brightness_step;
  if (brightness >= kBrightnessMax) {
    brightness = kBrightnessMax;
    brightness_step = -kBrightnessStep;
  } else if (brightness <= kBrightnessMin) {
    brightness = kBrightnessMin;
    brightness_step = kBrightnessStep;
  }

  delay(kBrightnessDelayMs);
}
