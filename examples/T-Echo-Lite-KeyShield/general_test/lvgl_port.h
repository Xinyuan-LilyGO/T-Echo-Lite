/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-08 00:00:00
 * @License: GPL 3.0
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

namespace lvgl_port {

/**
 * @brief SX1262 LoRa 页面显示状态。
 */
struct Sx1262LoraScreenState {
  const char* frequency_text = nullptr;
  const char* bandwidth_text = nullptr;
  bool auto_send_enabled = false;
  const char* rx_data = nullptr;
  const char* rssi_text = nullptr;
  const char* snr_text = nullptr;
  bool page_selected = false;
  bool frequency_selected = false;
  bool frequency_editing = false;
  bool bandwidth_selected = false;
  bool bandwidth_editing = false;
  bool auto_send_selected = false;
};

/**
 * @brief GPS 测试页面显示状态。
 */
struct GpsScreenState {
  bool module_found = false;
  bool has_fix = false;
  const char* latitude_text = nullptr;
  const char* longitude_text = nullptr;
  const char* satellites_text = nullptr;
  const char* cn0_text = nullptr;
  const char* dop_text = nullptr;
  const char* speed_text = nullptr;
  const char* time_text = nullptr;
  const char* fix_time_text = nullptr;
  bool page_selected = false;
};

/**
 * @brief IMU 测试页面显示状态。
 */
struct ImuScreenState {
  bool module_found = false;
  const char* pitch_text = nullptr;
  const char* roll_text = nullptr;
  const char* yaw_text = nullptr;
  const char* temp_text = nullptr;
  bool page_selected = false;
};

/**
 * @brief 初始化 LVGL 与墨水屏显示端口。
 */
void Init();

/**
 * @brief 启动墨水屏硬件显示。
 */
void BeginDisplay();

/**
 * @brief 结束墨水屏硬件显示。
 */
void EndDisplay();

/**
 * @brief 推进 LVGL 系统计时。
 * @param elapsed_ms 已经过的毫秒数。
 */
void Tick(uint32_t elapsed_ms);

/**
 * @brief 重置局部刷新状态。
 */
void ResetPartialRefresh();

/**
 * @brief 设置状态栏显示的电池百分比。
 * @param percentage 电池百分比，范围 0~100。
 */
void SetBatteryPercentage(uint8_t percentage);

/**
 * @brief 更新状态栏的 BLE 连接图标显示状态。
 * @param connected 为 true 时显示已连接图标，否则显示普通蓝牙图标。
 */
void SetBleConnected(bool connected);

/**
 * @brief 设置状态栏休眠图标显示状态。
 * @param enable true 显示休眠图标，false 隐藏休眠图标。
 */
void SetSleepMode(bool enable);

/**
 * @brief 显示开机启动页面。
 */
void ShowBootScreen();

/**
 * @brief 显示主页样式的信息列表页面。
 * @param lines 显示文本行列表。
 * @param scroll_index 起始滚动行索引。
 * @param page_name 页面名称。
 * @param page_selected 页面是否处于选中状态。
 * @param partial_refresh 是否使用局部刷新。
 * @param busy_enable 是否显示忙碌状态。
 */
void ShowHomeScreen(const std::vector<std::string>& lines, size_t scroll_index,
    const char* page_name, bool page_selected, bool partial_refresh,
    bool busy_enable = false);

/**
 * @brief 显示居中文本页面。
 * @param text 显示文本。
 */
void ShowCenteredText(const char* text);

/**
 * @brief 显示键盘测试文本列表页面。
 * @param text_list 显示文本列表。
 * @param page_name 页面名称。
 * @param page_selected 页面是否处于选中状态。
 * @param partial_refresh 是否使用局部刷新。
 * @param busy_enable 是否显示忙碌状态。
 */
void ShowTextList(const std::vector<std::string>& text_list,
    const char* page_name, bool page_selected, bool partial_refresh,
    bool busy_enable = false);

/**
 * @brief 显示音频测试页面。
 * @param page_selected 页面是否处于选中状态。
 * @param mic_selected true 选中麦克风，false 选中扬声器。
 * @param message 页面提示文本。
 * @param page_name 页面名称。
 * @param partial_refresh 是否使用局部刷新。
 * @param busy_enable 是否显示忙碌状态。
 * @param action_running 是否正在执行录音/播放操作。
 */
void ShowAudioScreen(bool page_selected, bool mic_selected, const char* message,
    const char* page_name, bool partial_refresh, bool busy_enable = false,
    bool action_running = false);

/**
 * @brief 渲染 SX1262 LoRa 页面并刷新屏幕。
 * @param state SX1262 LoRa 页面显示状态。
 * @param page_name 底部页签名称。
 * @param partial_refresh 为 true 时使用电子纸局部刷新。
 * @param busy_enable 为 true 时等待屏幕 busy 引脚。
 */
void ShowSx1262LoraScreen(const Sx1262LoraScreenState& state,
    const char* page_name, bool partial_refresh, bool busy_enable = false);

/**
 * @brief 渲染 GPS 测试页面并刷新屏幕。
 * @param state GPS 页面显示状态。
 * @param page_name 底部页签名称。
 * @param partial_refresh 为 true 时使用电子纸局部刷新。
 * @param busy_enable 为 true 时等待屏幕 busy 引脚。
 */
void ShowGpsScreen(const GpsScreenState& state,
    const char* page_name, bool partial_refresh, bool busy_enable = false);

/**
 * @brief 渲染 IMU 测试页面并刷新屏幕。
 * @param state IMU 页面显示状态。
 * @param page_name 底部页签名称。
 * @param partial_refresh 为 true 时使用电子纸局部刷新。
 * @param busy_enable 为 true 时等待屏幕 busy 引脚。
 */
void ShowImuScreen(const ImuScreenState& state,
    const char* page_name, bool partial_refresh, bool busy_enable = false);

}  // namespace lvgl_port
