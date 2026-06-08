/**
 * @file bluetooth_view.cpp
 * @brief Bluetooth 信息页面实现。
 */
#include "bluetooth_view.h"

#include <stdio.h>

#include "ble_uart_log.h"

namespace {

/**
 * @brief 将布尔状态转换为界面显示文本。
 * @param value 布尔状态。
 * @return 状态对应的英文文本。
 */
const char* BoolText(bool value) {
  return value ? "yes" : "no";
}

/**
 * @brief 生成 RSSI 的显示文本。
 * @param info BLE UART 状态快照。
 * @return RSSI 显示文本。
 */
std::string FormatRssi(const BleUartInfo& info) {
  if (!info.connected || !info.rssi_available) {
    return "unknown";
  }

  char line[24] = {};
  snprintf(line, sizeof(line), "%d dBm", info.rssi_dbm);
  return line;
}

/**
 * @brief 生成连接间隔的显示文本。
 * @param interval_units 连接间隔，单位为 1.25ms。
 * @return 连接间隔显示文本。
 */
std::string FormatConnectionInterval(uint16_t interval_units) {
  if (interval_units == 0) {
    return "unknown";
  }

  char line[24] = {};
  const uint32_t interval_x100_ms = interval_units * 125;
  snprintf(line, sizeof(line), "%lu.%02lums",
      static_cast<unsigned long>(interval_x100_ms / 100),
      static_cast<unsigned long>(interval_x100_ms % 100));
  return line;
}

/**
 * @brief 生成 BLE PHY 的显示文本。
 * @param phy BLE PHY 值。
 * @return PHY 显示文本。
 */
const char* FormatPhy(uint8_t phy) {
  switch (phy) {
    case 0x01:
      return "1 Mbps";
    case 0x02:
      return "2 Mbps";
    case 0x04:
      return "Coded";
    default:
      return "unknown";
  }
}

}  // namespace

std::vector<std::string> BuildBluetoothInfoLines() {
  const BleUartInfo info = GetBleUartInfo();
  std::vector<std::string> lines;

  lines.push_back("[Bluetooth]");
  lines.push_back(std::string("initialized: ") + BoolText(info.initialized));
  lines.push_back(std::string("connected: ") + BoolText(info.connected));
  lines.push_back(std::string("advertising: ") + BoolText(info.advertising));
  lines.push_back("");
  lines.push_back("[Device]");
  lines.push_back(std::string("name: ") + info.device_name);
  lines.push_back(std::string("service: ") + info.service_name);
  lines.push_back("");
  lines.push_back("[Central]");
  lines.push_back(std::string("name: ") +
                  (info.central_name[0] == '\0' ? "unknown"
                                                 : info.central_name));
  lines.push_back(std::string("rssi: ") + FormatRssi(info));
  lines.push_back(std::string("mtu: ") + std::to_string(info.mtu));
  lines.push_back(std::string("data length: ") +
                  std::to_string(info.data_length));
  lines.push_back(std::string("interval: ") +
                  FormatConnectionInterval(info.connection_interval_units));
  lines.push_back(std::string("phy: ") + FormatPhy(info.phy));
  return lines;
}
