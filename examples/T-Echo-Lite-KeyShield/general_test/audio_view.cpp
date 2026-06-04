/**
 * @file audio_view.cpp
 * @brief general_test audio page state and actions.
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

void ShowStatus(const char* status) {
  audio_status_text = status == nullptr ? "" : status;
  lvgl_port::ShowAudioScreen(true, audio_target == AudioTarget::kMic,
      audio_status_text.c_str(), "Audio", false);
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

  while (millis() < record_deadline && written_bytes < kAudioRecordBytes) {
    if (!WaitForAudioReadEvent(es8311, 200)) {
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
  bool buffer_ready[2] = {false};
  if (!LoadAudioChunk(0, 0, &buffer_length[0])) {
    ShowStatus("Audio read failed");
    return false;
  }
  loaded_bytes += buffer_length[0];
  buffer_ready[0] = true;

  if (!es8311.StartTransmitI2s(audio_buffer[0], nullptr,
          kAudioBufferSampleCount)) {
    ShowStatus("Play start failed");
    return false;
  }

  buffer_ready[0] = false;
  uint8_t current_buffer = 1;
  uint32_t active_buffer_length = buffer_length[0];

  while (played_bytes < audio_recorded_length) {
    if (!buffer_ready[current_buffer] && loaded_bytes < audio_recorded_length) {
      if (!LoadAudioChunk(
              loaded_bytes, current_buffer, &buffer_length[current_buffer])) {
        Stop(es8311);
        ShowStatus("Audio read failed");
        return false;
      }
      loaded_bytes += buffer_length[current_buffer];
      buffer_ready[current_buffer] = true;
    }

    const uint8_t next_buffer = current_buffer == 0 ? 1 : 0;
    if (!buffer_ready[next_buffer] && loaded_bytes < audio_recorded_length) {
      if (!LoadAudioChunk(
              loaded_bytes, next_buffer, &buffer_length[next_buffer])) {
        Stop(es8311);
        ShowStatus("Audio read failed");
        return false;
      }
      loaded_bytes += buffer_length[next_buffer];
      buffer_ready[next_buffer] = true;
    }

    if (!WaitForAudioWriteEvent(es8311, 500)) {
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

  Stop(es8311);
  ShowStatus("Play complete");
  if (completion_vibration != nullptr) {
    completion_vibration();
  }
  return true;
}

}  // namespace

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
