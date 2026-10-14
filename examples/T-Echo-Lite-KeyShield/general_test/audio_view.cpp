/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:38:32
 * @License: GPL 3.0
 */
#include "audio_view.h"

#include <Arduino.h>
#include <SPI.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "Adafruit_SPIFlash.h"
#include "lvgl_port.h"
#include "t_echo_lite_keyshield_config.h"

namespace audio_view {
namespace {

enum class AudioTarget : uint8_t {
  kMic,
  kSpeaker,
};

struct AudioRecordHeader {
  uint32_t magic = 0;
  uint32_t data_length = 0;
};

constexpr uint32_t kAudioSampleRate = 44100;
constexpr size_t kAudioBufferSampleCount = 1024;
constexpr size_t kAudioBufferBytes =
    kAudioBufferSampleCount * sizeof(uint32_t);
constexpr uint32_t kAudioRecordSeconds = 3;
constexpr uint32_t kAudioRecordDataAddress = 4096;
constexpr uint32_t kAudioRecordBytes =
    kAudioSampleRate * kAudioRecordSeconds * sizeof(uint32_t);
constexpr uint32_t kFlashSectorSize = 4096;
constexpr uint32_t kAudioFlashMagic = 0x41554430;
constexpr char kDefaultStatusText[] = "Select Mic or Speaker";

AudioTarget audio_target = AudioTarget::kMic;
std::string audio_status_text = kDefaultStatusText;
uint32_t audio_recorded_length = 0;
bool audio_record_available = false;
uint32_t audio_buffer[2][kAudioBufferSampleCount] = {0};
bool flash_ready = false;

Adafruit_FlashTransport_QSPI flash_transport(ZD25WQ32C_SCLK, ZD25WQ32C_CS,
    ZD25WQ32C_IO0, ZD25WQ32C_IO1, ZD25WQ32C_IO2, ZD25WQ32C_IO3);
Adafruit_SPIFlash flash(&flash_transport);
SPIFlash_Device_t zd25wq32_devices[] = {
    // ZD25WQ32C, JEDEC ID: BA 60 16
    {
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
    },
    // ZD25Q32D, JEDEC ID: BA 40 16
    {
        total_size : (1UL << 22),
        start_up_time_us : 12000,
        manufacturer_id : 0xBA,
        memory_type : 0x40,
        capacity : 0x16,
        max_clock_speed_mhz : 133,
        quad_enable_bit_mask : 0x02,
        has_sector_protection : false,
        supports_fast_read : true,
        supports_qspi : true,
        supports_qspi_writes : true,
        write_status_register_split : true,
        single_status_byte : false,
        is_fram : false,
    },
};
constexpr size_t kZd25wq32DeviceCount =
    sizeof(zd25wq32_devices) / sizeof(zd25wq32_devices[0]);

/**
 * @brief 更新音频页面状态提示。
 * @param status 状态提示文本。
 */
void ShowStatus(const char* status) {
  audio_status_text = status == nullptr ? "" : status;
  lvgl_port::ShowAudioScreen(true, audio_target == AudioTarget::kMic,
      audio_status_text.c_str(), "Audio", false);
}

/**
 * @brief 擦除音频录制数据使用的 Flash 区域。
 * @return 擦除成功返回 true，否则返回 false。
 */
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

/**
 * @brief 等待 ES8311 I2S 录音读缓冲事件。
 * @param es8311 ES8311 驱动对象。
 * @param timeout_ms 超时时间，单位为毫秒。
 * @return 等待到事件返回 true，超时返回 false。
 */
bool WaitForAudioReadEvent(
    cpp_bus_driver::Es8311& es8311, uint32_t timeout_ms) {
  const uint32_t deadline = millis() + timeout_ms;
  while (millis() < deadline) {
    if (es8311.GetReadI2sEventFlag()) {
      return true;
    }
    delay(1);
  }
  return false;
}

/**
 * @brief 等待 ES8311 I2S 播放写缓冲事件。
 * @param es8311 ES8311 驱动对象。
 * @param timeout_ms 超时时间，单位为毫秒。
 * @return 等待到事件返回 true，超时返回 false。
 */
bool WaitForAudioWriteEvent(
    cpp_bus_driver::Es8311& es8311, uint32_t timeout_ms) {
  const uint32_t deadline = millis() + timeout_ms;
  while (millis() < deadline) {
    if (es8311.GetWriteI2sEventFlag()) {
      return true;
    }
    delay(1);
  }
  return false;
}

/**
 * @brief 从 Flash 加载一段音频数据到播放缓冲区。
 * @param offset 音频数据偏移地址。
 * @param buffer_index 目标音频缓冲区索引。
 * @param length 输出实际读取长度。
 * @return 读取成功返回 true，否则返回 false。
 */
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

/**
 * @brief 录制音频并保存到 Flash。
 * @param es8311 ES8311 驱动对象。
 * @param completion_vibration 完成提示振动回调。
 * @return 录制成功返回 true，否则返回 false。
 */
bool RecordAudioToFlash(
    cpp_bus_driver::Es8311& es8311, void (*completion_vibration)()) {
  if (!flash_ready) {
    ShowStatus("Flash not ready");
    return false;
  }

  ShowStatus("Recording 3s...");
  Stop(es8311);
  if (!EraseAudioFlashArea()) {
    ShowStatus("Flash erase failed");
    return false;
  }

  uint8_t filling_buffer = 0;
  uint8_t free_buffer = 1;
  uint32_t written_bytes = 0;
  const uint32_t record_deadline = millis() + kAudioRecordSeconds * 1000;

  if (!es8311.StartTransmitI2s(
          nullptr, audio_buffer[filling_buffer], kAudioBufferSampleCount)) {
    ShowStatus("Record start failed");
    return false;
  }

  // 启动后的第一次 RXPTRUPD 仅表示 DMA 接管首个缓冲，还没有录满。
  // 先排入第二个缓冲，再等下一次事件才能读取已完成的首个缓冲。
  if (!WaitForAudioReadEvent(es8311, 200) ||
      !es8311.SetNextReadI2s(audio_buffer[free_buffer])) {
    Stop(es8311);
    ShowStatus("Record buffer failed");
    return false;
  }

  while (millis() < record_deadline && written_bytes < kAudioRecordBytes) {
    if (!WaitForAudioReadEvent(es8311, 200)) {
      continue;
    }

    const uint8_t ready_buffer = filling_buffer;
    filling_buffer = free_buffer;
    free_buffer = ready_buffer;
    // filling_buffer 已由 DMA 接管；排入刚释放的 ready_buffer。
    if (!es8311.SetNextReadI2s(audio_buffer[ready_buffer])) {
      Stop(es8311);
      ShowStatus("Record buffer failed");
      return false;
    }

    const uint32_t write_bytes =
        std::min<uint32_t>(kAudioBufferBytes, kAudioRecordBytes - written_bytes);
    if (flash.writeBuffer(kAudioRecordDataAddress + written_bytes,
            reinterpret_cast<uint8_t*>(audio_buffer[ready_buffer]),
            write_bytes) != write_bytes) {
      Stop(es8311);
      ShowStatus("Flash write failed");
      return false;
    }
    flash.waitUntilReady();
    written_bytes += write_bytes;
  }

  Stop(es8311);

  AudioRecordHeader header;
  header.magic = kAudioFlashMagic;
  header.data_length = written_bytes;
  if (flash.writeBuffer(0, reinterpret_cast<uint8_t*>(&header),
          sizeof(header)) != sizeof(header)) {
    ShowStatus("Header write failed");
    return false;
  }
  flash.waitUntilReady();

  audio_recorded_length = written_bytes;
  audio_record_available = written_bytes > 0;
  ShowStatus(audio_record_available ? "Record complete" : "No audio");
  if (audio_record_available && completion_vibration != nullptr) {
    completion_vibration();
  }
  return audio_record_available;
}

/**
 * @brief 从 Flash 读取录音并通过扬声器播放。
 * @param es8311 ES8311 驱动对象。
 * @param completion_vibration 完成提示振动回调。
 * @return 播放成功返回 true，否则返回 false。
 */
bool PlayAudioFromFlash(
    cpp_bus_driver::Es8311& es8311, void (*completion_vibration)()) {
  if (!flash_ready) {
    ShowStatus("Flash not ready");
    return false;
  }

  if (!audio_record_available || audio_recorded_length == 0) {
    ShowStatus("No audio");
    return false;
  }

  ShowStatus("Playing...");
  Stop(es8311);

  uint32_t loaded_bytes = 0;
  uint32_t played_bytes = 0;
  uint32_t buffer_length[2] = {0};
  if (!LoadAudioChunk(0, 0, &buffer_length[0])) {
    ShowStatus("Audio read failed");
    return false;
  }
  loaded_bytes += buffer_length[0];

  // 在启动 DMA 前准备下一段，禁止覆盖仍在播放的缓冲区。
  if (loaded_bytes < audio_recorded_length) {
    if (!LoadAudioChunk(loaded_bytes, 1, &buffer_length[1])) {
      ShowStatus("Audio read failed");
      return false;
    }
    loaded_bytes += buffer_length[1];
  } else {
    memset(audio_buffer[1], 0, kAudioBufferBytes);
  }

  if (!es8311.StartTransmitI2s(audio_buffer[0], nullptr,
          kAudioBufferSampleCount)) {
    ShowStatus("Play start failed");
    return false;
  }

  // 第一次 TXPTRUPD 是首个缓冲开始播放，并非播放完成。
  if (!WaitForAudioWriteEvent(es8311, 500) ||
      !es8311.SetNextWriteI2s(audio_buffer[1])) {
    Stop(es8311);
    ShowStatus("Play buffer failed");
    return false;
  }

  uint8_t active_buffer = 0;

  while (played_bytes < audio_recorded_length) {
    if (!WaitForAudioWriteEvent(es8311, 500)) {
      Stop(es8311);
      ShowStatus("Play timeout");
      return false;
    }

    // 下一缓冲已开始播放，只有上一缓冲现在可安全写入。
    const uint8_t released_buffer = active_buffer;
    played_bytes += buffer_length[released_buffer];
    active_buffer = active_buffer == 0 ? 1 : 0;
    if (played_bytes >= audio_recorded_length) {
      break;
    }

    if (loaded_bytes < audio_recorded_length) {
      if (!LoadAudioChunk(loaded_bytes, released_buffer,
              &buffer_length[released_buffer])) {
        Stop(es8311);
        ShowStatus("Audio read failed");
        return false;
      }
      loaded_bytes += buffer_length[released_buffer];
    } else {
      // 最后一段播放时排入静音，避免 DMA 重播旧音频。
      memset(audio_buffer[released_buffer], 0, kAudioBufferBytes);
      buffer_length[released_buffer] = 0;
    }
    if (!es8311.SetNextWriteI2s(audio_buffer[released_buffer])) {
      Stop(es8311);
      ShowStatus("Play buffer failed");
      return false;
    }
  }

  Stop(es8311);
  ShowStatus("Play complete");
  if (completion_vibration != nullptr) {
    completion_vibration();
  }
  return true;
}

}  // namespace

bool InitFlash() {
  if (!flash.begin(zd25wq32_devices, kZd25wq32DeviceCount)) {
    printf("flash init failed\n");
    flash_ready = false;
    return false;
  }
  printf("flash JEDEC ID: 0x%06lX\n",
      static_cast<unsigned long>(flash.getJEDECID()));
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

void EndFlash() {
  if (flash_ready) {
    flash_transport.runCommand(0xB9);
    flash.end();
    flash_ready = false;
  }
}

void Stop(cpp_bus_driver::Es8311& es8311) { es8311.StopTransmitI2s(); }

void Show(bool page_selected, const char* page_name, bool busy_enable) {
  lvgl_port::ShowAudioScreen(page_selected, audio_target == AudioTarget::kMic,
      audio_status_text.c_str(), page_name, busy_enable);
}

void ResetPrompt() { audio_status_text = kDefaultStatusText; }

KeyResult HandleKey(const std::string& key_text,
    cpp_bus_driver::Es8311& es8311, void (*completion_vibration)()) {
  KeyResult result;
  result.handled = true;

  if (key_text == "Down" || key_text == "Up") {
    audio_target =
        audio_target == AudioTarget::kMic ? AudioTarget::kSpeaker
                                          : AudioTarget::kMic;
    ResetPrompt();
  } else if (key_text == "Center") {
    result.use_key_vibration = false;
    if (audio_target == AudioTarget::kMic) {
      RecordAudioToFlash(es8311, completion_vibration);
    } else {
      PlayAudioFromFlash(es8311, completion_vibration);
    }
  } else if (key_text == "Esc") {
    Stop(es8311);
    ResetPrompt();
  } else {
    result.handled = false;
  }

  return result;
}

}  // namespace audio_view
