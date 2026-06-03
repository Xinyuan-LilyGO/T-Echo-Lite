/**
 * @file lvgl_port.h
 * @brief general_test LVGL display port.
 */
#ifndef T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_LVGL_PORT_H_
#define T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_LVGL_PORT_H_

#include <stdint.h>

#include <string>
#include <vector>

namespace lvgl_port {

void Init();
void BeginDisplay();
void EndDisplay();
void Tick(uint32_t elapsed_ms);
void ResetPartialRefresh();
void ShowBootScreen(const char* build_time);
void ShowCenteredText(const char* text);
void ShowTextList(const std::vector<std::string>& text_list,
    bool partial_refresh);

}  // namespace lvgl_port

#endif  // T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_LVGL_PORT_H_
