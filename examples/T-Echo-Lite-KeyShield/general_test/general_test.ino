/*
 * @Description: original_test
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:45:24
 * @License: GPL 3.0
 */
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <Wire.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "audio_view.h"
#include "battery_view.h"
#include "cpp_bus_driver_library.h"
#include "home_view.h"
#include "keyboard_view.h"
#include "lvgl.h"
#include "lvgl_port.h"
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
};

bool screen_refresh_flag = false;
UiPage current_page = UiPage::kHome;
bool page_selected = false;
size_t home_scroll_index = 0;
size_t battery_scroll_index = 0;
uint8_t filtered_battery_percentage = 0;
battery_view::BatteryInfo battery_info_snapshot;

TaskHandle_t screen_refresh_task_handle = nullptr;

/**
 * @brief 获取 UI 页面名称。
 * @param page UI 页面枚举。
 * @return 页面名称字符串。
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
    default:
      return "Unknown";
  }
}

/**
 * @brief 获取下一个 UI 页面。
 * @param page 当前 UI 页面。
 * @return 下一个 UI 页面。
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
      return UiPage::kHome;
    default:
      return UiPage::kHome;
  }
}

/**
 * @brief 获取上一个 UI 页面。
 * @param page 当前 UI 页面。
 * @return 上一个 UI 页面。
 */
UiPage GetPreviousUiPage(UiPage page) {
  switch (page) {
    case UiPage::kHome:
      return UiPage::kBatteryInfo;
    case UiPage::kKeyboardTest:
      return UiPage::kHome;
    case UiPage::kAudioTest:
      return UiPage::kKeyboardTest;
    case UiPage::kBatteryInfo:
      return UiPage::kAudioTest;
    default:
      return UiPage::kHome;
  }
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
 * @param page 目标 UI 页面。
 */
void SelectUiPage(UiPage page) {
  if (current_page == UiPage::kAudioTest && page != UiPage::kAudioTest) {
    audio_view::Stop(GetEs8311());
  }
  if (current_page != page && page == UiPage::kBatteryInfo) {
    battery_scroll_index = 0;
  }
  current_page = page;
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
 * @brief 更新状态栏电池显示信息。
 */
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
 * @brief 刷新当前 UI 页面。
 * @param busy_enable 是否显示忙碌状态。
 */
void RefreshCurrentPage(bool busy_enable) {
  if (current_page == UiPage::kHome) {
    const std::vector<std::string> home_lines = home_view::CreateLines();
    lvgl_port::ShowHomeScreen(home_lines, home_scroll_index,
        GetUiPageName(current_page), page_selected, busy_enable);
  } else if (current_page == UiPage::kAudioTest) {
    audio_view::Show(page_selected, GetUiPageName(current_page), busy_enable);
  } else if (current_page == UiPage::kBatteryInfo) {
    const std::vector<std::string> battery_lines =
        battery_view::CreateLines(battery_info_snapshot);
    lvgl_port::ShowHomeScreen(battery_lines, battery_scroll_index,
        GetUiPageName(current_page), page_selected, busy_enable);
  } else {
    lvgl_port::ShowTextList(keyboard_view::GetTextList(),
        GetUiPageName(current_page), page_selected, true, busy_enable);
  }
}

/**
 * @brief 屏幕刷新任务入口。
 * @param arg 任务参数。
 */
void ScreenRefreshTask(void* arg) {
  (void)arg;
  printf("ScreenRefreshTask start\n");

  while (true) {
    if (screen_refresh_flag) {
      screen_refresh_flag = false;
      UpdateStatusBar();
      RefreshCurrentPage(false);
    }

    lvgl_port::Tick(kScreenRefreshTaskPeriodMs);
    delay(kScreenRefreshTaskPeriodMs);
  }
}

/**
 * @brief 设置系统休眠或唤醒状态。
 * @param enable true 进入休眠，false 退出休眠。
 */
void SetSystemSleep(bool enable) {
  if (enable) {
    auto& es8311_i2s_bus = GetEs8311I2sBus();

    Serial.end();
    lvgl_port::EndDisplay();
    pinMode(SCREEN_BS1, INPUT);

    Wire.end();

    pinMode(IIC_1_SDA, INPUT);
    pinMode(IIC_1_SCL, INPUT);

    pinMode(BATTERY_MEASUREMENT_CONTROL, INPUT);

    audio_view::Stop(GetEs8311());
    es8311_i2s_bus->Deinit();
    audio_view::EndFlash();

    digitalWrite(RT9080_EN, LOW);
    pinMode(RT9080_EN, INPUT_PULLDOWN);

    vTaskSuspend(screen_refresh_task_handle);
  } else {
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    ConfigureBatteryMeasurement();

    Serial.begin(115200);
    pinMode(SCREEN_BS1, OUTPUT);
    digitalWrite(SCREEN_BS1, LOW);
    lvgl_port::BeginDisplay();

    InitTca8418();
    InitAw21009(kAw21009MaxBrightness);
    InitAw86224();
    InitEs8311();
    audio_view::InitFlash();

    vTaskResume(screen_refresh_task_handle);
  }
}

void setup() {
  Serial.begin(115200);
  const String build_time = home_view::GetBuildTime();
  Serial.println(String("[T-Echo-Lite-KeyShield_") +
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

  xTaskCreate(ScreenRefreshTask, "ScreenRefreshTask",
      kScreenRefreshTaskStackSize, nullptr, 3, &screen_refresh_task_handle);

  screen_refresh_flag = true;

  ResetAutoSleepTimer();
}

void loop() {
  // 自动进入休眠检测。
  if (sleep_op.current_mode == SleepOperator::Mode::kNotSleep &&
      millis() > sleep_op.wake_deadline_ms) {
    Serial.println("Light sleep on");

    // 显示休眠提示。
    lvgl_port::SetSleepMode(true);
    UpdateStatusBar();
    screen_refresh_flag = false;
    RefreshCurrentPage(true);

    boot_wake_requested = false;
    SetSystemSleep(true);
    sleep_op.current_mode = SleepOperator::Mode::kLightSleep;
  }

  // 休眠状态下通过BOOT按键中断唤醒，短按也可以触发。
  if (sleep_op.current_mode == SleepOperator::Mode::kLightSleep) {
    if (boot_wake_requested || digitalRead(nRF52840_BOOT) == LOW) {
      boot_wake_requested = false;
      SetSystemSleep(false);

      Serial.println("Awakening");

      lvgl_port::SetSleepMode(false);
      UpdateStatusBar();
      screen_refresh_flag = false;
      RefreshCurrentPage(true);

      sleep_op.current_mode = SleepOperator::Mode::kNotSleep;
      // 重置自动休眠计时。
      ResetAutoSleepTimer();
    } else {
      waitForEvent();
      return;
    }
  }

  if (digitalRead(TCA8418_INT) == LOW) {
    auto& tca8418 = GetTca8418();

    cpp_bus_driver::Tca8418::IrqStatus is;

    if (!tca8418.ParseIrqStatus(tca8418.GetIrqFlag(), is)) {
      printf("parse_irq_status fail\n");
    } else {
      if (is.fifo_overflow_flag) {
        printf("tca8418 fifo overflow\n");
        tca8418.ClearIrqFlag(cpp_bus_driver::Tca8418::IrqFlag::kFifoOverflow);
      }

      if (is.keypad_lock_flag) {
        cpp_bus_driver::Tca8418::KeyLockInfo lock_info;
        if (tca8418.GetKeyLockInfo(&lock_info)) {
          printf("key lock interrupt, locked: %u events: %u\n",
              static_cast<unsigned int>(lock_info.locked),
              static_cast<unsigned int>(lock_info.event_count));
        }
        tca8418.ClearIrqFlag(cpp_bus_driver::Tca8418::IrqFlag::kKeypadLock);
      }

      if (is.gpio_interrupt_flag) {
        uint32_t gpio_status = 0;
        if (tca8418.GetClearGpioIrqFlag(&gpio_status)) {
          printf("gpio irq status: %#lx\n",
              static_cast<unsigned long>(gpio_status));
        }
        tca8418.ClearIrqFlag(cpp_bus_driver::Tca8418::IrqFlag::kGpioInterrupt);
      }

      if (is.key_events_flag) {
        cpp_bus_driver::Tca8418::TouchPoint tp;
        if (tca8418.GetMultipleTouchPoint(tp)) {
          printf("touch finger: %d\n", tp.finger_count);

          for (uint8_t i = 0; i < tp.info.size(); i++) {
            switch (tp.info[i].event_type) {
              case cpp_bus_driver::Tca8418::EventType::kKeypad: {
                cpp_bus_driver::Tca8418::TouchPosition tp_2;
                if (tca8418.ParseTouchNum(tp.info[i].num, tp_2)) {
                  printf("keypad event\n");
                  printf(
                      "   touch num:[%d] num: %d x: %d y: %d "
                      "press_flag: %d\n",
                      i + 1, tp.info[i].num, tp_2.x, tp_2.y,
                      tp.info[i].press_flag);
                  const size_t key_index = tp.info[i].num - 1;
                  if (key_index <
                      (sizeof(Tca8418_Map) / sizeof(Tca8418_Map[0]))) {
                    printf("   touch string: %s\n",
                        Tca8418_Map[key_index].c_str());

                    if (tp.info[i].press_flag) {
                      // 重置自动休眠计时。
                      ResetAutoSleepTimer();

                      const std::string& key_text = Tca8418_Map[key_index];
                      if (key_text == "Home") {
                        SelectUiPage(UiPage::kHome);
                        page_selected = false;
                        home_scroll_index = 0;
                        battery_scroll_index = 0;
                        screen_refresh_flag = true;
                        StartVibration();
                        break;
                      }

                      if (!page_selected) {
                        if (key_text == "Down") {
                          SelectUiPage(GetNextUiPage(current_page));
                        } else if (key_text == "Up") {
                          SelectUiPage(GetPreviousUiPage(current_page));
                        } else if (key_text == "Center") {
                          if (current_page == UiPage::kBatteryInfo) {
                            UpdateStatusBar();
                            RefreshBatteryInfoSnapshot();
                            battery_scroll_index = 0;
                          }
                          page_selected = true;
                        } else {
                          break;
                        }
                        screen_refresh_flag = true;
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
                        screen_refresh_flag = true;
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
                        screen_refresh_flag = true;
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kAudioTest) {
                        const audio_view::KeyResult audio_key_result =
                            audio_view::HandleKey(
                                key_text, GetEs8311(), StartCompletionVibration);
                        if (audio_key_result.handled) {
                          if (key_text == "Esc") {
                            page_selected = false;
                          }
                          screen_refresh_flag = true;
                          if (audio_key_result.use_key_vibration) {
                            StartVibration();
                          }
                          break;
                        }
                      }

                      if (current_page == UiPage::kKeyboardTest &&
                          key_text == "Esc") {
                        page_selected = false;
                        screen_refresh_flag = true;
                        StartVibration();
                        break;
                      }

                      keyboard_view::AddText(key_text);
                      StartVibration();

                      screen_refresh_flag = true;
                    }
                  }
                }

                break;
              }
              case cpp_bus_driver::Tca8418::EventType::kGpio:
                printf("gpio event\n");
                printf("   touch num:[%d] num: %d press_flag: %d\n", i + 1,
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
        printf("ctrl-alt-del sequence interrupt\n");
        tca8418.ClearIrqFlag(
            cpp_bus_driver::Tca8418::IrqFlag::kCtrlAltDelKeySequence);
      }
    }
  }

  delay(10);
}
