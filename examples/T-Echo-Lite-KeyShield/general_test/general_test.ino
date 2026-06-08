/*
 * @Description: original_test
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-08 16:55:08
 * @License: GPL 3.0
 */
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "Adafruit_SPIFlash.h"
#include "audio_view.h"
#include "battery_view.h"
#include "ble_uart_log.h"
#include "bluetooth_view.h"
#include "cpp_bus_driver_library.h"
#include "gps_view.h"
#include "home_view.h"
#include "imu_view.h"
#include "keyboard_view.h"
#include "lvgl.h"
#include "lvgl_port.h"
#include "sx1262_lora_view.h"
#include "t_echo_lite_keyshield_config.h"

static constexpr uint8_t kAudioMclkMultiple = 32;
static constexpr uint32_t kAudioSampleRate = 44100;
static constexpr uint8_t kAudioBitsPerSample = 16;
static constexpr uint32_t kAutoSleepTimeoutMs = 20000;
static constexpr uint16_t kAw21009MaxBrightness = 4095;
static constexpr uint8_t kVibrationSequence = 1;
static constexpr uint8_t kVibrationLoopCount = 1;
static constexpr uint8_t kVibrationGain = 255;
static constexpr cpp_bus_driver::Aw862xx::RamWaveformLibrary
    kAw86224RamWaveformLibrary =
        cpp_bus_driver::Aw862xx::RamWaveformLibrary::kRam12k041230_235;
static constexpr float kAdcReferenceMv = 3000.0f;
static constexpr float kAdcResolutionCount = 4096.0f;
static constexpr float kBatteryDividerRatio = 2.0f;
static constexpr float kBatteryEmptyVoltage = 3.6f;
static constexpr float kBatteryFullVoltage = 4.2f;
static constexpr uint8_t kBatteryAdcSampleCount = 16;
static constexpr uint32_t kScreenRefreshTaskPeriodMs = 10;
static constexpr uint16_t kScreenRefreshTaskStackSize = 2048;
static constexpr size_t kHomeVisibleLineCount = 10;
static constexpr size_t kHomeScrollStep = kHomeVisibleLineCount / 2;
static constexpr size_t kMaxPartialRefreshCount = 30;
static constexpr size_t kSx1262FrequencyTextMaxLength = 7;
static constexpr float kSx1262MinFrequencyMhz = 150.0f;
static constexpr float kSx1262MaxFrequencyMhz = 960.0f;

struct SleepOperator {
  enum class Mode : uint8_t {
    kNotSleep,
    kLightSleep,
  };
  size_t wake_deadline_ms = 0;
  Mode current_mode = Mode::kNotSleep;
};

enum class UiPage : uint8_t {
  kHome,
  kKeyboardTest,
  kAudioTest,
  kBatteryInfo,
  kBluetooth,
  kSx1262Lora,
  kGps,
  kImu,
};

enum class AudioTarget : uint8_t {
  kMic,
  kSpeaker,
};

enum class Sx1262LoraControl : uint8_t {
  kFrequency,
  kBandwidth,
  kAutoSend,
};

bool screen_refresh_flag = false;
UiPage current_page = UiPage::kHome;
bool page_selected = false;
size_t home_scroll_index = 0;
size_t battery_scroll_index = 0;
size_t bluetooth_scroll_index = 0;
uint8_t filtered_battery_percentage = 0;
battery_view::BatteryInfo battery_info_snapshot;
bool system_sleeping = false;
bool partial_refresh_flag = true;
size_t partial_refresh_count = 0;

// Audio state
AudioTarget audio_target = AudioTarget::kMic;
std::string audio_status_text = "Select Mic or Speaker";
bool audio_action_running = false;

// SX1262 LoRa state
Sx1262LoraControl sx1262_lora_control = Sx1262LoraControl::kFrequency;
std::string sx1262_lora_frequency_text = "868";
bool sx1262_lora_frequency_editing = false;
std::string sx1262_lora_bandwidth_text = "125";
bool sx1262_lora_bandwidth_editing = false;
bool sx1262_lora_init_attempted = false;

TaskHandle_t screen_refresh_task_handle = nullptr;

/**
 * @brief 获取 UI 页面名称。
 */
const char* GetUiPageName(UiPage page) {
  switch (page) {
    case UiPage::kHome:
      return "Home";
    case UiPage::kKeyboardTest:
      return "Keyboard";
    case UiPage::kAudioTest:
      return "Audio";
    case UiPage::kBatteryInfo:
      return "Battery";
    case UiPage::kBluetooth:
      return "Bluetooth";
    case UiPage::kSx1262Lora:
      return "LoRa";
    case UiPage::kGps:
      return "GPS";
    case UiPage::kImu:
      return "IMU";
    default:
      return "Unknown";
  }
}

/**
 * @brief 获取下一个 UI 页面。
 */
UiPage GetNextUiPage(UiPage page) {
  switch (page) {
    case UiPage::kHome:
      return UiPage::kKeyboardTest;
    case UiPage::kKeyboardTest:
      return UiPage::kAudioTest;
    case UiPage::kAudioTest:
      return UiPage::kBatteryInfo;
    case UiPage::kBatteryInfo:
      return UiPage::kBluetooth;
    case UiPage::kBluetooth:
      return UiPage::kSx1262Lora;
    case UiPage::kSx1262Lora:
      return UiPage::kGps;
    case UiPage::kGps:
      return UiPage::kImu;
    case UiPage::kImu:
      return UiPage::kHome;
    default:
      return UiPage::kHome;
  }
}

/**
 * @brief 获取上一个 UI 页面。
 */
UiPage GetPreviousUiPage(UiPage page) {
  switch (page) {
    case UiPage::kHome:
      return UiPage::kImu;
    case UiPage::kKeyboardTest:
      return UiPage::kHome;
    case UiPage::kAudioTest:
      return UiPage::kKeyboardTest;
    case UiPage::kBatteryInfo:
      return UiPage::kAudioTest;
    case UiPage::kBluetooth:
      return UiPage::kBatteryInfo;
    case UiPage::kSx1262Lora:
      return UiPage::kBluetooth;
    case UiPage::kGps:
      return UiPage::kSx1262Lora;
    case UiPage::kImu:
      return UiPage::kGps;
    default:
      return UiPage::kHome;
  }
}

/**
 * @brief 计算 Bluetooth 页面最大有效滚动索引。
 */
size_t GetBluetoothMaxScrollIndex() {
  const size_t line_count = BuildBluetoothInfoLines().size();
  return line_count > kHomeVisibleLineCount ? line_count - kHomeVisibleLineCount
                                            : 0;
}

/**
 * @brief 获取 TCA8418 使用的 I2C 总线实例。
 * @return I2C 总线共享指针引用。
 */
std::shared_ptr<cpp_bus_driver::HardwareI2c2>& GetTca8418I2cBus() {
  static auto tca8418_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c2>(
      TCA8418_SDA, TCA8418_SCL, &Wire);
  return tca8418_i2c_bus;
}

/**
 * @brief 获取 AW21009 使用的 I2C 总线实例。
 * @return I2C 总线共享指针引用。
 */
std::shared_ptr<cpp_bus_driver::HardwareI2c2>& GetAw21009I2cBus() {
  static auto aw21009_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c2>(
      AW21009_SDA, AW21009_SCL, &Wire);
  return aw21009_i2c_bus;
}

/**
 * @brief 获取 ES8311 使用的 I2C 总线实例。
 * @return I2C 总线共享指针引用。
 */
std::shared_ptr<cpp_bus_driver::HardwareI2c2>& GetEs8311I2cBus() {
  static auto es8311_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c2>(
      ES8311_SDA, ES8311_SCL, &Wire);
  return es8311_i2c_bus;
}

/**
 * @brief 获取 AW86224 使用的 I2C 总线实例。
 * @return I2C 总线共享指针引用。
 */
std::shared_ptr<cpp_bus_driver::HardwareI2c2>& GetAw86224I2cBus() {
  static auto aw86224_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c2>(
      AW86224_SDA, AW86224_SCL, &Wire);
  return aw86224_i2c_bus;
}

/**
 * @brief 获取 ES8311 使用的 I2S 总线实例。
 * @return I2S 总线共享指针引用。
 */
std::shared_ptr<cpp_bus_driver::HardwareI2s>& GetEs8311I2sBus() {
  static auto es8311_i2s_bus =
      std::make_shared<cpp_bus_driver::HardwareI2s>(ES8311_ADC_DATA,
          ES8311_DAC_DATA, ES8311_WS_LRCK, ES8311_BCLK, ES8311_MCLK);
  return es8311_i2s_bus;
}

/**
 * @brief 获取 TCA8418 驱动实例。
 * @return TCA8418 驱动对象引用。
 */
cpp_bus_driver::Tca8418& GetTca8418() {
  static auto tca8418 = std::make_unique<cpp_bus_driver::Tca8418>(
      GetTca8418I2cBus(), TCA8418_IIC_ADDRESS);
  return *tca8418;
}

/**
 * @brief 获取 AW21009 驱动实例。
 * @return AW21009 驱动对象引用。
 */
cpp_bus_driver::Aw21009& GetAw21009() {
  static auto aw21009 = std::make_unique<cpp_bus_driver::Aw21009>(
      GetAw21009I2cBus(), AW21009_IIC_ADDRESS);
  return *aw21009;
}

/**
 * @brief 获取 AW86224 驱动实例。
 * @return AW862xx 驱动对象引用。
 */
cpp_bus_driver::Aw862xx& GetAw86224() {
  static auto aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
      GetAw86224I2cBus(), AW86224_IIC_ADDRESS);
  return *aw86224;
}

/**
 * @brief 获取 ES8311 驱动实例。
 * @return ES8311 驱动对象引用。
 */
cpp_bus_driver::Es8311& GetEs8311() {
  static auto es8311 = std::make_unique<cpp_bus_driver::Es8311>(
      GetEs8311I2cBus(), GetEs8311I2sBus(), ES8311_IIC_ADDRESS);
  return *es8311;
}

SleepOperator sleep_op;
volatile bool boot_wake_requested = false;

/**
 * @brief BOOT 按键唤醒中断回调。
 */
void BootWakeInterruptCallback() { boot_wake_requested = true; }

/**
 * @brief 播放普通按键提示振动。
 */
void StartVibration() {
  auto& aw86224 = GetAw86224();
  if (!aw86224.PlayRamWaveform(
          kVibrationSequence, kVibrationLoopCount, kVibrationGain, true)) {
    printf("StartVibration failed\n");
  }
}

/**
 * @brief 播放任务完成提示振动。
 */
void StartCompletionVibration() {
  auto& aw86224 = GetAw86224();
  if (!aw86224.PlayRamWaveform(
          kVibrationSequence, kVibrationLoopCount, kVibrationGain, false)) {
    printf("StartCompletionVibration first vibration failed\n");
  }
  delay(100);
  if (!aw86224.PlayRamWaveform(
          kVibrationSequence, kVibrationLoopCount, kVibrationGain, true)) {
    printf("StartCompletionVibration second vibration failed\n");
  }
}

/**
 * @brief 初始化并配置 ES8311 音频芯片。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool InitEs8311() {
  auto& es8311 = GetEs8311();

  if (!es8311.Init()) {
    printf("es8311.Init fail\n");
    return false;
  }

  if (!es8311.Init(kAudioMclkMultiple, kAudioSampleRate, kAudioBitsPerSample)) {
    printf("es8311.I2s Init fail\n");
    return false;
  }

  cpp_bus_driver::Es8311::PowerStatus ps;
  ps.contorl.analog_circuits = true;
  ps.contorl.analog_bias_circuits = true;
  ps.contorl.analog_adc_bias_circuits = true;
  ps.contorl.analog_adc_reference_circuits = true;
  ps.contorl.analog_dac_reference_circuit = true;
  ps.contorl.internal_reference_circuits = false;
  ps.vmid = cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge;

  bool result = true;
  result &= es8311.SetPowerStatus(ps);
  result &= es8311.SetPgaPower(true);
  result &= es8311.SetAdcPower(true);
  result &= es8311.SetDacPower(true);
  result &= es8311.SetOutputToHpDrive(true);
  result &= es8311.SetAdcOffsetFreeze(
      cpp_bus_driver::Es8311::AdcOffsetFreeze::kDynamicHpf);
  result &= es8311.SetAdcHpfStage2Coeff(10);
  result &= es8311.SetDacEqualizer(false);
  result &= es8311.SetMic(cpp_bus_driver::Es8311::MicType::kAnalogMic,
      cpp_bus_driver::Es8311::MicInput::kMic1p1n);
  result &= es8311.SetAdcAutoVolumeControl(false);
  result &= es8311.SetAdcGain(cpp_bus_driver::Es8311::AdcGain::kGain18db);
  result &= es8311.SetAdcPgaGain(cpp_bus_driver::Es8311::AdcPgaGain::kGain30db);
  result &= es8311.SetAdcVolume(191);
  result &= es8311.SetDacVolume(191);

  if (!result) {
    printf("es8311 config fail\n");
  }
  return result;
}

/**
 * @brief 初始化并配置 TCA8418 键盘芯片。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool InitTca8418() {
  auto& tca8418 = GetTca8418();

  if (!tca8418.Init()) {
    printf("tca8418.Init fail\n");
    return false;
  }

  bool result = true;
  result &= tca8418.SetKeypadScanWindow(
      0, 0, TCA8418_KEYPAD_SCAN_WIDTH, TCA8418_KEYPAD_SCAN_HEIGHT);
  result &= tca8418.SetInterruptPulseMode(true);
  result &= tca8418.SetInterruptEnable(
      static_cast<uint8_t>(cpp_bus_driver::Tca8418::IrqMask::kKeyEvents) |
      static_cast<uint8_t>(cpp_bus_driver::Tca8418::IrqMask::kFifoOverflow));
  result &= tca8418.ClearEventFifo();
  result &= tca8418.ClearIrqFlag(cpp_bus_driver::Tca8418::IrqFlag::kAll);

  if (!result) {
    printf("tca8418 config fail\n");
  }
  return result;
}

/**
 * @brief 初始化并配置 AW21009 LED 芯片。
 * @param brightness 初始亮度。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool InitAw21009(uint16_t brightness) {
  auto& aw21009 = GetAw21009();
  const uint16_t safe_brightness =
      brightness > kAw21009MaxBrightness ? kAw21009MaxBrightness : brightness;

  bool result = true;
  result &= aw21009.Init();
  result &= aw21009.SetBrightness(
      cpp_bus_driver::Aw21009::LedChannel::kAll, safe_brightness);

  if (!result) {
    printf("aw21009 config fail\n");
  }
  return result;
}

/**
 * @brief 初始化并配置 AW86224 触觉芯片。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool InitAw86224() {
  auto& aw86224 = GetAw86224();

  if (!aw86224.Init(500000)) {
    printf("aw86224.Init fail\n");
    return false;
  }

  const uint32_t detected_f0 = aw86224.GetF0Detection();
  if (detected_f0 == 0 || detected_f0 == static_cast<uint32_t>(-1)) {
    printf("aw86224 f0 reference read fail\n");
  } else {
    printf("aw86224 f0 reference: %u.%uHz\n",
        static_cast<unsigned int>(detected_f0 / 10),
        static_cast<unsigned int>(detected_f0 % 10));
  }

  const auto info =
      cpp_bus_driver::Aw862xx::GetRamWaveformInfo(kAw86224RamWaveformLibrary);
  printf("aw86224 selected ram library: %s\n",
      info.name == nullptr ? "unknown" : info.name);

  return aw86224.InitRamMode(kAw86224RamWaveformLibrary);
}

/**
 * @brief 切换当前 UI 页面。
 */
void SelectUiPage(UiPage page) {
  if (current_page == UiPage::kAudioTest && page != UiPage::kAudioTest) {
    audio_view::Stop(GetEs8311());
  }
  if (current_page != page && page == UiPage::kBatteryInfo) {
    battery_scroll_index = 0;
    UpdateStatusBar();
    RefreshBatteryInfoSnapshot();
  }
  if (current_page == UiPage::kSx1262Lora && page != UiPage::kSx1262Lora) {
    ShutdownSx1262Lora();
    sx1262_lora_init_attempted = false;
    page_selected = false;
  }
  if (current_page != UiPage::kSx1262Lora && page == UiPage::kSx1262Lora) {
    sx1262_lora_control = Sx1262LoraControl::kFrequency;
    sx1262_lora_frequency_editing = false;
    sx1262_lora_bandwidth_editing = false;
    SyncSx1262LoraFrequencyText();
    sx1262_lora_init_attempted = true;
    InitializeSx1262Lora();
  }
  if (current_page == UiPage::kGps && page != UiPage::kGps) {
    ShutdownGps();
  }
  if (current_page != UiPage::kGps && page == UiPage::kGps) {
    InitializeGps();
  }
  if (current_page == UiPage::kImu && page != UiPage::kImu) {
    ShutdownImu();
  }
  if (current_page != UiPage::kImu && page == UiPage::kImu) {
    InitializeImu();
  }
  current_page = page;
}

// ========== SX1262 LoRa 按键处理 ==========

/**
 * @brief 判断按键是否为单个数字字符。
 * @param key_text 按键文本。
 * @return 是数字时返回 true。
 */
bool IsSx1262FrequencyDigitKey(const std::string& key_text) {
  return key_text.size() == 1 && key_text[0] >= '0' && key_text[0] <= '9';
}

void SyncSx1262LoraFrequencyText() {
  char text[12] = {};
  snprintf(text, sizeof(text), "%.3f", GetSx1262LoraInfo().frequency_mhz);
  sx1262_lora_frequency_text = text;
  while (sx1262_lora_frequency_text.size() > 1 &&
         sx1262_lora_frequency_text.back() == '0') {
    sx1262_lora_frequency_text.pop_back();
  }
  if (!sx1262_lora_frequency_text.empty() &&
      sx1262_lora_frequency_text.back() == '.') {
    sx1262_lora_frequency_text.pop_back();
  }
}

bool ParseSx1262LoraFrequencyText(float* frequency_mhz) {
  if (frequency_mhz == nullptr || sx1262_lora_frequency_text.empty() ||
      sx1262_lora_frequency_text == ".") {
    return false;
  }
  char* end = nullptr;
  const char* begin = sx1262_lora_frequency_text.c_str();
  const float parsed_frequency = std::strtof(begin, &end);
  if (end == begin || *end != '\0') return false;
  if (parsed_frequency < kSx1262MinFrequencyMhz ||
      parsed_frequency > kSx1262MaxFrequencyMhz) {
    return false;
  }
  *frequency_mhz = parsed_frequency;
  return true;
}

bool CommitSx1262LoraFrequencyText() {
  float frequency_mhz = 0.0f;
  if (!ParseSx1262LoraFrequencyText(&frequency_mhz)) {
    SyncSx1262LoraFrequencyText();
    return false;
  }
  const bool success = SetSx1262LoraFrequency(frequency_mhz);
  SyncSx1262LoraFrequencyText();
  return success;
}

void SyncSx1262LoraBandwidthText() {
  const Sx1262LoraInfo info = GetSx1262LoraInfo();
  char text[12] = {};
  snprintf(text, sizeof(text), "%.2f", info.bandwidth_khz);
  sx1262_lora_bandwidth_text = text;
  while (sx1262_lora_bandwidth_text.size() > 1 &&
         sx1262_lora_bandwidth_text.back() == '0') {
    sx1262_lora_bandwidth_text.pop_back();
  }
  if (!sx1262_lora_bandwidth_text.empty() &&
      sx1262_lora_bandwidth_text.back() == '.') {
    sx1262_lora_bandwidth_text.pop_back();
  }
}

bool CommitSx1262LoraBandwidthText() {
  if (sx1262_lora_bandwidth_text.empty() ||
      sx1262_lora_bandwidth_text == ".") {
    SyncSx1262LoraBandwidthText();
    return false;
  }
  const float desired_khz = strtof(sx1262_lora_bandwidth_text.c_str(), nullptr);
  if (desired_khz <= 0) {
    SyncSx1262LoraBandwidthText();
    return false;
  }
  const bool success = SetSx1262LoraBandwidth(desired_khz);
  SyncSx1262LoraBandwidthText();
  return success;
}

void MoveSx1262LoraControlDown() {
  switch (sx1262_lora_control) {
    case Sx1262LoraControl::kFrequency:
      sx1262_lora_control = Sx1262LoraControl::kBandwidth; break;
    case Sx1262LoraControl::kBandwidth:
      sx1262_lora_control = Sx1262LoraControl::kAutoSend; break;
    case Sx1262LoraControl::kAutoSend:
      sx1262_lora_control = Sx1262LoraControl::kFrequency; break;
  }
}

void MoveSx1262LoraControlUp() {
  switch (sx1262_lora_control) {
    case Sx1262LoraControl::kFrequency:
      sx1262_lora_control = Sx1262LoraControl::kAutoSend; break;
    case Sx1262LoraControl::kBandwidth:
      sx1262_lora_control = Sx1262LoraControl::kFrequency; break;
    case Sx1262LoraControl::kAutoSend:
      sx1262_lora_control = Sx1262LoraControl::kBandwidth; break;
  }
}

void ToggleSx1262LoraAutoSend() {
  if (GetSx1262LoraInfo().initialized || InitializeSx1262Lora()) {
    SetSx1262LoraAutoSend(!IsSx1262LoraAutoSendEnabled());
  }
}

bool AppendSx1262LoraFrequencyKey(const std::string& key_text) {
  if (IsSx1262FrequencyDigitKey(key_text)) {
    if (sx1262_lora_frequency_text.size() < kSx1262FrequencyTextMaxLength) {
      sx1262_lora_frequency_text += key_text;
    }
    return true;
  }
  if (key_text == "*") {
    if (sx1262_lora_frequency_text.find('.') == std::string::npos &&
        sx1262_lora_frequency_text.size() < kSx1262FrequencyTextMaxLength) {
      if (sx1262_lora_frequency_text.empty()) {
        sx1262_lora_frequency_text = "0";
      }
      sx1262_lora_frequency_text += ".";
    }
    return true;
  }
  if (key_text == "#") {
    if (!sx1262_lora_frequency_text.empty()) {
      sx1262_lora_frequency_text.pop_back();
    }
    return true;
  }
  if (key_text == "No") {
    sx1262_lora_frequency_text.clear();
    return true;
  }
  return false;
}

bool HandleSx1262LoraFrequencyEditKey(const std::string& key_text) {
  if (AppendSx1262LoraFrequencyKey(key_text)) return true;
  if (key_text == "Center" || key_text == "Yes") {
    CommitSx1262LoraFrequencyText();
    sx1262_lora_frequency_editing = false;
    return true;
  }
  if (key_text == "Esc") {
    SyncSx1262LoraFrequencyText();
    sx1262_lora_frequency_editing = false;
    return true;
  }
  return false;
}

bool AppendSx1262LoraBandwidthKey(const std::string& key_text) {
  if (IsSx1262FrequencyDigitKey(key_text)) {
    if (sx1262_lora_bandwidth_text.size() < kSx1262FrequencyTextMaxLength) {
      sx1262_lora_bandwidth_text += key_text;
    }
    return true;
  }
  if (key_text == "*") {
    if (sx1262_lora_bandwidth_text.find('.') == std::string::npos &&
        sx1262_lora_bandwidth_text.size() < kSx1262FrequencyTextMaxLength) {
      if (sx1262_lora_bandwidth_text.empty()) {
        sx1262_lora_bandwidth_text = "0";
      }
      sx1262_lora_bandwidth_text += ".";
    }
    return true;
  }
  if (key_text == "#") {
    if (!sx1262_lora_bandwidth_text.empty()) {
      sx1262_lora_bandwidth_text.pop_back();
    }
    return true;
  }
  if (key_text == "No") {
    sx1262_lora_bandwidth_text.clear();
    return true;
  }
  return false;
}

bool HandleSx1262LoraBandwidthEditKey(const std::string& key_text) {
  if (AppendSx1262LoraBandwidthKey(key_text)) return true;
  if (key_text == "Center" || key_text == "Yes") {
    CommitSx1262LoraBandwidthText();
    sx1262_lora_bandwidth_editing = false;
    return true;
  }
  if (key_text == "Esc") {
    SyncSx1262LoraBandwidthText();
    sx1262_lora_bandwidth_editing = false;
    return true;
  }
  return false;
}

bool HandleSx1262LoraPageKey(const std::string& key_text) {
  if (sx1262_lora_frequency_editing) {
    return HandleSx1262LoraFrequencyEditKey(key_text);
  }
  if (sx1262_lora_bandwidth_editing) {
    return HandleSx1262LoraBandwidthEditKey(key_text);
  }
  if (key_text == "Down") {
    if (!GetSx1262LoraInfo().initialized) return false;
    MoveSx1262LoraControlDown(); return true; }
  if (key_text == "Up") {
    if (!GetSx1262LoraInfo().initialized) return false;
    MoveSx1262LoraControlUp(); return true; }
  if (key_text == "Center") {
    if (!GetSx1262LoraInfo().initialized) {
      sx1262_lora_init_attempted = true;
      InitializeSx1262Lora();
      return true;
    }
    if (sx1262_lora_control == Sx1262LoraControl::kFrequency) {
      sx1262_lora_frequency_editing = true;
      sx1262_lora_frequency_text.clear();
    } else if (sx1262_lora_control == Sx1262LoraControl::kBandwidth) {
      sx1262_lora_bandwidth_editing = true;
      sx1262_lora_bandwidth_text.clear();
    } else {
      ToggleSx1262LoraAutoSend();
    }
    return true;
  }
  if (key_text == "Yes") {
    sx1262_lora_control = Sx1262LoraControl::kAutoSend;
    ToggleSx1262LoraAutoSend();
    return true;
  }
  if (key_text == "Esc") {
    page_selected = false;
    sx1262_lora_control = Sx1262LoraControl::kFrequency;
    sx1262_lora_frequency_editing = false;
    sx1262_lora_bandwidth_editing = false;
    SyncSx1262LoraFrequencyText();
    return true;
  }
  return false;
}

// ========== 屏幕刷新 ==========

/**
 * @brief 向屏幕刷新任务请求一次刷新。
 * @param partial_refresh 为 true 时允许使用局部刷新。
 */
void RequestScreenRefresh(bool partial_refresh) {
  if (!partial_refresh) {
    partial_refresh_count = 0;
  }
  partial_refresh_flag =
      screen_refresh_flag ? partial_refresh_flag && partial_refresh
                          : partial_refresh;
  screen_refresh_flag = true;
}

/**
 * @brief 请求键盘文本刷新，定期用快刷清理残影。
 */
void RequestKeyboardTextRefresh() {
  partial_refresh_count++;
  if (partial_refresh_count > kMaxPartialRefreshCount) {
    RequestScreenRefresh(false);
    return;
  }
  RequestScreenRefresh(true);
}

/**
 * @brief 处理 BLE UART 连接状态变化并刷新状态栏。
 * @param connected 为 true 时表示 BLE 已连接。
 */
void HandleBleUartConnectionChanged(bool connected) {
  ResetAutoSleepTimer();
  lvgl_port::SetBleConnected(connected);
  RequestScreenRefresh(true);
}

/**
 * @brief 重置自动休眠计时器。
 */
void ResetAutoSleepTimer() {
  sleep_op.wake_deadline_ms = millis() + kAutoSleepTimeoutMs;
}

/**
 * @brief 配置电池电压检测 ADC。
 */
void ConfigureBatteryMeasurement() {
  pinMode(BATTERY_ADC_DATA, INPUT);
  pinMode(BATTERY_MEASUREMENT_CONTROL, OUTPUT);
  digitalWrite(BATTERY_MEASUREMENT_CONTROL, HIGH);

  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
}

/**
 * @brief 根据电池电压计算百分比。
 * @param voltage 电池电压，单位为 V。
 * @return 电池百分比，范围 0~100。
 */
uint8_t CalculateBatteryPercentage(float voltage) {
  if (voltage <= kBatteryEmptyVoltage) {
    return 0;
  }
  if (voltage >= kBatteryFullVoltage) {
    return 100;
  }

  return static_cast<uint8_t>(((voltage - kBatteryEmptyVoltage) * 100.0f) /
                              (kBatteryFullVoltage - kBatteryEmptyVoltage));
}

/**
 * @brief 读取当前电池信息快照。
 * @return 电池信息快照。
 */
battery_view::BatteryInfo ReadBatteryInfo() {
  uint32_t adc_sum = 0;
  for (uint8_t i = 0; i < kBatteryAdcSampleCount; i++) {
    adc_sum += analogRead(BATTERY_ADC_DATA);
  }

  const float adc_average =
      static_cast<float>(adc_sum) / kBatteryAdcSampleCount;
  const float adc_voltage_mv =
      adc_average * (kAdcReferenceMv / kAdcResolutionCount);
  const float battery_voltage =
      (adc_voltage_mv / 1000.0f) * kBatteryDividerRatio;

  battery_view::BatteryInfo info;
  info.has_data = true;
  info.adc_raw = static_cast<uint16_t>(adc_average + 0.5f);
  info.adc_voltage_mv = adc_voltage_mv;
  info.battery_voltage = battery_voltage;
  info.percentage = CalculateBatteryPercentage(battery_voltage);
  info.filtered_percentage = filtered_battery_percentage;
  info.sample_count = kBatteryAdcSampleCount;
  info.reference_mv = kAdcReferenceMv;
  info.adc_resolution = kAdcResolutionCount;
  info.divider_ratio = kBatteryDividerRatio;
  info.empty_voltage = kBatteryEmptyVoltage;
  info.full_voltage = kBatteryFullVoltage;
  return info;
}

/**
 * @brief 读取当前电池电压。
 * @return 电池电压，单位为 V。
 */
float ReadBatteryVoltage() {
  return ReadBatteryInfo().battery_voltage;
}

/**
 * @brief 读取当前电池百分比。
 * @return 电池百分比，范围 0~100。
 */
uint8_t ReadBatteryPercentage() {
  return ReadBatteryInfo().percentage;
}

/**
 * @brief 刷新电池信息页面缓存快照。
 */
void RefreshBatteryInfoSnapshot() {
  battery_info_snapshot = ReadBatteryInfo();
  battery_info_snapshot.filtered_percentage = filtered_battery_percentage;
}

/**
 * @brief 消费刷新请求并调用 LVGL 渲染当前页面。
 */
void ProcessPendingScreenRefresh() {
  if (!screen_refresh_flag) return;
  screen_refresh_flag = false;

  UpdateStatusBar();
  RefreshCurrentPage(false);
  partial_refresh_flag = true;
}

/**
 * @brief 刷新当前 UI 页面。
 * @param busy_enable 是否显示忙碌状态。
 */
void RefreshCurrentPage(bool busy_enable) {
  if (current_page == UiPage::kHome) {
    const std::vector<std::string> home_lines = home_view::CreateLines();
    lvgl_port::ShowHomeScreen(home_lines, home_scroll_index,
        GetUiPageName(current_page), page_selected, partial_refresh_flag,
        busy_enable);
  } else if (current_page == UiPage::kAudioTest) {
    lvgl_port::ShowAudioScreen(page_selected,
        audio_target == AudioTarget::kMic, audio_status_text.c_str(),
        GetUiPageName(current_page), partial_refresh_flag, busy_enable,
        audio_action_running);
  } else if (current_page == UiPage::kBatteryInfo) {
    const std::vector<std::string> battery_lines =
        battery_view::CreateLines(battery_info_snapshot);
    lvgl_port::ShowHomeScreen(battery_lines, battery_scroll_index,
        GetUiPageName(current_page), page_selected, partial_refresh_flag,
        busy_enable);
  } else if (current_page == UiPage::kBluetooth) {
    const std::vector<std::string> bluetooth_lines = BuildBluetoothInfoLines();
    bluetooth_scroll_index =
        std::min(bluetooth_scroll_index, GetBluetoothMaxScrollIndex());
    lvgl_port::ShowHomeScreen(bluetooth_lines, bluetooth_scroll_index,
        GetUiPageName(current_page), page_selected, partial_refresh_flag,
        busy_enable);
  } else if (current_page == UiPage::kSx1262Lora) {
    const Sx1262LoraInfo sx1262_info = GetSx1262LoraInfo();
    if (!sx1262_info.initialized) {
      std::vector<std::string> lines;
      lines.push_back("[sx1262 lora]");
      lines.push_back("");
      lines.push_back("LoRa module init failed");
      lvgl_port::ShowHomeScreen(lines, 0,
          GetUiPageName(current_page), page_selected, partial_refresh_flag,
          busy_enable);
    } else {
      char rssi_text[24] = {};
      char snr_text[24] = {};
      if (sx1262_info.rx_count > 0) {
        snprintf(rssi_text, sizeof(rssi_text), "rssi: %.1fdBm",
            sx1262_info.last_rssi_dbm);
        snprintf(snr_text, sizeof(snr_text), "snr: %.1fdB",
            sx1262_info.last_snr_db);
      } else {
        snprintf(rssi_text, sizeof(rssi_text), "rssi: unknown");
        snprintf(snr_text, sizeof(snr_text), "snr: unknown");
      }
      lvgl_port::Sx1262LoraScreenState sx1262_state;
      sx1262_state.frequency_text = sx1262_lora_frequency_text.c_str();
      sx1262_state.bandwidth_text = sx1262_lora_bandwidth_text.c_str();
      sx1262_state.auto_send_enabled = sx1262_info.auto_send_enabled;
      sx1262_state.rx_data =
          sx1262_info.rx_count > 0 ? sx1262_info.last_rx_data : "none";
      sx1262_state.rssi_text = rssi_text;
      sx1262_state.snr_text = snr_text;
      sx1262_state.page_selected = page_selected;
      sx1262_state.frequency_selected =
          sx1262_lora_control == Sx1262LoraControl::kFrequency;
      sx1262_state.frequency_editing = sx1262_lora_frequency_editing;
      sx1262_state.bandwidth_selected =
          sx1262_lora_control == Sx1262LoraControl::kBandwidth;
      sx1262_state.bandwidth_editing = sx1262_lora_bandwidth_editing;
      sx1262_state.auto_send_selected =
          sx1262_lora_control == Sx1262LoraControl::kAutoSend;
      lvgl_port::ShowSx1262LoraScreen(sx1262_state,
          GetUiPageName(current_page), partial_refresh_flag, busy_enable);
    }
  } else if (current_page == UiPage::kGps) {
    const GpsInfo gps_info = GetGpsInfo();
    char lat_text[32] = {}, lon_text[32] = {}, sat_text[32] = {};
    char cn0_text[32] = {}, dop_text[32] = {}, speed_text[32] = {};
    char time_text[48] = {}, fix_time_text[32] = {};

    if (gps_info.has_fix) {
      snprintf(lat_text, sizeof(lat_text), "lat: %.6f", gps_info.latitude);
      snprintf(lon_text, sizeof(lon_text), "lon: %.6f", gps_info.longitude);
      snprintf(fix_time_text, sizeof(fix_time_text),
          "time to fix: %lu s",
          static_cast<unsigned long>(gps_info.time_to_first_fix_s));
    } else if (gps_info.module_found) {
      snprintf(fix_time_text, sizeof(fix_time_text),
          "waiting: %lu s",
          static_cast<unsigned long>(gps_info.time_to_first_fix_s));
    }
    snprintf(sat_text, sizeof(sat_text), "sat: %u used / %u visible",
        static_cast<unsigned int>(gps_info.satellites_used),
        static_cast<unsigned int>(gps_info.satellites_visible));
    if (gps_info.max_cn0 > 0) {
      snprintf(cn0_text, sizeof(cn0_text), "max cn0: %d dBHz",
          static_cast<int>(gps_info.max_cn0));
    }
    if (gps_info.has_fix) {
      snprintf(dop_text, sizeof(dop_text), "dop: h %.1f | v %.1f | p %.1f",
          gps_info.hdop, gps_info.vdop, gps_info.pdop);
      snprintf(speed_text, sizeof(speed_text), "speed: %.1f km/h",
          gps_info.speed_kmh);
    }
    if (gps_info.utc_valid && gps_info.date_valid) {
      const uint8_t china_hour = (gps_info.utc_hour + 8) % 24;
      snprintf(time_text, sizeof(time_text),
          "time: %02u/%02u/%04u %02u:%02u:%05.2f",
          static_cast<unsigned int>(gps_info.utc_day),
          static_cast<unsigned int>(gps_info.utc_month),
          static_cast<unsigned int>(gps_info.utc_year),
          static_cast<unsigned int>(china_hour),
          static_cast<unsigned int>(gps_info.utc_minute),
          static_cast<double>(gps_info.utc_second));
    } else if (gps_info.module_found) {
      snprintf(time_text, sizeof(time_text), "time: waiting...");
    }

    lvgl_port::GpsScreenState gps_state;
    gps_state.module_found = gps_info.module_found;
    gps_state.has_fix = gps_info.has_fix;
    gps_state.latitude_text = gps_info.has_fix ? lat_text : nullptr;
    gps_state.longitude_text = gps_info.has_fix ? lon_text : nullptr;
    gps_state.satellites_text = gps_info.module_found ? sat_text : nullptr;
    gps_state.cn0_text = gps_info.max_cn0 > 0 ? cn0_text : nullptr;
    gps_state.dop_text = gps_info.has_fix ? dop_text : nullptr;
    gps_state.speed_text = gps_info.has_fix ? speed_text : nullptr;
    gps_state.time_text = (gps_info.module_found || gps_info.utc_valid)
        ? time_text : nullptr;
    gps_state.fix_time_text = gps_info.module_found ? fix_time_text : nullptr;
    gps_state.page_selected = page_selected;
    lvgl_port::ShowGpsScreen(gps_state, GetUiPageName(current_page),
        partial_refresh_flag, busy_enable);
  } else if (current_page == UiPage::kImu) {
    const ImuInfo imu_info = GetImuInfo();
    char pitch_text[24] = {}, roll_text[24] = {};
    char yaw_text[24] = {}, temp_text[24] = {};
    if (imu_info.data_valid) {
      snprintf(pitch_text, sizeof(pitch_text),
          "pitch: %6.1f deg", static_cast<double>(imu_info.pitch));
      snprintf(roll_text, sizeof(roll_text),
          "roll:  %6.1f deg", static_cast<double>(imu_info.roll));
      snprintf(yaw_text, sizeof(yaw_text),
          "yaw:   %6.1f deg", static_cast<double>(imu_info.yaw));
      snprintf(temp_text, sizeof(temp_text),
          "temp:  %.1f C", static_cast<double>(imu_info.temperature));
    }
    lvgl_port::ImuScreenState imu_state;
    imu_state.module_found = imu_info.module_found;
    imu_state.pitch_text = imu_info.data_valid ? pitch_text : nullptr;
    imu_state.roll_text = imu_info.data_valid ? roll_text : nullptr;
    imu_state.yaw_text = imu_info.data_valid ? yaw_text : nullptr;
    imu_state.temp_text = imu_info.data_valid ? temp_text : nullptr;
    imu_state.page_selected = page_selected;
    lvgl_port::ShowImuScreen(imu_state, GetUiPageName(current_page),
        partial_refresh_flag, busy_enable);
  } else {
    lvgl_port::ShowTextList(keyboard_view::GetTextList(),
        GetUiPageName(current_page), page_selected, partial_refresh_flag,
        busy_enable);
  }
}
void UpdateStatusBar() {
  static bool initialized = false;

  const uint8_t current_percentage = ReadBatteryPercentage();
  if (!initialized) {
    filtered_battery_percentage = current_percentage;
    initialized = true;
  } else {
    const int16_t delta =
        static_cast<int16_t>(current_percentage) - filtered_battery_percentage;
    if (delta >= 2 || delta <= -2) {
      filtered_battery_percentage = static_cast<uint8_t>(
          (static_cast<uint16_t>(filtered_battery_percentage) * 3 +
              current_percentage) /
          4);
    }
  }

  lvgl_port::SetBatteryPercentage(filtered_battery_percentage);
}

/**
 * @brief 屏幕刷新任务入口。
 */
void ScreenRefreshTask(void* arg) {
  (void)arg;
  LogPrintln("ScreenRefreshTask start");

  while (true) {
    if (!system_sleeping) {
      ProcessPendingScreenRefresh();
      lvgl_port::Tick(kScreenRefreshTaskPeriodMs);
    }

    vTaskDelay(pdMS_TO_TICKS(kScreenRefreshTaskPeriodMs));
  }
}

/**
 * @brief 设置系统休眠或唤醒状态。
 */
void SetSystemSleep(bool enable) {
  if (enable) {
    system_sleeping = true;
    if (screen_refresh_task_handle != nullptr) {
      vTaskSuspend(screen_refresh_task_handle);
    }

    Serial.end();

    // 停止 BLE
    ShutdownBleUart();

    // 复位所有 GPIO 为默认状态（最低功耗）
    nrf_gpio_cfg_default(SCREEN_MISO);
    nrf_gpio_cfg_default(SCREEN_SCLK);
    nrf_gpio_cfg_default(SCREEN_MOSI);
    nrf_gpio_cfg_default(SCREEN_DC);
    nrf_gpio_cfg_default(SCREEN_RST);
    nrf_gpio_cfg_default(SCREEN_CS);
    nrf_gpio_cfg_default(SCREEN_SRAM_CS);
    nrf_gpio_cfg_default(SCREEN_BUSY);
    nrf_gpio_cfg_default(SCREEN_BS1);
    lvgl_port::EndDisplay();

    nrf_gpio_cfg_default(IIC_1_SDA);
    nrf_gpio_cfg_default(IIC_1_SCL);
    nrf_gpio_cfg_default(TCA8418_INT);
    Wire.end();

    nrf_gpio_cfg_default(BATTERY_MEASUREMENT_CONTROL);
    nrf_gpio_cfg_default(BATTERY_ADC_DATA);

    nrf_gpio_cfg_default(SX1262_MISO);
    nrf_gpio_cfg_default(SX1262_MOSI);
    nrf_gpio_cfg_default(SX1262_SCLK);
    nrf_gpio_cfg_default(SX1262_CS);
    nrf_gpio_cfg_default(SX1262_DIO1);
    nrf_gpio_cfg_default(SX1262_RST);
    nrf_gpio_cfg_default(SX1262_BUSY);
    nrf_gpio_cfg_default(SX1262_RF_VC1);
    nrf_gpio_cfg_default(SX1262_RF_VC2);
    EndSx1262LoraSpi();

    nrf_gpio_cfg_default(LED_1);
    nrf_gpio_cfg_default(LED_2);

    GetEs8311I2sBus()->Deinit();
    nrf_gpio_cfg_default(ES8311_ADC_DATA);
    nrf_gpio_cfg_default(ES8311_DAC_DATA);
    nrf_gpio_cfg_default(ES8311_WS_LRCK);
    nrf_gpio_cfg_default(ES8311_BCLK);
    nrf_gpio_cfg_default(ES8311_MCLK);

    audio_view::EndFlash();

    // 切断 3.3V 外设供电
    digitalWrite(RT9080_EN, LOW);
    nrf_gpio_cfg_default(RT9080_EN);
  } else {
    // 恢复 3.3V 供电
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    ConfigureBatteryMeasurement();

    Serial.begin(115200);
    pinMode(SCREEN_BS1, OUTPUT);
    digitalWrite(SCREEN_BS1, LOW);
    lvgl_port::BeginDisplay();

    InitTca8418();
    pinMode(TCA8418_INT, INPUT_PULLUP);
    InitAw21009(kAw21009MaxBrightness);
    InitAw86224();
    InitEs8311();
    audio_view::InitFlash();

    // 恢复 BLE
    InitializeBleUart();
    lvgl_port::SetBleConnected(IsBleUartConnected());

    system_sleeping = false;
    if (screen_refresh_task_handle != nullptr) {
      vTaskResume(screen_refresh_task_handle);
    }
  }
}

void setup() {
  Serial.begin(115200);
  const String build_time = home_view::GetBuildTime();
  LogPrintln(String("[T-Echo-Lite-KeyShield_") +
             home_view::GetBoardVersion() + "][" +
             home_view::GetSoftwareName() + "]_firmware_" + build_time);

  // 3.3V Power ON
  pinMode(RT9080_EN, OUTPUT);
  digitalWrite(RT9080_EN, HIGH);
  delay(100);
  digitalWrite(RT9080_EN, LOW);
  delay(1500);
  digitalWrite(RT9080_EN, HIGH);
  delay(200);

  pinMode(SCREEN_BS1, OUTPUT);
  digitalWrite(SCREEN_BS1, LOW);

  pinMode(nRF52840_BOOT, INPUT_PULLUP);
  attachInterrupt(nRF52840_BOOT, BootWakeInterruptCallback, FALLING);

  pinMode(TCA8418_INT, INPUT_PULLUP);
  ConfigureBatteryMeasurement();

  InitTca8418();
  InitAw21009(0);
  InitAw86224();
  InitEs8311();
  audio_view::InitFlash();

  lvgl_port::Init();
  lvgl_port::SetSleepMode(false);
  UpdateStatusBar();
  lvgl_port::ShowBootScreen();

  GetAw21009().SetBrightness(
      cpp_bus_driver::Aw21009::LedChannel::kAll, kAw21009MaxBrightness);

  // 初始化 BLE
  SetBleUartConnectionChangedCallback(HandleBleUartConnectionChanged);
  InitializeBleUart();
  lvgl_port::SetBleConnected(IsBleUartConnected());

  xTaskCreate(ScreenRefreshTask, "ScreenRefreshTask",
      kScreenRefreshTaskStackSize, nullptr, 3, &screen_refresh_task_handle);

  RequestScreenRefresh(false);

  ResetAutoSleepTimer();
}

void loop() {
  // 自动休眠检测（SX1262 和 GPS 页面不休眠）。
  if (current_page != UiPage::kSx1262Lora && current_page != UiPage::kGps &&
      sleep_op.current_mode == SleepOperator::Mode::kNotSleep &&
      millis() > sleep_op.wake_deadline_ms) {
    LogPrintln("Light sleep on");

    lvgl_port::SetBleConnected(false);

    lvgl_port::SetSleepMode(true);
    UpdateStatusBar();
    screen_refresh_flag = false;
    RefreshCurrentPage(true);

    boot_wake_requested = false;
    SetSystemSleep(true);
    sleep_op.current_mode = SleepOperator::Mode::kLightSleep;
  }

  // 休眠状态下通过BOOT按键唤醒
  if (sleep_op.current_mode == SleepOperator::Mode::kLightSleep) {
    while (sleep_op.current_mode == SleepOperator::Mode::kLightSleep) {
      if (boot_wake_requested || digitalRead(nRF52840_BOOT) == LOW) {
        boot_wake_requested = false;
        SetSystemSleep(false);

        LogPrintln("Awakening");

        lvgl_port::SetSleepMode(false);
        UpdateStatusBar();
        screen_refresh_flag = false;
        RefreshCurrentPage(true);

        sleep_op.current_mode = SleepOperator::Mode::kNotSleep;
        ResetAutoSleepTimer();
        break;
      }
      waitForEvent();
      delay(1000);
    }
  }

  if (ProcessSx1262Lora()) {
    RequestScreenRefresh(true);
  }

  if (ProcessGps()) {
    RequestScreenRefresh(true);
  }

  if (current_page == UiPage::kSx1262Lora && page_selected) {
    ResetAutoSleepTimer();
  }

  if (digitalRead(TCA8418_INT) == LOW) {
    auto& tca8418 = GetTca8418();

    cpp_bus_driver::Tca8418::IrqStatus is;

    if (!tca8418.ParseIrqStatus(tca8418.GetIrqFlag(), is)) {
      LogPrintf("parse_irq_status fail\n");
    } else {
      if (is.fifo_overflow_flag) {
        LogPrintf("tca8418 fifo overflow\n");
        tca8418.ClearIrqFlag(cpp_bus_driver::Tca8418::IrqFlag::kFifoOverflow);
      }

      if (is.keypad_lock_flag) {
        cpp_bus_driver::Tca8418::KeyLockInfo lock_info;
        if (tca8418.GetKeyLockInfo(&lock_info)) {
          LogPrintf("key lock interrupt, locked: %u events: %u\n",
              static_cast<unsigned int>(lock_info.locked),
              static_cast<unsigned int>(lock_info.event_count));
        }
        tca8418.ClearIrqFlag(cpp_bus_driver::Tca8418::IrqFlag::kKeypadLock);
      }

      if (is.gpio_interrupt_flag) {
        uint32_t gpio_status = 0;
        if (tca8418.GetClearGpioIrqFlag(&gpio_status)) {
          LogPrintf("gpio irq status: %#lx\n",
              static_cast<unsigned long>(gpio_status));
        }
        tca8418.ClearIrqFlag(cpp_bus_driver::Tca8418::IrqFlag::kGpioInterrupt);
      }

      if (is.key_events_flag) {
        cpp_bus_driver::Tca8418::TouchPoint tp;
        if (tca8418.GetMultipleTouchPoint(tp)) {
          // LogPrintf("touch finger: %d\n", tp.finger_count);

          for (uint8_t i = 0; i < tp.info.size(); i++) {
            switch (tp.info[i].event_type) {
              case cpp_bus_driver::Tca8418::EventType::kKeypad: {
                cpp_bus_driver::Tca8418::TouchPosition tp_2;
                if (tca8418.ParseTouchNum(tp.info[i].num, tp_2)) {
                  // LogPrintf("keypad event\n");
                  // LogPrintf(
                  //     "   touch num:[%d] num: %d x: %d y: %d "
                  //     "press_flag: %d\n",
                  //     i + 1, tp.info[i].num, tp_2.x, tp_2.y,
                  //     tp.info[i].press_flag);
                  const size_t key_index = tp.info[i].num - 1;
                  if (key_index <
                      (sizeof(Tca8418_Map) / sizeof(Tca8418_Map[0]))) {
                    // LogPrintf("   touch string: %s\n",
                    //     Tca8418_Map[key_index].c_str());

                    if (tp.info[i].press_flag) {
                      ResetAutoSleepTimer();

                      const std::string& key_text = Tca8418_Map[key_index];
                      if (key_text == "Home") {
                        SelectUiPage(UiPage::kHome);
                        page_selected = false;
                        home_scroll_index = 0;
                        battery_scroll_index = 0;
                        RequestScreenRefresh(false);
                        StartVibration();
                        break;
                      }

                      if (!page_selected) {
                        if (key_text == "Down") {
                          SelectUiPage(GetNextUiPage(current_page));
                        } else if (key_text == "Up") {
                          SelectUiPage(GetPreviousUiPage(current_page));
                        } else if (key_text == "Center") {
                          if (current_page == UiPage::kSx1262Lora) {
                            sx1262_lora_control = Sx1262LoraControl::kFrequency;
                            sx1262_lora_frequency_editing = false;
                            sx1262_lora_bandwidth_editing = false;
                            sx1262_lora_init_attempted = true;
                            InitializeSx1262Lora();
                            page_selected = true;
                          } else if (current_page == UiPage::kImu) {
                            ReadImuSensor();
                            page_selected = true;
                          } else if (current_page == UiPage::kBatteryInfo) {
                            UpdateStatusBar();
                            RefreshBatteryInfoSnapshot();
                            battery_scroll_index = 0;
                            page_selected = true;
                          } else {
                            page_selected = true;
                          }
                        } else {
                          break;
                        }
                        RequestScreenRefresh(false);
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kHome) {
                        if (key_text == "Down") {
                          const size_t max_scroll_index =
                              home_view::GetMaxScrollIndex();
                          home_scroll_index =
                              std::min(home_scroll_index + kHomeScrollStep,
                                  max_scroll_index);
                        } else if (key_text == "Up") {
                          home_scroll_index =
                              home_scroll_index > kHomeScrollStep
                                  ? home_scroll_index - kHomeScrollStep
                                  : 0;
                        } else if (key_text == "Esc") {
                          page_selected = false;
                        } else {
                          break;
                        }
                        RequestScreenRefresh(true);
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kBatteryInfo) {
                        if (key_text == "Down") {
                          const size_t max_scroll_index =
                              battery_view::GetMaxScrollIndex(
                                  battery_info_snapshot);
                          battery_scroll_index =
                              std::min(battery_scroll_index + kHomeScrollStep,
                                  max_scroll_index);
                        } else if (key_text == "Up") {
                          battery_scroll_index =
                              battery_scroll_index > kHomeScrollStep
                                  ? battery_scroll_index - kHomeScrollStep
                                  : 0;
                        } else if (key_text == "Esc") {
                          page_selected = false;
                        } else if (key_text == "Center") {
                          UpdateStatusBar();
                          RefreshBatteryInfoSnapshot();
                          battery_scroll_index = 0;
                        } else {
                          break;
                        }
                        RequestScreenRefresh(true);
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kAudioTest) {
                        // 追踪 Mic/Speaker 选择用于渲染
                        if (key_text == "Down" || key_text == "Up") {
                          audio_target = audio_target == AudioTarget::kMic
                                             ? AudioTarget::kSpeaker
                                             : AudioTarget::kMic;
                        }
                        const audio_view::KeyResult audio_key_result =
                            audio_view::HandleKey(
                                key_text, GetEs8311(), StartCompletionVibration);
                        if (audio_key_result.handled) {
                          if (key_text == "Esc") {
                            page_selected = false;
                          }
                          RequestScreenRefresh(key_text != "Center");
                          if (audio_key_result.use_key_vibration) {
                            StartVibration();
                          }
                          break;
                        }
                      }

                      if (current_page == UiPage::kBluetooth) {
                        if (key_text == "Down") {
                          const size_t max_scroll_index =
                              GetBluetoothMaxScrollIndex();
                          bluetooth_scroll_index =
                              std::min(bluetooth_scroll_index + kHomeScrollStep,
                                  max_scroll_index);
                        } else if (key_text == "Up") {
                          bluetooth_scroll_index =
                              bluetooth_scroll_index > kHomeScrollStep
                                  ? bluetooth_scroll_index - kHomeScrollStep
                                  : 0;
                        } else if (key_text == "Center") {
                          // Center 刷新当前 Bluetooth 信息
                        } else if (key_text == "Esc") {
                          page_selected = false;
                        } else {
                          break;
                        }
                        RequestScreenRefresh(true);
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kSx1262Lora) {
                        if (!HandleSx1262LoraPageKey(key_text)) {
                          break;
                        }
                        RequestScreenRefresh(true);
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kGps) {
                        if (key_text == "Esc") {
                          page_selected = false;
                        } else {
                          break;
                        }
                        RequestScreenRefresh(true);
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kImu) {
                        if (key_text == "Center") {
                          ReadImuSensor();
                        } else if (key_text == "Esc") {
                          page_selected = false;
                        } else {
                          break;
                        }
                        RequestScreenRefresh(true);
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kKeyboardTest &&
                          key_text == "Esc") {
                        page_selected = false;
                        RequestScreenRefresh(true);
                        StartVibration();
                        break;
                      }

                      keyboard_view::AddText(key_text);
                      StartVibration();
                      RequestKeyboardTextRefresh();
                    }
                  }
                }

                break;
              }
              case cpp_bus_driver::Tca8418::EventType::kGpio:
                LogPrintf("gpio event\n");
                LogPrintf("   touch num:[%d] num: %d press_flag: %d\n", i + 1,
                    tp.info[i].num, tp.info[i].press_flag);
                break;

              default:
                break;
            }
          }
        }

        tca8418.ClearIrqFlag(cpp_bus_driver::Tca8418::IrqFlag::kKeyEvents);
      }

      if (is.ctrl_alt_del_key_sequence_flag) {
        LogPrintf("ctrl-alt-del sequence interrupt\n");
        tca8418.ClearIrqFlag(
            cpp_bus_driver::Tca8418::IrqFlag::kCtrlAltDelKeySequence);
      }
    }
  }

  vTaskDelay(pdMS_TO_TICKS(10));
}
