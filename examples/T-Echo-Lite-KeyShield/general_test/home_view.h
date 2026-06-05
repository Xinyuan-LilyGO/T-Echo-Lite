/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-04 17:25:31
 * @License: GPL 3.0
 */
#pragma once

#include <stddef.h>

#include <Arduino.h>

#include <string>
#include <vector>

namespace home_view {

/**
 * @brief 获取当前固件显示名称。
 * @return 固件名称字符串。
 */
const char* GetSoftwareName();

/**
 * @brief 获取当前板卡版本。
 * @return 板卡版本字符串。
 */
const char* GetBoardVersion();

/**
 * @brief 获取当前固件编译时间。
 * @return 形如 yyyyMMddHHmm 的编译时间。
 */
String GetBuildTime();

/**
 * @brief 创建主页硬件与软件信息文本。
 * @return 格式化后的文本行列表。
 */
std::vector<std::string> CreateLines();

/**
 * @brief 获取主页最大滚动索引。
 * @return 最大滚动索引。
 */
size_t GetMaxScrollIndex();

}  // namespace home_view
