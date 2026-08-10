/*
 * @Description: speaker_certification
 * @Author: LILYGO_L
 * @Date: 2026-08-10
 * @LastEditTime: 2026-08-10
 * @License: GPL 3.0
 */

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <Wire.h>

#include "cpp_bus_driver_library.h"
#include "t_echo_lite_keyshield_config.h"

static constexpr uint16_t kAudioMclkMultiple = 32;
static constexpr uint32_t kAudioSampleRateHz = 44100;
static constexpr uint8_t kAudioBitsPerSample = 16;
static constexpr uint32_t kSquareWaveFrequencyHz = 1000;
static constexpr int16_t kSquareWaveAmplitude = 10000;
static constexpr uint8_t kDacVolume = 175;  // -8 dB，避免上电音量过大。

// 441 个采样点正好包含 10 个 1 kHz 周期，循环播放时相位连续。
static constexpr size_t kI2sBufferWordCount = 441;
static_assert(
    (kI2sBufferWordCount * kSquareWaveFrequencyHz) % kAudioSampleRateHz == 0,
    "The I2S buffer must contain a whole number of square-wave periods");
static uint32_t i2s_tx_buffers[2][kI2sBufferWordCount] = {};
static bool square_wave_started = false;

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
    printf("ES8311 init failed\n");
    return false;
  }

  if (!es8311.Init(
          kAudioMclkMultiple, kAudioSampleRateHz, kAudioBitsPerSample)) {
    printf("ES8311 I2S init failed\n");
    return false;
  }

  // 仅启用扬声器输出所需的模拟偏置、DAC 参考和耳机驱动通路。
  cpp_bus_driver::Es8311::PowerStatus power_status;
  power_status.contorl.analog_circuits = true;
  power_status.contorl.analog_bias_circuits = true;
  power_status.contorl.analog_adc_bias_circuits = false;
  power_status.contorl.analog_adc_reference_circuits = false;
  power_status.contorl.analog_dac_reference_circuit = true;
  power_status.contorl.internal_reference_circuits = false;
  power_status.vmid =
      cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge;

  bool result = true;
  result &= es8311.SetPowerStatus(power_status);
  result &= es8311.SetPgaPower(false);
  result &= es8311.SetAdcPower(false);
  result &= es8311.SetDacPower(true);
  result &= es8311.SetOutputToHpDrive(true);
  result &= es8311.SetDacEqualizer(false);
  result &= es8311.SetDacVolume(kDacVolume);

  if (!result) {
    printf("ES8311 config failed\n");
  }
  return result;
}

uint32_t PackStereoSample(int16_t sample) {
  const uint16_t packed_sample = static_cast<uint16_t>(sample);
  return static_cast<uint32_t>(packed_sample) |
         (static_cast<uint32_t>(packed_sample) << 16);
}

void FillSquareWaveBuffer(uint32_t* buffer) {
  for (size_t i = 0; i < kI2sBufferWordCount; i++) {
    const uint32_t phase =
        (i * kSquareWaveFrequencyHz) % kAudioSampleRateHz;
    const int16_t sample = phase < (kAudioSampleRateHz / 2)
                               ? kSquareWaveAmplitude
                               : -kSquareWaveAmplitude;
    buffer[i] = PackStereoSample(sample);
  }
}

bool StartSquareWave() {
  FillSquareWaveBuffer(i2s_tx_buffers[0]);
  FillSquareWaveBuffer(i2s_tx_buffers[1]);

  return Es8311().StartTransmitI2s(
      i2s_tx_buffers[0], nullptr, kI2sBufferWordCount);
}

void setup() {
  Serial.begin(115200);
  uint8_t serial_init_count = 0;
  while (!Serial && serial_init_count < 30) {
    delay(100);  // 最多等待原生 USB 串口 3 秒。
    serial_init_count++;
  }

  printf("Speaker certification: 1 kHz square wave\n");
  PowerOnKeyShield3v3();

  if (!ConfigureEs8311(Es8311())) {
    printf("Speaker certification init failed\n");
    return;
  }

  if (!StartSquareWave()) {
    printf("Square wave output start failed\n");
    return;
  }

  square_wave_started = true;
  printf("Square wave output started: 1000 Hz, 44.1 kHz, 16-bit\n");
}

void loop() {
  static uint8_t next_buffer_index = 1;

  if (!square_wave_started) {
    delay(1000);
    return;
  }

  if (Es8311().GetWriteI2sEventFlag()) {
    if (!Es8311().SetNextWriteI2s(i2s_tx_buffers[next_buffer_index])) {
      Es8311().StopTransmitI2s();
      printf("Square wave output stopped\n");
      while (true) {
        delay(1000);
      }
    }
    next_buffer_index ^= 1;
  } else {
    delay(1);
  }
}
