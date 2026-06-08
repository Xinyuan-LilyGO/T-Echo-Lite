/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-08 15:16:14
 * @License: GPL 3.0
 */
#pragma once

#include <Arduino.h>

// SX1262 LoRa 测试状态快照。
struct Sx1262LoraInfo {
  // SX1262 是否已经初始化完成。
  bool initialized = false;

  // 自动发送开关是否打开。
  bool auto_send_enabled = false;

  // 当前 LoRa 工作频率，单位 MHz。
  float frequency_mhz = 868.0f;

  // 当前 LoRa 带宽，单位 kHz。
  float bandwidth_khz = 125.0f;

  // 自动发送间隔，单位 ms。
  uint32_t auto_send_interval_ms = 5000;

  // 当前自动发送计数。
  uint32_t tx_counter = 0;

  // 成功发送的数据包数量。
  uint32_t tx_count = 0;

  // 成功接收的数据包数量。
  uint32_t rx_count = 0;

  // 最近一次收到的载荷长度。
  uint8_t last_rx_length = 0;

  // 最近一次收到的 SXLT 计数值是否有效。
  bool last_rx_counter_valid = false;

  // 最近一次收到的 SXLT 计数值。
  uint32_t last_rx_counter = 0;

  // 最近一次收到的数据文本。
  char last_rx_data[32] = {};

  // 最近一次接收 RSSI，单位 dBm。
  float last_rssi_dbm = 0.0f;

  // 最近一次接收 SNR，单位 dB。
  float last_snr_db = 0.0f;

  // 最近一次 RadioLib 错误码。
  int16_t last_error = 0;
};

/**
 * @brief 初始化 SX1262 LoRa。
 * @return 初始化成功或已经初始化时返回 true。
 */
bool InitializeSx1262Lora();

/**
 * @brief 关闭 SX1262 LoRa 并释放无线状态。
 */
void ShutdownSx1262Lora();

/**
 * @brief 处理 SX1262 接收和自动发送任务。
 * @return 状态有变化并需要刷新页面时返回 true。
 */
bool ProcessSx1262Lora();

/**
 * @brief 修改 SX1262 LoRa 工作频率。
 * @param frequency_mhz 新频率，单位 MHz。
 * @return 设置成功时返回 true。
 */
bool SetSx1262LoraFrequency(float frequency_mhz);

/**
 * @brief 设置 SX1262 LoRa 带宽。
 * @param bandwidth_khz 新带宽，单位 kHz，会自动匹配最接近的有效值。
 * @return 设置成功时返回 true。
 */
bool SetSx1262LoraBandwidth(float bandwidth_khz);

/**
 * @brief 根据输入值查找最接近的有效 SX1262 LoRa 带宽。
 * @param desired_khz 期望的带宽值，单位 kHz。
 * @return 最接近的有效带宽值，单位 kHz。
 */
float GetClosestValidBandwidthKhz(float desired_khz);

/**
 * @brief 设置 SX1262 LoRa 自动发送开关。
 * @param enable 为 true 时打开自动发送，并重置发送计数。
 */
void SetSx1262LoraAutoSend(bool enable);

/**
 * @brief 设置 SX1262 LoRa 自动发送间隔。
 * @param interval_ms 自动发送间隔，单位 ms。
 */
void SetSx1262LoraAutoSendInterval(uint32_t interval_ms);

/**
 * @brief 获取 SX1262 LoRa 自动发送状态。
 * @return 自动发送打开时返回 true。
 */
bool IsSx1262LoraAutoSendEnabled();

/**
 * @brief 获取 SX1262 LoRa 当前状态快照。
 * @return 当前 SX1262 LoRa 测试状态。
 */
Sx1262LoraInfo GetSx1262LoraInfo();
