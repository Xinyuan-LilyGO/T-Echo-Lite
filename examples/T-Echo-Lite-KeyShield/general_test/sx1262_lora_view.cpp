#include "sx1262_lora_view.h"

#include <SPI.h>

#include <algorithm>

#include "RadioLib.h"
#include "ble_uart_log.h"
#include "t_echo_lite_keyshield_config.h"

#ifndef LED_STATE_ON
#define LED_STATE_ON LOW
#endif

namespace {

static constexpr float kBandwidthKhz = 125.0f;
static constexpr float kValidBandwidthsKhz[] = {
    7.81f, 10.42f, 15.63f, 20.83f, 31.25f,
    41.67f, 62.5f, 125.0f, 250.0f, 500.0f};
static constexpr size_t kValidBandwidthCount =
    sizeof(kValidBandwidthsKhz) / sizeof(kValidBandwidthsKhz[0]);
static constexpr uint8_t kSpreadingFactor = 12;
static constexpr uint8_t kCodingRate = 8;
static constexpr uint8_t kSyncWord = RADIOLIB_SX126X_SYNC_WORD_PRIVATE;
static constexpr int8_t kOutputPowerDbm = 22;
static constexpr float kCurrentLimitMa = 140.0f;
static constexpr uint16_t kPreambleLength = 16;
static constexpr bool kCrcEnabled = true;
static constexpr uint32_t kDefaultAutoSendIntervalMs = 5000;
static constexpr uint32_t kMinAutoSendIntervalMs = 1000;
static constexpr uint8_t kMaxRxPayloadLength = 64;
static constexpr uint8_t kTxPayloadLength = 8;
static constexpr uint32_t kLoraActivityLedPulseMs = 80;
static constexpr int16_t kLoraTxLedPin = LED_1;
static constexpr int16_t kLoraRxLedPin = LED_2;

SPIClass sx1262_spi(NRF_SPIM3, SX1262_MISO, SX1262_SCLK, SX1262_MOSI);
SX1262 sx1262_radio =
    new Module(SX1262_CS, SX1262_DIO1, SX1262_RST, SX1262_BUSY, sx1262_spi);

volatile bool sx1262_packet_received = false;
Sx1262LoraInfo sx1262_info;
uint32_t next_auto_send_ms = 0;
uint32_t tx_led_off_deadline_ms = 0;
uint32_t rx_led_off_deadline_ms = 0;

void Sx1262PacketReceivedCallback() {
  sx1262_packet_received = true;
}

/**
 * @brief 设置 LoRa 活动指示灯亮灭。
 * @param pin 指示灯 GPIO。
 * @param enable 为 true 时点亮指示灯。
 */
void SetSx1262ActivityLed(int16_t pin, bool enable) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, enable ? LED_STATE_ON : !LED_STATE_ON);
}

/**
 * @brief 关闭 LoRa 收发活动指示灯。
 */
void ClearSx1262ActivityLeds() {
  SetSx1262ActivityLed(kLoraTxLedPin, false);
  SetSx1262ActivityLed(kLoraRxLedPin, false);
  tx_led_off_deadline_ms = 0;
  rx_led_off_deadline_ms = 0;
}

/**
 * @brief 点亮一次 LoRa 活动指示灯。
 * @param pin 指示灯 GPIO。
 * @param off_deadline_ms 指示灯关闭时间戳。
 */
void PulseSx1262ActivityLed(int16_t pin, uint32_t* off_deadline_ms) {
  if (off_deadline_ms == nullptr) {
    return;
  }
  SetSx1262ActivityLed(pin, true);
  *off_deadline_ms = millis() + kLoraActivityLedPulseMs;
}

/**
 * @brief 按时间自动关闭 LoRa 活动指示灯。
 */
void ProcessSx1262ActivityLeds() {
  const uint32_t now = millis();
  if (tx_led_off_deadline_ms != 0 &&
      static_cast<int32_t>(now - tx_led_off_deadline_ms) >= 0) {
    SetSx1262ActivityLed(kLoraTxLedPin, false);
    tx_led_off_deadline_ms = 0;
  }
  if (rx_led_off_deadline_ms != 0 &&
      static_cast<int32_t>(now - rx_led_off_deadline_ms) >= 0) {
    SetSx1262ActivityLed(kLoraRxLedPin, false);
    rx_led_off_deadline_ms = 0;
  }
}

void SetSx1262RfTransmitSwitch(bool transmit) {
  if (transmit) {
    digitalWrite(SX1262_RF_VC1, HIGH);
    digitalWrite(SX1262_RF_VC2, LOW);
    return;
  }

  digitalWrite(SX1262_RF_VC1, LOW);
  digitalWrite(SX1262_RF_VC2, HIGH);
}

bool ApplySx1262Parameters() {
  int16_t state = sx1262_radio.setFrequency(sx1262_info.frequency_mhz);
  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  state = sx1262_radio.setBandwidth(sx1262_info.bandwidth_khz);
  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  state = sx1262_radio.setOutputPower(kOutputPowerDbm);
  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  state = sx1262_radio.setCurrentLimit(kCurrentLimitMa);
  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  state = sx1262_radio.setPreambleLength(kPreambleLength);
  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  state = sx1262_radio.setCRC(kCrcEnabled);
  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  state = sx1262_radio.setSpreadingFactor(kSpreadingFactor);
  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  state = sx1262_radio.setCodingRate(kCodingRate);
  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  state = sx1262_radio.setSyncWord(kSyncWord);
  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  sx1262_info.last_error = RADIOLIB_ERR_NONE;
  return true;
}

bool StartSx1262Receive() {
  SetSx1262RfTransmitSwitch(false);
  const int16_t state = sx1262_radio.startReceive();
  sx1262_info.last_error = state;
  return state == RADIOLIB_ERR_NONE;
}

bool SendSx1262CounterPacket() {
  uint8_t packet[kTxPayloadLength] = {
      'S',
      'X',
      'L',
      'T',
      static_cast<uint8_t>(sx1262_info.tx_counter >> 24),
      static_cast<uint8_t>(sx1262_info.tx_counter >> 16),
      static_cast<uint8_t>(sx1262_info.tx_counter >> 8),
      static_cast<uint8_t>(sx1262_info.tx_counter),
  };

  PulseSx1262ActivityLed(kLoraTxLedPin, &tx_led_off_deadline_ms);
  SetSx1262RfTransmitSwitch(true);
  const int16_t state = sx1262_radio.transmit(packet, sizeof(packet));
  sx1262_info.last_error = state;
  sx1262_packet_received = false;
  StartSx1262Receive();

  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return false;
  }

  sx1262_info.tx_count++;
  sx1262_info.tx_counter++;
  return true;
}

void UpdateLastRxDataText(const uint8_t* packet, size_t length) {
  if (sx1262_info.last_rx_counter_valid) {
    snprintf(sx1262_info.last_rx_data, sizeof(sx1262_info.last_rx_data), "%lu",
        static_cast<unsigned long>(sx1262_info.last_rx_counter));
    return;
  }

  if (length == 0) {
    snprintf(sx1262_info.last_rx_data, sizeof(sx1262_info.last_rx_data),
        "empty");
    return;
  }

  size_t offset = 0;
  const size_t visible_length = std::min<size_t>(length, 8);
  for (size_t i = 0; i < visible_length &&
                     offset + 3 < sizeof(sx1262_info.last_rx_data);
       i++) {
    offset += snprintf(sx1262_info.last_rx_data + offset,
        sizeof(sx1262_info.last_rx_data) - offset, "%02X", packet[i]);
  }
  if (length > visible_length &&
      offset + 4 < sizeof(sx1262_info.last_rx_data)) {
    snprintf(sx1262_info.last_rx_data + offset,
        sizeof(sx1262_info.last_rx_data) - offset, "...");
  }
}

bool HandleReceivedPacket() {
  uint8_t packet[kMaxRxPayloadLength] = {};
  size_t length = sx1262_radio.getPacketLength();
  length = std::min(length, sizeof(packet));

  const int16_t state = sx1262_radio.readData(packet, length);
  sx1262_info.last_error = state;
  sx1262_packet_received = false;
  StartSx1262Receive();

  if (state != RADIOLIB_ERR_NONE) {
    sx1262_info.last_error = state;
    return true;
  }

  sx1262_info.rx_count++;
  sx1262_info.last_rx_length = static_cast<uint8_t>(length);
  sx1262_info.last_rssi_dbm = sx1262_radio.getRSSI();
  sx1262_info.last_snr_db = sx1262_radio.getSNR();
  sx1262_info.last_rx_counter_valid = false;

  if (length >= kTxPayloadLength && packet[0] == 'S' && packet[1] == 'X' &&
      packet[2] == 'L' && packet[3] == 'T') {
    sx1262_info.last_rx_counter =
        (static_cast<uint32_t>(packet[4]) << 24) |
        (static_cast<uint32_t>(packet[5]) << 16) |
        (static_cast<uint32_t>(packet[6]) << 8) |
        static_cast<uint32_t>(packet[7]);
    sx1262_info.last_rx_counter_valid = true;
  }

  UpdateLastRxDataText(packet, length);
  PulseSx1262ActivityLed(kLoraRxLedPin, &rx_led_off_deadline_ms);
  LogPrintf("SX1262 RX len=%u rssi=%.1f snr=%.1f\n",
      static_cast<unsigned int>(length), sx1262_info.last_rssi_dbm,
      sx1262_info.last_snr_db);
  return true;
}

}  // namespace

bool InitializeSx1262Lora() {
  if (sx1262_info.initialized) {
    return true;
  }

  pinMode(SX1262_RF_VC1, OUTPUT);
  pinMode(SX1262_RF_VC2, OUTPUT);
  SetSx1262RfTransmitSwitch(false);
  ClearSx1262ActivityLeds();

  sx1262_spi.begin();
  sx1262_spi.setClockDivider(SPI_CLOCK_DIV2);

  int16_t state = sx1262_radio.begin();
  sx1262_info.last_error = state;
  if (state != RADIOLIB_ERR_NONE) {
    LogPrintf("SX1262 initialization failed, error=%d\n", state);
    return false;
  }

  if (!ApplySx1262Parameters()) {
    LogPrintf("SX1262 parameter setup failed, error=%d\n",
        sx1262_info.last_error);
    return false;
  }

  sx1262_radio.setDio1Action(Sx1262PacketReceivedCallback);

  if (!StartSx1262Receive()) {
    LogPrintf("SX1262 start receive failed, error=%d\n",
        sx1262_info.last_error);
    sx1262_radio.clearDio1Action();
    return false;
  }

  sx1262_info.initialized = true;
  sx1262_info.auto_send_interval_ms = kDefaultAutoSendIntervalMs;
  next_auto_send_ms = millis() + sx1262_info.auto_send_interval_ms;
  LogPrintln("SX1262 LoRa initialized");
  return true;
}

void ShutdownSx1262Lora() {
  if (!sx1262_info.initialized) {
    return;
  }

  sx1262_radio.clearDio1Action();
  sx1262_radio.sleep();
  sx1262_packet_received = false;
  sx1262_info.initialized = false;
  sx1262_info.auto_send_enabled = false;
  sx1262_info.tx_counter = 0;
  SetSx1262RfTransmitSwitch(false);
  ClearSx1262ActivityLeds();
  LogPrintln("SX1262 LoRa shutdown");
}

bool ProcessSx1262Lora() {
  ProcessSx1262ActivityLeds();

  if (!sx1262_info.initialized) {
    return false;
  }

  bool screen_changed = false;
  if (sx1262_packet_received) {
    screen_changed = HandleReceivedPacket() || screen_changed;
  }

  if (sx1262_info.auto_send_enabled &&
      static_cast<int32_t>(millis() - next_auto_send_ms) >= 0) {
    SendSx1262CounterPacket();
    next_auto_send_ms = millis() + sx1262_info.auto_send_interval_ms;
  }

  return screen_changed;
}

bool SetSx1262LoraFrequency(float frequency_mhz) {
  sx1262_info.frequency_mhz = frequency_mhz;
  if (!sx1262_info.initialized) {
    return true;
  }

  const bool success = ApplySx1262Parameters() && StartSx1262Receive();
  if (success) {
    LogPrintf("SX1262 frequency set to %.3f MHz\n", frequency_mhz);
  } else {
    LogPrintf("SX1262 frequency setup failed, error=%d\n",
        sx1262_info.last_error);
  }
  return success;
}

float GetClosestValidBandwidthKhz(float desired_khz) {
  float best = kValidBandwidthsKhz[0];
  float best_diff = fabsf(desired_khz - best);
  for (size_t i = 1; i < kValidBandwidthCount; i++) {
    const float diff = fabsf(desired_khz - kValidBandwidthsKhz[i]);
    if (diff < best_diff) {
      best_diff = diff;
      best = kValidBandwidthsKhz[i];
    }
  }
  return best;
}

bool SetSx1262LoraBandwidth(float bandwidth_khz) {
  sx1262_info.bandwidth_khz = GetClosestValidBandwidthKhz(bandwidth_khz);
  if (!sx1262_info.initialized) {
    return true;
  }

  const bool success = ApplySx1262Parameters() && StartSx1262Receive();
  if (success) {
    LogPrintf("SX1262 bandwidth set to %.2f kHz\n",
        sx1262_info.bandwidth_khz);
  } else {
    LogPrintf("SX1262 bandwidth setup failed, error=%d\n",
        sx1262_info.last_error);
  }
  return success;
}

void SetSx1262LoraAutoSend(bool enable) {
  sx1262_info.auto_send_enabled = enable;
  sx1262_info.tx_counter = 0;
  next_auto_send_ms = millis() + sx1262_info.auto_send_interval_ms;
}

void SetSx1262LoraAutoSendInterval(uint32_t interval_ms) {
  sx1262_info.auto_send_interval_ms =
      std::max(interval_ms, kMinAutoSendIntervalMs);
  next_auto_send_ms = millis() + sx1262_info.auto_send_interval_ms;
}

bool IsSx1262LoraAutoSendEnabled() {
  return sx1262_info.auto_send_enabled;
}

Sx1262LoraInfo GetSx1262LoraInfo() {
  return sx1262_info;
}
