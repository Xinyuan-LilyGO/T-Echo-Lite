/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-08 15:15:13
 * @License: GPL 3.0
 */
#pragma once

#include <Arduino.h>

/**
 * @brief BLE UART 连接状态变化回调类型。
 */
using BleUartConnectionChangedCallback = void (*)(bool connected);

/**
 * @brief BLE UART 状态快照。
 */
struct BleUartInfo {
  // BLE UART 是否已经初始化完成。
  bool initialized = false;

  // BLE 中心设备当前是否已连接。
  bool connected = false;

  // BLE 广播是否已经启动。
  bool advertising = false;

  // 最近一次 BLE 断开原因码。
  uint8_t last_disconnect_reason = 0;

  // 当前 ATT MTU。
  uint16_t mtu = 0;

  // 当前数据包长度。
  uint16_t data_length = 0;

  // 当前连接间隔，单位为 1.25ms。
  uint16_t connection_interval_units = 0;

  // 当前 BLE PHY。
  uint8_t phy = 0;

  // 当前 RSSI 是否有效。
  bool rssi_available = false;

  // 当前 BLE 连接 RSSI，单位 dBm。
  int8_t rssi_dbm = 0;

  // 当前设备显示名。
  char device_name[32] = {};

  // 当前 BLE 服务名。
  char service_name[16] = {};

  // 最近一次连接的中心设备名。
  char central_name[32] = {};
};

/**
 * @brief 初始化 BLE UART 外设服务。
 * @return BLE UART 初始化成功时返回 true。
 */
bool InitializeBleUart();

/**
 * @brief 关闭 BLE UART 外设服务和 SoftDevice。
 */
void ShutdownBleUart();

/**
 * @brief 获取 BLE UART 当前状态信息。
 * @return BLE UART 状态快照。
 */
BleUartInfo GetBleUartInfo();

/**
 * @brief 注册 BLE UART 连接状态变化回调。
 * @param callback 连接状态变化时调用的回调；传入 nullptr 时取消回调。
 */
void SetBleUartConnectionChangedCallback(
    BleUartConnectionChangedCallback callback);

/**
 * @brief 获取 BLE UART 当前连接状态。
 * @return BLE 中心设备已连接时返回 true。
 */
bool IsBleUartConnected();

/**
 * @brief 同时向硬件串口和 BLE UART 输出格式化日志。
 * @param format printf 风格的格式化字符串。
 * @return 实际格式化出的字符数。
 */
int LogPrintf(const char* format, ...);

/**
 * @brief 同时向硬件串口和 BLE UART 输出字符串。
 * @param text 要输出的字符串。
 */
void LogPrint(const char* text);

/**
 * @brief 同时向硬件串口和 BLE UART 输出字符串并换行。
 * @param text 要输出的字符串。
 */
void LogPrintln(const char* text);

/**
 * @brief 同时向硬件串口和 BLE UART 输出 String 并换行。
 * @param text 要输出的 String。
 */
void LogPrintln(const String& text);
