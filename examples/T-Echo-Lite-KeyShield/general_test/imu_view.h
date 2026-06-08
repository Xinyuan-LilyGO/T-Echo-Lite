/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-08 15:15:57
 * @License: GPL 3.0
 */
#pragma once

#include <stdint.h>

/**
 * @brief IMU 测试页面显示信息。
 */
struct ImuInfo {
  // IMU 模块是否已检测到。
  bool module_found = false;

  // 俯仰角，单位度。
  float pitch = 0.0f;

  // 横滚角，单位度。
  float roll = 0.0f;

  // 偏航角（地磁航向），单位度。
  float yaw = 0.0f;

  // 芯片温度，单位 °C。
  float temperature = 0.0f;

  // 传感器读数是否有效。
  bool data_valid = false;
};

/**
 * @brief 初始化 IMU 模块。
 */
bool InitializeImu();

/**
 * @brief 关闭 IMU 模块。
 */
void ShutdownImu();

/**
 * @brief 读取一次 IMU 传感器数据。
 * @return 数据有变化时返回 true。
 */
bool ReadImuSensor();

/**
 * @brief 获取 IMU 当前状态。
 */
ImuInfo GetImuInfo();
