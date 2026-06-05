/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:39:02
 * @License: GPL 3.0
 */
#pragma once

#include <stddef.h>

#include <string>
#include <vector>

namespace keyboard_view {

/**
 * @brief 获取键盘测试页面当前显示文本。
 * @return 当前文本列表引用。
 */
const std::vector<std::string>& GetTextList();

/**
 * @brief 添加一条键盘测试文本。
 * @param text 按键显示文本。
 */
void AddText(const std::string& text);

/**
 * @brief 清空键盘测试页面文本。
 */
void Clear();

}  // namespace keyboard_view
