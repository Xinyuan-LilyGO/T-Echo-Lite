/**
 * @file lvgl_port.h
 * @brief general_test LVGL display port.
 */
#ifndef T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_LVGL_PORT_H_
#define T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_LVGL_PORT_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

namespace lvgl_port {

void Init();
void BeginDisplay();
void EndDisplay();
void Tick(uint32_t elapsed_ms);
void ResetPartialRefresh();
void SetBatteryPercentage(uint8_t percentage);
void SetSleepMode(bool enable);
void ShowBootScreen();
void ShowHomeScreen(const std::vector<std::string>& lines, size_t scroll_index,
    const char* page_name, bool page_selected, bool busy_enable = false);
void ShowCenteredText(const char* text);
void ShowTextList(const std::vector<std::string>& text_list,
    const char* page_name, bool page_selected, bool partial_refresh,
    bool busy_enable = false);
void ShowAudioScreen(bool page_selected, bool mic_selected, const char* message,
    const char* page_name, bool busy_enable = false);

}  // namespace lvgl_port

#endif  // T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_LVGL_PORT_H_
