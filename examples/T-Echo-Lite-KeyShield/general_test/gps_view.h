/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:38:36
 * @License: GPL 3.0
 */
#pragma once

#include <stdint.h>

// GPS 测试页面显示信息。
struct GpsInfo {
  // GPS 模块是否已检测到并初始化成功。
  bool module_found = false;

  // 当前是否有有效定位。
  bool has_fix = false;

  // 纬度，单位度（正数为北纬）。
  double latitude = 0.0;

  // 经度，单位度（正数为东经）。
  double longitude = 0.0;

  // 海拔高度，单位米。
  float altitude = 0.0f;

  // 当前使用的卫星数量。
  uint8_t satellites_used = 0;

  // 可见卫星总数。
  uint8_t satellites_visible = 0;

  // 水平精度因子。
  float hdop = 0.0f;

  // 垂直精度因子。
  float vdop = 0.0f;

  // 三维位置精度因子。
  float pdop = 0.0f;

  // 可见卫星中最大信号信噪比，单位 dBHz。
  int16_t max_cn0 = 0;

  // 地面速率，单位 km/h。
  float speed_kmh = 0.0f;

  // UTC 时间 — 小时。
  uint8_t utc_hour = 0;

  // UTC 时间 — 分钟。
  uint8_t utc_minute = 0;

  // UTC 时间 — 秒。
  float utc_second = 0.0f;

  // UTC 时间是否有效。
  bool utc_valid = false;

  // UTC 日期（日/月/年）。
  uint8_t utc_day = 0;
  uint8_t utc_month = 0;
  uint16_t utc_year = 0;
  bool date_valid = false;

  // 从进入 GPS 页面到首次获取有效定位的耗时，单位秒。
  // 未获取定位时为已等待秒数，获取后为首次定位耗时。
  uint32_t time_to_first_fix_s = 0;
};

/**
 * @brief 初始化 GPS 模块。
 * @return 初始化成功时返回 true（模块不存在也返回 true，通过 GetGpsInfo 判断）。
 */
bool InitializeGps();

/**
 * @brief 关闭 GPS 模块并释放资源。
 */
void ShutdownGps();

/**
 * @brief 处理 GPS 接收数据。
 * @return 状态有变化需要刷新页面时返回 true。
 */
bool ProcessGps();

/**
 * @brief 获取 GPS 当前状态。
 */
GpsInfo GetGpsInfo();
