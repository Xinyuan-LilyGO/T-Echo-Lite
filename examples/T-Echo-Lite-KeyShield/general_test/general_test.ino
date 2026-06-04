/*
 * @Description: original_test
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-04 17:25:31
 * @License: GPL 3.0
 */
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <algorithm>

#include "Adafruit_SPIFlash.h"
#include "cpp_bus_driver_library.h"
#include "lvgl.h"
#include "lvgl_port.h"
#include "t_echo_lite_keyshield_config.h"

static constexpr char kDisplaySoftwareName[] = "general_test";
static constexpr char kBoardVersion[] = "v1.0";
static constexpr uint8_t kAudioMclkMultiple = 32;
static constexpr uint32_t kAudioSampleRate = 44100;
static constexpr uint8_t kAudioBitsPerSample = 16;
static constexpr size_t kAudioBufferSampleCount = 1024;
static constexpr size_t kAudioBufferBytes =
    kAudioBufferSampleCount * sizeof(uint32_t);
static constexpr uint32_t kAudioRecordSeconds = 3;
static constexpr uint32_t kAudioRecordDataAddress = 4096;
static constexpr uint32_t kAudioRecordBytes =
    kAudioSampleRate * kAudioRecordSeconds * sizeof(uint32_t);
static constexpr uint32_t kFlashSectorSize = 4096;
static constexpr uint32_t kAudioFlashMagic = 0x41554430;
static constexpr size_t kMaxCurrentTextCount = 8;
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
};

enum class AudioTarget : uint8_t {
  kMic,
  kSpeaker,
};

struct AudioRecordHeader {
  uint32_t magic = 0;
  uint32_t data_length = 0;
};

std::vector<std::string> current_text;

bool screen_refresh_flag = false;
UiPage current_page = UiPage::kHome;
AudioTarget audio_target = AudioTarget::kMic;
std::string audio_status_text = "Select Mic or Speaker";
bool page_selected = false;
size_t home_scroll_index = 0;
uint32_t audio_recorded_length = 0;
bool audio_record_available = false;
uint32_t audio_buffer[2][kAudioBufferSampleCount] = {0};
bool flash_ready = false;
bool partial_refresh_flag = true;
size_t fast_refresh_count = 0;

TaskHandle_t screen_refresh_task_handle = nullptr;

SPIClass custom_spi_flash(
    NRF_SPIM3, ZD25WQ32C_MISO, ZD25WQ32C_SCLK, ZD25WQ32C_MOSI);
Adafruit_FlashTransport_SPI flash_transport(ZD25WQ32C_CS, custom_spi_flash);
Adafruit_SPIFlash flash(&flash_transport);
SPIFlash_Device_t zd25wq32c = {
    total_size : (1UL << 22),
    start_up_time_us : 12000,
    manufacturer_id : 0xBA,
    memory_type : 0x60,
    capacity : 0x16,
    max_clock_speed_mhz : 104,
    quad_enable_bit_mask : 0x02,
    has_sector_protection : false,
    supports_fast_read : true,
    supports_qspi : true,
    supports_qspi_writes : true,
    write_status_register_split : false,
    single_status_byte : false,
    is_fram : false,
};

uint8_t DigitOrZero(char value) {
  return value >= '0' && value <= '9' ? value - '0' : 0;
}

uint8_t ParseBuildMonth(const char* date) {
  if (date[0] == 'J' && date[1] == 'a') {
    return 1;
  }
  if (date[0] == 'F') {
    return 2;
  }
  if (date[0] == 'M' && date[2] == 'r') {
    return 3;
  }
  if (date[0] == 'A' && date[1] == 'p') {
    return 4;
  }
  if (date[0] == 'M' && date[2] == 'y') {
    return 5;
  }
  if (date[0] == 'J' && date[2] == 'n') {
    return 6;
  }
  if (date[0] == 'J' && date[2] == 'l') {
    return 7;
  }
  if (date[0] == 'A' && date[1] == 'u') {
    return 8;
  }
  if (date[0] == 'S') {
    return 9;
  }
  if (date[0] == 'O') {
    return 10;
  }
  if (date[0] == 'N') {
    return 11;
  }
  if (date[0] == 'D') {
    return 12;
  }
  return 0;
}

String GetSoftwareBuildTime() {
  static constexpr char kBuildDate[] = __DATE__;
  static constexpr char kBuildTime[] = __TIME__;
  char build_time[13] = {0};
  const uint8_t month = ParseBuildMonth(kBuildDate);
  const uint8_t day =
      DigitOrZero(kBuildDate[4]) * 10 + DigitOrZero(kBuildDate[5]);

  snprintf(build_time, sizeof(build_time), "%c%c%c%c%02u%02u%c%c%c%c",
      kBuildDate[7], kBuildDate[8], kBuildDate[9], kBuildDate[10],
      static_cast<unsigned int>(month), static_cast<unsigned int>(day),
      kBuildTime[0], kBuildTime[1], kBuildTime[3], kBuildTime[4]);

  return String(build_time);
}

const char* GetMcuModelName() {
#if defined(NRF52840_XXAA)
  return "nRF52840";
#elif defined(NRF52833_XXAA)
  return "nRF52833";
#else
  return "nRF52";
#endif
}

uint32_t GetFlashSizeKb() {
#if defined(NRF52840_XXAA)
  return 1024;
#else
  return 0;
#endif
}

uint32_t GetRamSizeKb() {
#if defined(NRF52840_XXAA)
  return 256;
#else
  return 0;
#endif
}

std::string GetDeviceIdText() {
  char text[32] = {};
  snprintf(text, sizeof(text), "%08lX-%08lX",
      static_cast<unsigned long>(NRF_FICR->DEVICEID[0]),
      static_cast<unsigned long>(NRF_FICR->DEVICEID[1]));
  return text;
}

std::vector<std::string> CreateHomeScreenLines() {
  const String build_time = GetSoftwareBuildTime();
  const std::string device_id = GetDeviceIdText();
  std::vector<std::string> lines;
  char line[64] = {};
  snprintf(line, sizeof(line), "T-Echo-Lite KeyShield  %s", kBoardVersion);
  lines.push_back(line);
  lines.push_back("");
  lines.push_back("[Chip]");
  snprintf(line, sizeof(line), "model: %s", GetMcuModelName());
  lines.push_back(line);
  snprintf(line, sizeof(line), "clock: %luMHz",
      static_cast<unsigned long>(SystemCoreClock / 1000000));
  lines.push_back(line);
  snprintf(line, sizeof(line), "id: %s", device_id.c_str());
  lines.push_back(line);
  lines.push_back("");
  lines.push_back("[Memory]");
  snprintf(line, sizeof(line), "flash / ram: %lu / %luKB",
      static_cast<unsigned long>(GetFlashSizeKb()),
      static_cast<unsigned long>(GetRamSizeKb()));
  lines.push_back(line);
  lines.push_back("");
  lines.push_back("[Software]");
  snprintf(line, sizeof(line), "name: %s", kDisplaySoftwareName);
  lines.push_back(line);
  snprintf(line, sizeof(line), "build: %s", build_time.c_str());
  lines.push_back(line);
  lines.push_back("");
  lines.push_back("[Screen]");
  lines.push_back("type: SSD1681 EPD");
  snprintf(
      line, sizeof(line), "size: %d x %dpx", SCREEN_HEIGHT, SCREEN_WIDTH);
  lines.push_back(line);
  lines.push_back("");
  lines.push_back("[LVGL]");
  snprintf(line, sizeof(line), "version: v%d.%d.%d", LVGL_VERSION_MAJOR,
      LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
  lines.push_back(line);
  return lines;
}

const char* GetUiPageName(UiPage page) {
  switch (page) {
    case UiPage::kHome:
      return "Home";
    case UiPage::kKeyboardTest:
      return "Keyboard";
    case UiPage::kAudioTest:
      return "Audio";
    default:
      return "Unknown";
  }
}

UiPage GetNextUiPage(UiPage page) {
  switch (page) {
    case UiPage::kHome:
      return UiPage::kKeyboardTest;
    case UiPage::kKeyboardTest:
      return UiPage::kAudioTest;
    case UiPage::kAudioTest:
      return UiPage::kHome;
    default:
      return UiPage::kHome;
  }
}

UiPage GetPreviousUiPage(UiPage page) {
  switch (page) {
    case UiPage::kHome:
      return UiPage::kAudioTest;
    case UiPage::kKeyboardTest:
      return UiPage::kHome;
    case UiPage::kAudioTest:
      return UiPage::kKeyboardTest;
    default:
      return UiPage::kHome;
  }
}

size_t GetHomeMaxScrollIndex() {
  const size_t line_count = CreateHomeScreenLines().size();
  return line_count > kHomeVisibleLineCount ? line_count - kHomeVisibleLineCount
                                            : 0;
}

std::shared_ptr<cpp_bus_driver::HardwareI2c2>& GetTca8418I2cBus() {
  static auto tca8418_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c2>(
      TCA8418_SDA, TCA8418_SCL, &Wire);
  return tca8418_i2c_bus;
}

std::shared_ptr<cpp_bus_driver::HardwareI2c2>& GetAw21009I2cBus() {
  static auto aw21009_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c2>(
      AW21009_SDA, AW21009_SCL, &Wire);
  return aw21009_i2c_bus;
}

std::shared_ptr<cpp_bus_driver::HardwareI2c2>& GetEs8311I2cBus() {
  static auto es8311_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c2>(
      ES8311_SDA, ES8311_SCL, &Wire);
  return es8311_i2c_bus;
}

std::shared_ptr<cpp_bus_driver::HardwareI2c2>& GetAw86224I2cBus() {
  static auto aw86224_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c2>(
      AW86224_SDA, AW86224_SCL, &Wire);
  return aw86224_i2c_bus;
}

std::shared_ptr<cpp_bus_driver::HardwareI2s>& GetEs8311I2sBus() {
  static auto es8311_i2s_bus =
      std::make_shared<cpp_bus_driver::HardwareI2s>(ES8311_ADC_DATA,
          ES8311_DAC_DATA, ES8311_WS_LRCK, ES8311_BCLK, ES8311_MCLK);
  return es8311_i2s_bus;
}

cpp_bus_driver::Tca8418& GetTca8418() {
  static auto tca8418 = std::make_unique<cpp_bus_driver::Tca8418>(
      GetTca8418I2cBus(), TCA8418_IIC_ADDRESS);
  return *tca8418;
}

cpp_bus_driver::Aw21009& GetAw21009() {
  static auto aw21009 = std::make_unique<cpp_bus_driver::Aw21009>(
      GetAw21009I2cBus(), AW21009_IIC_ADDRESS);
  return *aw21009;
}

cpp_bus_driver::Aw862xx& GetAw86224() {
  static auto aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
      GetAw86224I2cBus(), AW86224_IIC_ADDRESS);
  return *aw86224;
}

cpp_bus_driver::Es8311& GetEs8311() {
  static auto es8311 = std::make_unique<cpp_bus_driver::Es8311>(
      GetEs8311I2cBus(), GetEs8311I2sBus(), ES8311_IIC_ADDRESS);
  return *es8311;
}

SleepOperator sleep_op;
volatile bool boot_wake_requested = false;

void BootWakeInterruptCallback() { boot_wake_requested = true; }

void StartVibration() {
  auto& aw86224 = GetAw86224();
  if (!aw86224.PlayRamWaveform(
          kVibrationSequence, kVibrationLoopCount, kVibrationGain, true)) {
    printf("StartVibration failed\n");
  }
}

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

void ShowAudioStatus(const char* status) {
  audio_status_text = status == nullptr ? "" : status;
  lvgl_port::ShowAudioScreen(page_selected,
      audio_target == AudioTarget::kMic, audio_status_text.c_str(),
      GetUiPageName(current_page), false);
}

bool InitFlash() {
  custom_spi_flash.setClockDivider(SPI_CLOCK_DIV2);
  if (!flash.begin(&zd25wq32c)) {
    printf("flash init failed\n");
    flash_ready = false;
    return false;
  }
  flash_ready = true;

  AudioRecordHeader header;
  if (flash.readBuffer(0, reinterpret_cast<uint8_t*>(&header),
          sizeof(header)) == sizeof(header) &&
      header.magic == kAudioFlashMagic &&
      header.data_length > 0 && header.data_length <= kAudioRecordBytes) {
    audio_recorded_length = header.data_length;
    audio_record_available = true;
  }
  return true;
}

void StopAudio() {
  GetEs8311().StopTransmitI2s();
}

bool EraseAudioFlashArea() {
  const uint32_t erase_end = kAudioRecordDataAddress + kAudioRecordBytes;
  for (uint32_t address = 0; address < erase_end;
       address += kFlashSectorSize) {
    if (!flash.eraseSector(address / kFlashSectorSize)) {
      printf("flash erase sector failed: 0x%08lX\n",
          static_cast<unsigned long>(address));
      return false;
    }
    flash.waitUntilReady();
  }
  return true;
}

bool WaitForAudioReadEvent(uint32_t timeout_ms) {
  auto& es8311 = GetEs8311();
  const uint32_t deadline = millis() + timeout_ms;
  while (millis() < deadline) {
    if (es8311.GetReadI2sEventFlag()) {
      return true;
    }
    delay(1);
  }
  return false;
}

bool WaitForAudioWriteEvent(uint32_t timeout_ms) {
  auto& es8311 = GetEs8311();
  const uint32_t deadline = millis() + timeout_ms;
  while (millis() < deadline) {
    if (es8311.GetWriteI2sEventFlag()) {
      return true;
    }
    delay(1);
  }
  return false;
}

bool RecordAudioToFlash() {
  auto& es8311 = GetEs8311();

  if (!flash_ready) {
    ShowAudioStatus("Flash not ready");
    return false;
  }

  ShowAudioStatus("Recording 3s...");
  StopAudio();
  if (!EraseAudioFlashArea()) {
    ShowAudioStatus("Flash erase failed");
    return false;
  }

  uint8_t filling_buffer = 0;
  uint8_t free_buffer = 1;
  uint32_t written_bytes = 0;
  const uint32_t record_deadline = millis() + kAudioRecordSeconds * 1000;

  if (!es8311.StartTransmitI2s(
          nullptr, audio_buffer[filling_buffer], kAudioBufferSampleCount)) {
    ShowAudioStatus("Record start failed");
    return false;
  }

  while (millis() < record_deadline && written_bytes < kAudioRecordBytes) {
    if (!WaitForAudioReadEvent(200)) {
      continue;
    }

    const uint8_t ready_buffer = filling_buffer;
    filling_buffer = free_buffer;
    free_buffer = ready_buffer;
    es8311.SetNextReadI2s(audio_buffer[filling_buffer]);

    const uint32_t write_bytes =
        std::min<uint32_t>(kAudioBufferBytes, kAudioRecordBytes - written_bytes);
    if (flash.writeBuffer(kAudioRecordDataAddress + written_bytes,
            reinterpret_cast<uint8_t*>(audio_buffer[ready_buffer]),
            write_bytes) != write_bytes) {
      StopAudio();
      ShowAudioStatus("Flash write failed");
      return false;
    }
    flash.waitUntilReady();
    written_bytes += write_bytes;
  }

  StopAudio();

  AudioRecordHeader header;
  header.magic = kAudioFlashMagic;
  header.data_length = written_bytes;
  if (flash.writeBuffer(0, reinterpret_cast<uint8_t*>(&header),
          sizeof(header)) != sizeof(header)) {
    ShowAudioStatus("Header write failed");
    return false;
  }
  flash.waitUntilReady();

  audio_recorded_length = written_bytes;
  audio_record_available = written_bytes > 0;
  ShowAudioStatus(audio_record_available ? "Record complete" : "No audio");
  if (audio_record_available) {
    StartCompletionVibration();
  }
  return audio_record_available;
}

bool LoadAudioChunk(uint32_t offset, uint8_t buffer_index, uint32_t* length) {
  if (length == nullptr || offset >= audio_recorded_length) {
    return false;
  }

  *length = std::min<uint32_t>(
      kAudioBufferBytes, audio_recorded_length - offset);
  memset(audio_buffer[buffer_index], 0, kAudioBufferBytes);
  return flash.readBuffer(kAudioRecordDataAddress + offset,
             reinterpret_cast<uint8_t*>(audio_buffer[buffer_index]),
             *length) == *length;
}

bool PlayAudioFromFlash() {
  auto& es8311 = GetEs8311();

  if (!flash_ready) {
    ShowAudioStatus("Flash not ready");
    return false;
  }

  if (!audio_record_available || audio_recorded_length == 0) {
    ShowAudioStatus("No audio");
    return false;
  }

  ShowAudioStatus("Playing...");
  StopAudio();

  uint32_t loaded_bytes = 0;
  uint32_t played_bytes = 0;
  uint32_t buffer_length[2] = {0};
  bool buffer_ready[2] = {false};
  if (!LoadAudioChunk(0, 0, &buffer_length[0])) {
    ShowAudioStatus("Audio read failed");
    return false;
  }
  loaded_bytes += buffer_length[0];
  buffer_ready[0] = true;

  if (!es8311.StartTransmitI2s(audio_buffer[0], nullptr,
          kAudioBufferSampleCount)) {
    ShowAudioStatus("Play start failed");
    return false;
  }

  buffer_ready[0] = false;
  uint8_t current_buffer = 1;
  uint32_t active_buffer_length = buffer_length[0];

  while (played_bytes < audio_recorded_length) {
    if (!buffer_ready[current_buffer] && loaded_bytes < audio_recorded_length) {
      if (!LoadAudioChunk(
              loaded_bytes, current_buffer, &buffer_length[current_buffer])) {
        StopAudio();
        ShowAudioStatus("Audio read failed");
        return false;
      }
      loaded_bytes += buffer_length[current_buffer];
      buffer_ready[current_buffer] = true;
    }

    const uint8_t next_buffer = current_buffer == 0 ? 1 : 0;
    if (!buffer_ready[next_buffer] && loaded_bytes < audio_recorded_length) {
      if (!LoadAudioChunk(
              loaded_bytes, next_buffer, &buffer_length[next_buffer])) {
        StopAudio();
        ShowAudioStatus("Audio read failed");
        return false;
      }
      loaded_bytes += buffer_length[next_buffer];
      buffer_ready[next_buffer] = true;
    }

    if (!WaitForAudioWriteEvent(500)) {
      continue;
    }

    played_bytes += active_buffer_length;
    if (buffer_ready[current_buffer]) {
      es8311.SetNextWriteI2s(audio_buffer[current_buffer]);
      active_buffer_length = buffer_length[current_buffer];
      buffer_ready[current_buffer] = false;
      current_buffer = current_buffer == 0 ? 1 : 0;
    } else if (loaded_bytes >= audio_recorded_length) {
      break;
    }
  }

  StopAudio();
  ShowAudioStatus("Play complete");
  StartCompletionVibration();
  return true;
}

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

void AddCurrentText(const std::string& text) {
  if (current_text.size() >= kMaxCurrentTextCount) {
    current_text.erase(current_text.begin());
  }
  current_text.push_back(text);
}

void SelectUiPage(UiPage page) {
  if (current_page == UiPage::kAudioTest && page != UiPage::kAudioTest) {
    StopAudio();
  }
  current_page = page;
}

void ResetAutoSleepTimer() {
  sleep_op.wake_deadline_ms = millis() + kAutoSleepTimeoutMs;
}

void ConfigureBatteryMeasurement() {
  pinMode(BATTERY_ADC_DATA, INPUT);
  pinMode(BATTERY_MEASUREMENT_CONTROL, OUTPUT);
  digitalWrite(BATTERY_MEASUREMENT_CONTROL, HIGH);

  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
}

float ReadBatteryVoltage() {
  uint32_t adc_sum = 0;
  for (uint8_t i = 0; i < kBatteryAdcSampleCount; i++) {
    adc_sum += analogRead(BATTERY_ADC_DATA);
  }

  const float adc_average =
      static_cast<float>(adc_sum) / kBatteryAdcSampleCount;
  return ((adc_average * (kAdcReferenceMv / kAdcResolutionCount)) / 1000.0f) *
         kBatteryDividerRatio;
}

uint8_t ReadBatteryPercentage() {
  const float voltage = ReadBatteryVoltage();
  if (voltage <= kBatteryEmptyVoltage) {
    return 0;
  }
  if (voltage >= kBatteryFullVoltage) {
    return 100;
  }

  return static_cast<uint8_t>(((voltage - kBatteryEmptyVoltage) * 100.0f) /
                              (kBatteryFullVoltage - kBatteryEmptyVoltage));
}

void UpdateStatusBar() {
  static bool initialized = false;
  static uint8_t filtered_percentage = 0;

  const uint8_t current_percentage = ReadBatteryPercentage();
  if (!initialized) {
    filtered_percentage = current_percentage;
    initialized = true;
  } else {
    const int16_t delta =
        static_cast<int16_t>(current_percentage) - filtered_percentage;
    if (delta >= 2 || delta <= -2) {
      filtered_percentage =
          static_cast<uint8_t>((static_cast<uint16_t>(filtered_percentage) * 3 +
                                   current_percentage) /
                               4);
    }
  }

  lvgl_port::SetBatteryPercentage(filtered_percentage);
}

void RefreshCurrentPage(bool partial_refresh, bool busy_enable) {
  if (current_page == UiPage::kHome) {
    const std::vector<std::string> home_lines = CreateHomeScreenLines();
    lvgl_port::ShowHomeScreen(home_lines, home_scroll_index,
        GetUiPageName(current_page), page_selected, busy_enable);
  } else if (current_page == UiPage::kAudioTest) {
    lvgl_port::ShowAudioScreen(page_selected,
        audio_target == AudioTarget::kMic, audio_status_text.c_str(),
        GetUiPageName(current_page), busy_enable);
  } else {
    lvgl_port::ShowTextList(current_text, GetUiPageName(current_page),
        page_selected, partial_refresh, busy_enable);
  }
}

void ScreenRefreshTask(void* arg) {
  (void)arg;
  printf("ScreenRefreshTask start\n");

  while (true) {
    if (screen_refresh_flag) {
      screen_refresh_flag = false;
      UpdateStatusBar();
      RefreshCurrentPage(partial_refresh_flag, false);
      partial_refresh_flag = true;
    }

    lvgl_port::Tick(kScreenRefreshTaskPeriodMs);
    delay(kScreenRefreshTaskPeriodMs);
  }
}

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

    StopAudio();
    es8311_i2s_bus->Deinit();
    if (flash_ready) {
      flash_transport.runCommand(0xB9);
      flash.end();
      flash_ready = false;
    }

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
    InitFlash();

    vTaskResume(screen_refresh_task_handle);
  }
}

void setup() {
  Serial.begin(115200);
  const String build_time = GetSoftwareBuildTime();
  Serial.println(String("[T-Echo-Lite-KeyShield_") + kBoardVersion + "][" +
                 kDisplaySoftwareName + "]_firmware_" + build_time);

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
  InitFlash();

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
    RefreshCurrentPage(false, true);

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
      RefreshCurrentPage(false, true);

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
                        partial_refresh_flag = false;
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
                          page_selected = true;
                        } else {
                          break;
                        }
                        partial_refresh_flag = false;
                        screen_refresh_flag = true;
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kHome) {
                        if (key_text == "Down") {
                          const size_t max_scroll_index =
                              GetHomeMaxScrollIndex();
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
                        partial_refresh_flag = false;
                        screen_refresh_flag = true;
                        StartVibration();
                        break;
                      }

                      if (current_page == UiPage::kAudioTest) {
                        bool use_key_vibration = true;
                        if (key_text == "Down" || key_text == "Up") {
                          audio_target = audio_target == AudioTarget::kMic
                                             ? AudioTarget::kSpeaker
                                             : AudioTarget::kMic;
                          audio_status_text = "Select Mic or Speaker";
                        } else if (key_text == "Center") {
                          use_key_vibration = false;
                          if (audio_target == AudioTarget::kMic) {
                            RecordAudioToFlash();
                          } else {
                            PlayAudioFromFlash();
                          }
                        } else if (key_text == "Esc") {
                          StopAudio();
                          page_selected = false;
                          audio_status_text = "Select Mic or Speaker";
                        } else {
                          break;
                        }
                        partial_refresh_flag = false;
                        screen_refresh_flag = true;
                        if (use_key_vibration) {
                          StartVibration();
                        }
                        break;
                      }

                      if (current_page == UiPage::kKeyboardTest &&
                          key_text == "Esc") {
                        page_selected = false;
                        partial_refresh_flag = false;
                        screen_refresh_flag = true;
                        StartVibration();
                        break;
                      }

                      AddCurrentText(key_text);
                      StartVibration();

                      fast_refresh_count++;
                      if (fast_refresh_count > 30) {
                        partial_refresh_flag = false;
                        fast_refresh_count = 0;
                      }

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
