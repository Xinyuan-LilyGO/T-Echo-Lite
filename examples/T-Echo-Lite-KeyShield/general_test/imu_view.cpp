/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-08 18:02:59
 * @License: GPL 3.0
 */
#include "imu_view.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "ICM20948_WE.h"
#include "ble_uart_log.h"
#include "t_echo_lite_keyshield_config.h"

// IMU demo 模式：无硬件时生成模拟传感器数据。
// #define IMU_DEMO_MODE

namespace {

#ifdef IMU_DEMO_MODE
uint32_t demo_start_ms = 0;
#endif

ICM20948_WE& GetImuSensor() {
  static ICM20948_WE sensor(ICM20948_ADDRESS);
  return sensor;
}

ImuInfo imu_info;
bool imu_initialized = false;

}  // namespace

bool InitializeImu() {
  if (imu_initialized) return true;

#ifdef IMU_DEMO_MODE
  demo_start_ms = millis();
  imu_info.module_found = true;
  imu_initialized = true;
  return true;
#else
  // 先快速扫描 I2C 地址，设备不存在则直接跳过。
  Wire.setTimeout(10);
  Wire.beginTransmission(ICM20948_ADDRESS);
  const bool device_found = (Wire.endTransmission() == 0);
  Wire.setTimeout(1000);

  if (!device_found) {
    LogPrintln("IMU not found (I2C address NACK)");
    imu_info.module_found = false;
    imu_initialized = true;
    return true;
  }

  auto& sensor = GetImuSensor();
  if (!sensor.init()) {
    LogPrintln("IMU ICM20948 init failed");
    imu_info.module_found = false;
    imu_initialized = true;
    return true;
  }

  sensor.initMagnetometer();
  LogPrintln("IMU ICM20948 initialized");
  imu_info.module_found = true;
  imu_initialized = true;
  return true;
#endif
}

void ShutdownImu() {
  if (!imu_initialized) return;

#ifndef IMU_DEMO_MODE
  GetImuSensor().sleep(true);
#endif

  imu_initialized = false;
  imu_info = ImuInfo{};
}

bool ReadImuSensor() {
  if (!imu_info.module_found) return false;

#ifdef IMU_DEMO_MODE
  const float t = static_cast<float>(millis() - demo_start_ms) * 0.001f;

  imu_info.pitch = sinf(t * 0.5f) * 5.0f;
  imu_info.roll = cosf(t * 0.7f) * 3.0f;
  imu_info.yaw = fmodf(45.0f + t * 2.0f, 360.0f);
  imu_info.temperature = 31.0f + sinf(t * 0.1f) * 2.0f;
  imu_info.data_valid = true;
  return true;
#else
  auto& sensor = GetImuSensor();
  sensor.readSensor();

  const xyzFloat acc = sensor.getGValues();
  const xyzFloat mag = sensor.getMagValues();

  // 俯仰角（pitch）：绕 Y 轴旋转。
  imu_info.pitch = atan2f(-acc.x, sqrtf(acc.y * acc.y + acc.z * acc.z)) *
                   180.0f / static_cast<float>(M_PI);

  // 横滚角（roll）：绕 X 轴旋转。
  imu_info.roll = atan2f(acc.y, acc.z) * 180.0f / static_cast<float>(M_PI);

  // 偏航角（yaw）：地磁航向，倾斜补偿后。
  const float cos_pitch = cosf(-imu_info.pitch * static_cast<float>(M_PI) / 180.0f);
  const float sin_pitch = sinf(-imu_info.pitch * static_cast<float>(M_PI) / 180.0f);
  const float cos_roll = cosf(imu_info.roll * static_cast<float>(M_PI) / 180.0f);
  const float sin_roll = sinf(imu_info.roll * static_cast<float>(M_PI) / 180.0f);

  const float mag_x = mag.x * cos_pitch + mag.z * sin_pitch;
  const float mag_y = mag.x * sin_roll * sin_pitch +
                      mag.y * cos_roll -
                      mag.z * sin_roll * cos_pitch;

  imu_info.yaw = atan2f(-mag_y, mag_x) * 180.0f / static_cast<float>(M_PI);
  if (imu_info.yaw < 0) imu_info.yaw += 360.0f;

  imu_info.temperature = sensor.getTemperature();
  imu_info.data_valid = true;
  return true;
#endif
}

ImuInfo GetImuInfo() { return imu_info; }
