/**
 * @file home_view.cpp
 * @brief general_test home page data provider.
 */
#include "home_view.h"

#include <Arduino.h>
#include <lvgl.h>

#include <cstdio>

#include "t_echo_lite_keyshield_config.h"

namespace home_view {
namespace {

constexpr char kDisplaySoftwareName[] = "general_test";
constexpr char kBoardVersion[] = "v1.0";
constexpr size_t kHomeVisibleLineCount = 10;

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

}  // namespace

const char* GetSoftwareName() { return kDisplaySoftwareName; }

const char* GetBoardVersion() { return kBoardVersion; }

String GetBuildTime() { return GetSoftwareBuildTime(); }

std::vector<std::string> CreateLines() {
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

size_t GetMaxScrollIndex() {
  const size_t line_count = CreateLines().size();
  return line_count > kHomeVisibleLineCount ? line_count - kHomeVisibleLineCount
                                            : 0;
}

}  // namespace home_view
