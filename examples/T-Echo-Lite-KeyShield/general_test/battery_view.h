/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:38:44
 * @License: GPL 3.0
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

namespace battery_view {

struct BatteryInfo {
  bool has_data = false;
  uint16_t adc_raw = 0;
  float adc_voltage_mv = 0.0f;
  float battery_voltage = 0.0f;
  uint8_t percentage = 0;
  uint8_t filtered_percentage = 0;
  uint8_t sample_count = 0;
  float reference_mv = 0.0f;
  float adc_resolution = 0.0f;
  float divider_ratio = 0.0f;
  float empty_voltage = 0.0f;
  float full_voltage = 0.0f;
};

/**
 * @brief 创建电池信息页面显示文本。
 * @param info 电池信息快照。
 * @return 格式化后的文本行列表。
 */
std::vector<std::string> CreateLines(const BatteryInfo& info);

/**
 * @brief 获取电池信息页面最大滚动索引。
 * @param info 电池信息快照。
 * @return 最大滚动索引。
 */
size_t GetMaxScrollIndex(const BatteryInfo& info);

}  // namespace battery_view
