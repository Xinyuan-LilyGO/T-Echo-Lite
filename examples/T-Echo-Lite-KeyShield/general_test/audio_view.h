/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:38:36
 * @License: GPL 3.0
 */
#pragma once

#include <string>

#include "cpp_bus_driver_library.h"

namespace audio_view {

struct KeyResult {
  bool handled = false;
  bool use_key_vibration = true;
};

/**
 * @brief 初始化音频录制使用的外部 Flash。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool InitFlash();

/**
 * @brief 结束外部 Flash 并进入低功耗状态。
 */
void EndFlash();

/**
 * @brief 停止 ES8311 的 I2S 录音或播放传输。
 * @param es8311 ES8311 驱动对象。
 */
void Stop(cpp_bus_driver::Es8311& es8311);

/**
 * @brief 显示音频测试页面。
 * @param page_selected 页面是否处于选中状态。
 * @param page_name 页面名称。
 * @param busy_enable 是否显示忙碌状态。
 */
void Show(bool page_selected, const char* page_name, bool busy_enable = false);

/**
 * @brief 重置音频页面提示文本。
 */
void ResetPrompt();

/**
 * @brief 处理音频页面按键事件。
 * @param key_text 按键名称。
 * @param es8311 ES8311 驱动对象。
 * @param completion_vibration 完成提示振动回调。
 * @return 返回按键处理结果。
 */
KeyResult HandleKey(const std::string& key_text,
    cpp_bus_driver::Es8311& es8311, void (*completion_vibration)());

}  // namespace audio_view
