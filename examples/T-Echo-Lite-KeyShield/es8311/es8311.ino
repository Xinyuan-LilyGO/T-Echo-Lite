/*
 * @Description: es8311
 * @Author: LILYGO_L
 * @Date: 2025-08-25 16:09:08
 * @LastEditTime: 2026-06-01 16:59:26
 * @License: GPL 3.0
 */

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <Wire.h>

#include "new_notification_010_c2_b16_s44100.h"
#include "cpp_bus_driver_library.h"
#include "t_echo_lite_keyshield_config.h"

static constexpr uint16_t kAudioMclkMultiple = 32;
static constexpr uint32_t kAudioSampleRateHz = 44100;
static constexpr uint8_t kAudioBitsPerSample = 16;
static constexpr size_t kI2sBufferWordCount = 512;
static constexpr size_t kI2sBufferByteSize =
    kI2sBufferWordCount * sizeof(uint32_t);
static constexpr uint32_t kFinalBufferDelayMs = 50;
static constexpr uint32_t kI2sEventTimeoutMs = 1000;
static constexpr uint32_t kButtonDebounceMs = 300;
static constexpr uint32_t kLoopDelayMs = 10;

static uint32_t i2s_tx_buffers[2][kI2sBufferWordCount] = {};

std::shared_ptr<cpp_bus_driver::HardwareI2c2>& Es8311I2cBus() {
  static auto es8311_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c2>(
          ES8311_SDA, ES8311_SCL, &Wire);
  return es8311_i2c_bus;
}

std::shared_ptr<cpp_bus_driver::HardwareI2s>& Es8311I2sBus() {
  static auto es8311_i2s_bus =
      std::make_shared<cpp_bus_driver::HardwareI2s>(
          ES8311_ADC_DATA, ES8311_DAC_DATA, ES8311_WS_LRCK, ES8311_BCLK,
          ES8311_MCLK);
  return es8311_i2s_bus;
}

cpp_bus_driver::Es8311& Es8311() {
  static auto es8311 = std::make_unique<cpp_bus_driver::Es8311>(
      Es8311I2cBus(), Es8311I2sBus(), ES8311_IIC_ADDRESS);
  return *es8311;
}

void PowerOnKeyShield3v3() {
  pinMode(RT9080_EN, OUTPUT);
  digitalWrite(RT9080_EN, HIGH);
  delay(100);
  digitalWrite(RT9080_EN, LOW);
  delay(100);
  digitalWrite(RT9080_EN, HIGH);
  delay(1000);
}

bool ConfigureEs8311(cpp_bus_driver::Es8311& es8311) {
  if (!es8311.Init()) {
    printf("Es8311 init failed\n");
    return false;
  }

  if (!es8311.Init(
          kAudioMclkMultiple, kAudioSampleRateHz, kAudioBitsPerSample)) {
    printf("Es8311 i2s init failed\n");
    return false;
  }

  // 启用模拟电路、ADC 和 DAC，用于麦克风输入与音频输出。
  cpp_bus_driver::Es8311::PowerStatus power_status;
  power_status.contorl.analog_circuits = true;
  power_status.contorl.analog_bias_circuits = true;
  power_status.contorl.analog_adc_bias_circuits = true;
  power_status.contorl.analog_adc_reference_circuits = true;
  power_status.contorl.analog_dac_reference_circuit = true;
  power_status.contorl.internal_reference_circuits = false;
  power_status.vmid =
      cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge;

  bool result = true;
  result &= es8311.SetPowerStatus(power_status);
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
  result &=
      es8311.SetAdcPgaGain(cpp_bus_driver::Es8311::AdcPgaGain::kGain30db);
  result &= es8311.SetAdcVolume(191);
  result &= es8311.SetDacVolume(191);

  if (!result) {
    printf("Es8311 config failed\n");
  }
  return result;
}

size_t FillI2sBuffer(
    const void* data, size_t data_size, size_t offset, uint32_t* buffer) {
  memset(buffer, 0, kI2sBufferByteSize);

  if (offset >= data_size) {
    return 0;
  }

  const size_t remaining_byte = data_size - offset;
  const size_t copy_byte = remaining_byte < kI2sBufferByteSize
                               ? remaining_byte
                               : kI2sBufferByteSize;
  const uint8_t* input = static_cast<const uint8_t*>(data) + offset;
  memcpy(buffer, input, copy_byte);

  return copy_byte;
}

bool PlayAudio(const void* data, size_t data_size) {
  if (data == nullptr || data_size == 0) {
    return false;
  }

  auto& es8311 = Es8311();
  size_t send_byte = 0;
  uint8_t buffer_index = 0;

  send_byte +=
      FillI2sBuffer(data, data_size, send_byte, i2s_tx_buffers[buffer_index]);
  if (!es8311.StartTransmitI2s(
          i2s_tx_buffers[buffer_index], nullptr, kI2sBufferWordCount)) {
    return false;
  }

  buffer_index ^= 1;
  uint32_t wait_ms = 0;
  while (send_byte < data_size) {
    if (es8311.GetWriteI2sEventFlag()) {
      wait_ms = 0;
      send_byte += FillI2sBuffer(
          data, data_size, send_byte, i2s_tx_buffers[buffer_index]);
      if (!es8311.SetNextWriteI2s(i2s_tx_buffers[buffer_index])) {
        es8311.StopTransmitI2s();
        return false;
      }
      buffer_index ^= 1;
    } else {
      wait_ms++;
      if (wait_ms > kI2sEventTimeoutMs) {
        es8311.StopTransmitI2s();
        return false;
      }
    }
    delay(1);
  }

  delay(kFinalBufferDelayMs);
  es8311.StopTransmitI2s();
  return true;
}

void DumpEs8311Registers() {
  uint8_t buffer = 0;
  for (size_t i = 0; i < 256; i++) {
    Es8311I2cBus()->BusI2cGuide::Read(static_cast<uint8_t>(i), &buffer);
    printf("Es8311 register[%u]: %#X\n", static_cast<unsigned int>(i),
        buffer);
  }
}

void setup() {
  Serial.begin(115200);
  uint8_t serial_init_count = 0;
  while (!Serial) {
    delay(100);  // 等待原生 USB 串口就绪。
    serial_init_count++;
    if (serial_init_count > 30) {
      break;
    }
  }
  printf("Ciallo\n");

  PowerOnKeyShield3v3();
  pinMode(nRF52840_BOOT, INPUT_PULLUP);

  auto& es8311 = Es8311();
  ConfigureEs8311(es8311);
}

void loop() {
  static size_t play_count = 1;

  if (digitalRead(nRF52840_BOOT) == LOW) {
    delay(kButtonDebounceMs);

    DumpEs8311Registers();

    play_count++;
    printf("play_count: %u\n", static_cast<unsigned int>(play_count));

    // 播放音乐测试。
    printf("music play start\n");
    if (PlayAudio(c2_b16_s44100, sizeof(c2_b16_s44100))) {
      printf("music play finish\n");
    } else {
      printf("music play fail\n");
    }
  }

  delay(kLoopDelayMs);
}
