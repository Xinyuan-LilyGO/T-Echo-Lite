/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:38:40
 * @License: GPL 3.0
 */
#include "ble_uart_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <bluefruit.h>

namespace {

constexpr size_t kLogBufferSize = 256;
constexpr char kBleDeviceName[] = "T-Echo-Lite-KeyShield";
constexpr char kBleServiceName[] = "BLE UART";
constexpr uint32_t kBleDisconnectWaitMs = 200;

BLEDfu bledfu;
BLEDis bledis;
BLEUart bleuart;
bool ble_uart_ready = false;
bool ble_uart_connected = false;
bool ble_advertising_started = false;
bool ble_advertising_configured = false;
bool ble_softdevice_enabled = false;
uint16_t ble_conn_handle = BLE_CONN_HANDLE_INVALID;
uint8_t ble_last_disconnect_reason = 0;
bool ble_rssi_available = false;
bool ble_rssi_monitoring = false;
int8_t ble_rssi_dbm = 0;
char ble_central_name[32] = {};
BleUartConnectionChangedCallback connection_changed_callback = nullptr;

/**
 * @brief 判断 BLE UART 当前是否可以写入。
 * @return BLE UART 已初始化且设备已连接时返回 true。
 */
bool IsBleUartWritable() {
  return ble_uart_ready && ble_uart_connected;
}

/**
 * @brief 根据当前连接句柄获取 BLE 连接对象。
 * @return BLE 已连接时返回连接对象，否则返回 nullptr。
 */
BLEConnection* GetCurrentConnection() {
  if (!ble_uart_connected || ble_conn_handle == BLE_CONN_HANDLE_INVALID) {
    return nullptr;
  }
  return Bluefruit.Connection(ble_conn_handle);
}

/**
 * @brief 主动刷新当前连接 RSSI 缓存。
 */
void RefreshRssi() {
  BLEConnection* connection = GetCurrentConnection();
  if (connection == nullptr) {
    ble_rssi_available = false;
    return;
  }

  if (!ble_rssi_monitoring) {
    ble_rssi_monitoring = connection->monitorRssi();
  }

  const int8_t rssi = connection->getRssi();
  if (rssi != 0) {
    ble_rssi_dbm = rssi;
    ble_rssi_available = true;
  }
}

/**
 * @brief 尝试刷新中心设备名称缓存。
 */
void RefreshCentralName() {
  BLEConnection* connection = GetCurrentConnection();
  if (connection == nullptr || ble_central_name[0] != '\0') {
    return;
  }

  char central_name[32] = {};
  if (connection->getPeerName(central_name, sizeof(central_name)) > 0) {
    strncpy(ble_central_name, central_name, sizeof(ble_central_name) - 1);
    ble_central_name[sizeof(ble_central_name) - 1] = '\0';
  }
}

/**
 * @brief 更新 BLE UART 连接状态并通知界面层。
 * @param connected BLE 中心设备已连接时为 true。
 */
void UpdateBleConnectionState(bool connected) {
  if (ble_uart_connected == connected) {
    return;
  }

  ble_uart_connected = connected;
  if (connection_changed_callback != nullptr) {
    connection_changed_callback(connected);
  }
}

/**
 * @brief 同时向硬件串口和 BLE UART 写入原始字节。
 * @param data 要写入的数据。
 * @param length 数据长度。
 */
void LogWrite(const uint8_t* data, size_t length) {
  if (data == nullptr || length == 0) {
    return;
  }

  Serial.write(data, length);
  if (IsBleUartWritable()) {
    bleuart.write(data, length);
  }
}

/**
 * @brief 处理 BLE 中心设备连接事件。
 * @param conn_handle BLE 连接句柄。
 */
void ConnectCallback(uint16_t conn_handle) {
  ble_conn_handle = conn_handle;
  ble_central_name[0] = '\0';
  ble_rssi_available = false;
  ble_rssi_monitoring = false;
  LogPrintln("BLE Connected");
  UpdateBleConnectionState(true);
}

/**
 * @brief 处理 BLE RSSI 变化事件。
 * @param conn_handle BLE 连接句柄。
 * @param rssi 当前连接 RSSI，单位 dBm。
 */
void RssiCallback(uint16_t conn_handle, int8_t rssi) {
  (void)conn_handle;
  ble_rssi_dbm = rssi;
  ble_rssi_available = true;
}

/**
 * @brief 处理 BLE 中心设备断开事件。
 * @param conn_handle BLE 连接句柄。
 * @param reason BLE 断开原因码。
 */
void DisconnectCallback(uint16_t conn_handle, uint8_t reason) {
  BLEConnection* connection = Bluefruit.Connection(conn_handle);
  if (connection != nullptr) {
    connection->stopRssi();
  }
  ble_last_disconnect_reason = reason;
  ble_rssi_available = false;
  ble_rssi_monitoring = false;
  ble_conn_handle = BLE_CONN_HANDLE_INVALID;
  UpdateBleConnectionState(false);
  LogPrintf("\nBLE Disconnected, reason = 0x%02X\n", reason);
}

/**
 * @brief 配置并启动 BLE 广播。
 */
void StartAdvertising() {
  if (ble_advertising_started) {
    return;
  }

  if (!ble_advertising_configured) {
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addService(bleuart);
    Bluefruit.ScanResponse.addName();
    ble_advertising_configured = true;
  }

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
  ble_advertising_started = true;
}

/**
 * @brief 清除 BLE UART 当前连接状态缓存。
 */
void ResetBleConnectionState() {
  ble_uart_connected = false;
  ble_advertising_started = false;
  ble_conn_handle = BLE_CONN_HANDLE_INVALID;
  ble_rssi_available = false;
  ble_rssi_monitoring = false;
  ble_rssi_dbm = 0;
  ble_central_name[0] = '\0';
}

/**
 * @brief 清除 BLE UART 运行状态缓存。
 */
void ResetBleRuntimeState() {
  ble_uart_ready = false;
  ble_advertising_configured = false;
  ResetBleConnectionState();
}

}  // namespace

bool InitializeBleUart() {
  if (ble_softdevice_enabled) {
    StartAdvertising();
    return true;
  }

  Bluefruit.autoConnLed(false);
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  if (!Bluefruit.begin()) {
    LogPrintln("BLE initialization failed");
    ble_softdevice_enabled = false;
    UpdateBleConnectionState(false);
    ResetBleRuntimeState();
    return false;
  }
  ble_softdevice_enabled = true;
  LogPrintln("BLE initialization successful");

  Bluefruit.setTxPower(8);
  Bluefruit.setRssiCallback(RssiCallback);
  Bluefruit.Periph.setConnectCallback(ConnectCallback);
  Bluefruit.Periph.setDisconnectCallback(DisconnectCallback);

  bledfu.begin();
  bledis.setManufacturer("LILYGO Industries");
  bledis.setModel("T-Echo-Lite-KeyShield");
  bledis.begin();
  bleuart.begin();
  ble_uart_ready = true;

  StartAdvertising();

  LogPrintln("BLE UART ready, please connect with BLE debugging tool");
  return true;
}

void ShutdownBleUart() {
  if (!ble_softdevice_enabled) {
    UpdateBleConnectionState(false);
    ResetBleRuntimeState();
    return;
  }

  Bluefruit.Advertising.restartOnDisconnect(false);
  Bluefruit.Advertising.stop();

  uint16_t conn_handles[BLE_MAX_CONNECTION] = {};
  const uint8_t conn_count =
      Bluefruit.getConnectedHandles(conn_handles, BLE_MAX_CONNECTION);
  for (uint8_t i = 0; i < conn_count; i++) {
    Bluefruit.disconnect(conn_handles[i]);
  }

  const uint32_t deadline = millis() + kBleDisconnectWaitMs;
  while (Bluefruit.connected() && millis() < deadline) {
    delay(1);
  }

  UpdateBleConnectionState(false);
  ResetBleConnectionState();
}

BleUartInfo GetBleUartInfo() {
  RefreshCentralName();
  RefreshRssi();

  BleUartInfo info;
  info.initialized = ble_uart_ready;
  info.connected = ble_uart_connected;
  info.advertising = ble_advertising_started;
  info.last_disconnect_reason = ble_last_disconnect_reason;
  info.rssi_available = ble_rssi_available;
  info.rssi_dbm = ble_rssi_dbm;
  strncpy(info.device_name, kBleDeviceName, sizeof(info.device_name) - 1);
  strncpy(info.service_name, kBleServiceName, sizeof(info.service_name) - 1);
  strncpy(info.central_name, ble_central_name, sizeof(info.central_name) - 1);

  BLEConnection* connection = GetCurrentConnection();
  if (connection != nullptr) {
    info.mtu = connection->getMtu();
    info.data_length = connection->getDataLength();
    info.connection_interval_units = connection->getConnectionInterval();
    info.phy = connection->getPHY();
  }

  return info;
}

void SetBleUartConnectionChangedCallback(
    BleUartConnectionChangedCallback callback) {
  connection_changed_callback = callback;
}

bool IsBleUartConnected() { return ble_uart_connected; }

int LogPrintf(const char* format, ...) {
  char buffer[kLogBufferSize] = {0};
  va_list args;
  va_start(args, format);
  const int length = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (length <= 0) {
    return length;
  }

  const size_t write_length =
      static_cast<size_t>(length) < sizeof(buffer)
          ? static_cast<size_t>(length)
          : sizeof(buffer) - 1;
  LogWrite(reinterpret_cast<const uint8_t*>(buffer), write_length);
  return length;
}

void LogPrint(const char* text) {
  if (text == nullptr) {
    return;
  }
  LogWrite(reinterpret_cast<const uint8_t*>(text), strlen(text));
}

void LogPrintln(const char* text) {
  LogPrint(text);
  LogPrint("\n");
}

void LogPrintln(const String& text) { LogPrintln(text.c_str()); }
