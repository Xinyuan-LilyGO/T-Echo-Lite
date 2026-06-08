/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:38:36
 * @License: GPL 3.0
 */
#pragma once

#include <string>
#include <vector>

/**
 * @brief 生成 Bluetooth 信息页面需要显示的文本行。
 * @return 当前 BLE UART 状态信息文本列表。
 */
std::vector<std::string> BuildBluetoothInfoLines();
