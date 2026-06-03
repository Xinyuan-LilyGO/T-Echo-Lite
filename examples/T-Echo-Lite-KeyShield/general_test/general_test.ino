/*
 * @Description: original_test
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-03 12:11:16
 * @License: GPL 3.0
 */
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <Wire.h>

#include "Adafruit_EPD.h"
#include "Display_Fonts.h"
#include "c2_b16_s44100_2.h"
#include "c2_b16_s44100_3.h"
#include "cpp_bus_driver_library.h"
#include "material_monochrome_176x192px.h"
#include "t_echo_lite_keyshield_config.h"

static constexpr char kSoftwareName[] = "Original_Test";
static constexpr char kSoftwareLastEditTime[] = "202604090958";
static constexpr char kBoardVersion[] = "V1.0";
static constexpr uint8_t kAudioMclkMultiple = 32;
static constexpr uint32_t kAudioSampleRate = 44100;
static constexpr uint8_t kAudioBitsPerSample = 16;
static constexpr size_t kMaxIisDataTransmitSize = 1024;
static constexpr size_t kIisTxBufferBytes =
    kMaxIisDataTransmitSize * sizeof(uint32_t);
static constexpr size_t kMaxCurrentTextCount = 8;
static constexpr uint32_t kAutoSleepTimeoutMs = 10000;
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

struct SleepOperator {
  enum class Mode : uint8_t {
    kNotSleep,
    kLightSleep,
  };
  size_t wake_deadline_ms = 0;
  Mode current_mode = Mode::kNotSleep;
};

std::vector<std::string> current_text;

bool screen_refresh_flag = false;

bool screen_partial_refresh_init_lock = false;

uint32_t iis_tx_buffer[2][kMaxIisDataTransmitSize] = {0};
uint32_t iis_rx_buffer[kMaxIisDataTransmitSize] = {0};

uint8_t current_iis_tx_buffer_index = 0;
bool iis_tx_buffer_full[2] = {false};

// 已发送的数据大小。
size_t iis_send_data_size = 0;

bool partial_refresh_flag = true;
size_t fast_refresh_count = 0;

TaskHandle_t screen_refresh_task_handle = nullptr;

SPIClass custom_spi1(NRF_SPIM1, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC, SCREEN_RST,
    SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &custom_spi1, 8000000);

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

void CopyIisData(const void* input_data, void* out_buffer,
    size_t input_data_start_index, size_t byte) {
  const uint8_t* input_ptr =
      static_cast<const uint8_t*>(input_data) + input_data_start_index;
  uint8_t* out_ptr = static_cast<uint8_t*>(out_buffer);

  memcpy(out_ptr, input_ptr, byte);
}

uint8_t GetNextIisTxBufferIndex(uint8_t index) { return index == 0 ? 1 : 0; }

void StartVibration() {
  auto& aw86224 = GetAw86224();
  if (!aw86224.PlayRamWaveform(
          kVibrationSequence, kVibrationLoopCount, kVibrationGain, true)) {
    printf("StartVibration failed\n");
  }
}

void ReadMicrophone() {
  auto& es8311 = GetEs8311();

  printf("ReadMicrophone start\n");
  if (!es8311.StartTransmitI2s(
          nullptr, iis_rx_buffer, kMaxIisDataTransmitSize)) {
    printf("ReadMicrophone failed (es8311.StartTransmitI2s failed)\n");
  }

  size_t timeout_count = 0;

  for (size_t i = 0; i < 2; i++) {
    while (true) {
      if (es8311.GetReadI2sEventFlag()) {
        es8311.SetNextReadI2s(iis_rx_buffer);
        break;
      }

      timeout_count++;
      if (timeout_count > 10) {
        printf("ReadMicrophone timeout\n");
        break;
      }
      delay(10);
    }
  }

  printf("ReadMicrophone finished\n");
  es8311.StopTransmitI2s();
}

bool PlayMusic(const uint16_t* data, size_t size) {
  // 这边测试音乐太吵暂时静音一下
  return true;
  auto& es8311 = GetEs8311();

  // 播放音乐测试。
  printf("PlayMusic start\n");

  current_iis_tx_buffer_index = 0;
  iis_tx_buffer_full[0] = false;
  iis_tx_buffer_full[1] = false;
  iis_send_data_size = 0;
  CopyIisData(
      data, iis_tx_buffer[current_iis_tx_buffer_index], 0, kIisTxBufferBytes);
  iis_tx_buffer_full[current_iis_tx_buffer_index] = true;

  if (es8311.StartTransmitI2s(iis_tx_buffer[current_iis_tx_buffer_index],
          nullptr, kMaxIisDataTransmitSize)) {
    iis_send_data_size += kIisTxBufferBytes;

    iis_tx_buffer_full[current_iis_tx_buffer_index] = false;
    current_iis_tx_buffer_index =
        GetNextIisTxBufferIndex(current_iis_tx_buffer_index);
  } else {
    printf("PlayMusic failed (es8311.StartTransmitI2s failed)\n");

    return false;
  }

  while (true) {
    if (iis_send_data_size >= size) {
      printf("PlayMusic finished, sent bytes: %u\n",
          static_cast<unsigned int>(iis_send_data_size));
      es8311.StopTransmitI2s();

      break;
    } else {
      const uint8_t next_buffer_index =
          GetNextIisTxBufferIndex(current_iis_tx_buffer_index);
      if (!iis_tx_buffer_full[current_iis_tx_buffer_index]) {
        size_t buffer_length =
            min(kIisTxBufferBytes, size - iis_send_data_size);

        // printf("iis_send_data_size: %d\n", iis_send_data_size);
        // printf("iis send data length: %d\n", buffer_length);

        if (buffer_length != kIisTxBufferBytes) {
          memset(
              iis_tx_buffer[current_iis_tx_buffer_index], 0, kIisTxBufferBytes);
        }
        CopyIisData(data, iis_tx_buffer[current_iis_tx_buffer_index],
            iis_send_data_size, buffer_length);

        iis_send_data_size += buffer_length;

        iis_tx_buffer_full[current_iis_tx_buffer_index] = true;
      } else if (!iis_tx_buffer_full[next_buffer_index]) {
        size_t buffer_length =
            min(kIisTxBufferBytes, size - iis_send_data_size);

        // printf("iis_send_data_size: %d\n", iis_send_data_size);
        // printf("iis send data length: %d\n", buffer_length);

        if (buffer_length != kIisTxBufferBytes) {
          memset(iis_tx_buffer[next_buffer_index], 0, kIisTxBufferBytes);
        }
        CopyIisData(data, iis_tx_buffer[next_buffer_index], iis_send_data_size,
            buffer_length);

        iis_send_data_size += buffer_length;

        iis_tx_buffer_full[next_buffer_index] = true;
      }
    }

    if (es8311.GetWriteI2sEventFlag()) {
      if (iis_tx_buffer_full[current_iis_tx_buffer_index]) {
        es8311.SetNextWriteI2s(iis_tx_buffer[current_iis_tx_buffer_index]);

        iis_tx_buffer_full[current_iis_tx_buffer_index] = false;

        current_iis_tx_buffer_index =
            GetNextIisTxBufferIndex(current_iis_tx_buffer_index);
      }
    }
  }

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
  return ((static_cast<float>(analogRead(BATTERY_ADC_DATA)) *
              (kAdcReferenceMv / kAdcResolutionCount)) /
             1000.0f) *
         kBatteryDividerRatio;
}

void ScreenRefreshTask(void* arg) {
  (void)arg;
  printf("ScreenRefreshTask start\n");

  while (true) {
    if (screen_refresh_flag) {
      screen_refresh_flag = false;

      if (current_text.size() == 0) {
        display.fillScreen(EPD_WHITE);
        display.clearBuffer();
        display.setTextColor(EPD_BLACK);
        display.setTextSize(1);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(15, 70);
        display.print("Please enter the text");

        display.display(display.Update_Mode::FAST_REFRESH, true);
      } else {
        std::string show_text;
        for (uint8_t i = 0; i < current_text.size(); i++) {
          show_text += "[" + current_text[i] + "]\n";
        }
        display.fillScreen(EPD_WHITE);
        display.clearBuffer();
        display.setTextColor(EPD_BLACK);
        display.setTextSize(1);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(0, 13);
        display.print(show_text.c_str());

        if (partial_refresh_flag) {
          if (!screen_partial_refresh_init_lock) {
            display.setRAMValueBaseMap(display.Update_Mode::FAST_REFRESH);
            screen_partial_refresh_init_lock = true;
          }
          display.display(display.Update_Mode::PARTIAL_REFRESH, true);
        } else {
          display.display(display.Update_Mode::FAST_REFRESH, true, false);

          partial_refresh_flag = true;
          screen_partial_refresh_init_lock = false;
        }
      }
    }

    delay(10);
  }
}

void SetSystemSleep(bool enable) {
  if (enable) {
    auto& es8311_i2s_bus = GetEs8311I2sBus();

    Serial.end();
    display.end();
    pinMode(SCREEN_BS1, INPUT);

    Wire.end();

    pinMode(IIC_1_SDA, INPUT);
    pinMode(IIC_1_SCL, INPUT);

    pinMode(BATTERY_MEASUREMENT_CONTROL, INPUT);

    es8311_i2s_bus->Deinit();

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
    display.begin();
    display.setRotation(1);

    InitTca8418();
    InitAw21009(kAw21009MaxBrightness);
    InitAw86224();
    InitEs8311();

    vTaskResume(screen_refresh_task_handle);
  }
}

void setup() {
  Serial.begin(115200);
  uint8_t serial_init_count = 0;
  while (!Serial) {
    delay(100);  // Wait for native USB.
    serial_init_count++;
    if (serial_init_count > 30) {
      break;
    }
  }
  Serial.println("Ciallo");
  Serial.println(String("[T-Echo-Lite-KeyShield_") + kBoardVersion + "][" +
                 kSoftwareName + "]_firmware_" + kSoftwareLastEditTime);

  // 3.3V Power ON
  pinMode(RT9080_EN, OUTPUT);
  digitalWrite(RT9080_EN, HIGH);
  delay(100);
  digitalWrite(RT9080_EN, LOW);
  delay(100);
  digitalWrite(RT9080_EN, HIGH);
  delay(1000);

  pinMode(SCREEN_BS1, OUTPUT);
  digitalWrite(SCREEN_BS1, LOW);

  pinMode(nRF52840_BOOT, INPUT_PULLUP);

  pinMode(TCA8418_INT, INPUT_PULLUP);
  ConfigureBatteryMeasurement();

  InitTca8418();
  InitAw21009(0);
  InitAw86224();
  InitEs8311();

  display.begin();
  display.setRotation(1);
  display.fillScreen(EPD_WHITE);
  display.drawBitmap(0, 0, gImage_1, 192, 176, EPD_BLACK);

  display.setTextColor(EPD_WHITE);
  display.setTextSize(1);
  display.setFont(&Org_01);
  display.setCursor(25, 90);
  display.print("MCU: nRF52840");
  display.setCursor(25, 100);
  display.print("Screen: GDEM0122T61");
  display.setCursor(25, 110);
  display.print("Keyboard: Tca8418");
  display.setCursor(25, 120);
  display.print("Dac: Es8311");
  display.setCursor(25, 130);
  display.print("Vibration: Aw86224");

  display.setCursor(25, 150);
  display.print("Software: " + String(kSoftwareName));
  display.setCursor(25, 160);
  display.print("LastEditTime: " + String(kSoftwareLastEditTime));

  display.display(display.Update_Mode::FULL_REFRESH, true);

  PlayMusic(c2_b16_s44100_2, sizeof(c2_b16_s44100_2));

  delay(3000);

  GetAw21009().SetBrightness(
      cpp_bus_driver::Aw21009::LedChannel::kAll, kAw21009MaxBrightness);

  xTaskCreate(ScreenRefreshTask, "ScreenRefreshTask", 1024, nullptr, 3,
      &screen_refresh_task_handle);

  screen_refresh_flag = true;

  ResetAutoSleepTimer();
}

void loop() {
  // 自动进入休眠检测。
  if (sleep_op.current_mode == SleepOperator::Mode::kNotSleep &&
      millis() > sleep_op.wake_deadline_ms) {
    Serial.println("Light sleep on");

    // 显示休眠提示。
    display.fillScreen(EPD_WHITE);
    display.setCursor(15, 70);
    display.setTextColor(EPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.print("Light sleep on");
    display.display(display.Update_Mode::FAST_REFRESH, true);

    delay(3000);
    SetSystemSleep(true);
    sleep_op.current_mode = SleepOperator::Mode::kLightSleep;
  }

  // 休眠状态下的处理。
  if (sleep_op.current_mode == SleepOperator::Mode::kLightSleep) {
    if (digitalRead(nRF52840_BOOT) == LOW) {
      SetSystemSleep(false);

      Serial.println("Awakening");

      display.fillScreen(EPD_WHITE);
      display.setCursor(15, 70);
      display.print("Awakening");
      display.display(display.Update_Mode::FAST_REFRESH, true);

      sleep_op.current_mode = SleepOperator::Mode::kNotSleep;
      // 重置自动休眠计时。
      ResetAutoSleepTimer();
    } else {
      waitForEvent();
      delay(1000);

      // systemOff(nRF52840_BOOT, LOW);
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
                      // 有按键操作就刷新自动休眠计时。
                      ResetAutoSleepTimer();

                      AddCurrentText(Tca8418_Map[key_index]);

                      PlayMusic(c2_b16_s44100_3, sizeof(c2_b16_s44100_3));

                      if (Tca8418_Map[key_index] == "Home") {
                        float voltage = ReadBatteryVoltage();
                        char voltage_str[32];
                        snprintf(voltage_str, sizeof(voltage_str), "Bat:%.3fv",
                            voltage);

                        AddCurrentText(voltage_str);

                        ReadMicrophone();

                        int16_t buffer_microphone =
                            static_cast<int16_t>(iis_rx_buffer[0]);
                        char microphone_str[32];
                        snprintf(microphone_str, sizeof(microphone_str),
                            "Mic:%d", buffer_microphone);

                        AddCurrentText(microphone_str);
                      }

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
