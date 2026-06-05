/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:38:40
 * @License: GPL 3.0
 */
#include "battery_view.h"

#include <cstdio>

namespace battery_view {
namespace {

constexpr size_t kBatteryVisibleLineCount = 10;

}  // namespace

std::vector<std::string> CreateLines(const BatteryInfo& info) {
  std::vector<std::string> lines;
  char line[64] = {};

  lines.push_back("[Battery]");
  if (!info.has_data) {
    lines.push_back("press Center to refresh");
    return lines;
  }

  snprintf(line, sizeof(line), "level: %u%%",
      static_cast<unsigned int>(info.percentage));
  lines.push_back(line);
  snprintf(line, sizeof(line), "filtered: %u%%",
      static_cast<unsigned int>(info.filtered_percentage));
  lines.push_back(line);
  snprintf(line, sizeof(line), "voltage: %.3fV", info.battery_voltage);
  lines.push_back(line);
  lines.push_back("");

  lines.push_back("[ADC]");
  snprintf(line, sizeof(line), "raw average: %u",
      static_cast<unsigned int>(info.adc_raw));
  lines.push_back(line);
  snprintf(line, sizeof(line), "adc voltage: %.1fmV", info.adc_voltage_mv);
  lines.push_back(line);
  snprintf(line, sizeof(line), "samples: %u",
      static_cast<unsigned int>(info.sample_count));
  lines.push_back(line);
  snprintf(line, sizeof(line), "reference: %.0fmV", info.reference_mv);
  lines.push_back(line);
  snprintf(line, sizeof(line), "resolution: %.0f", info.adc_resolution);
  lines.push_back(line);
  lines.push_back("");

  lines.push_back("[Calibration]");
  snprintf(line, sizeof(line), "divider: %.2fx", info.divider_ratio);
  lines.push_back(line);
  snprintf(line, sizeof(line), "empty: %.2fV", info.empty_voltage);
  lines.push_back(line);
  snprintf(line, sizeof(line), "full: %.2fV", info.full_voltage);
  lines.push_back(line);

  return lines;
}

size_t GetMaxScrollIndex(const BatteryInfo& info) {
  const size_t line_count = CreateLines(info).size();
  return line_count > kBatteryVisibleLineCount
             ? line_count - kBatteryVisibleLineCount
             : 0;
}

}  // namespace battery_view
