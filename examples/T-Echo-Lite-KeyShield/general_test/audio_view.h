/**
 * @file audio_view.h
 * @brief general_test audio page state and actions.
 */
#ifndef T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_AUDIO_VIEW_H_
#define T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_AUDIO_VIEW_H_

#include <string>

#include "cpp_bus_driver_library.h"

namespace audio_view {

struct KeyResult {
  bool handled = false;
  bool use_key_vibration = true;
};

bool InitFlash();
void EndFlash();
void Stop(cpp_bus_driver::Es8311& es8311);
void Show(bool page_selected, const char* page_name, bool busy_enable = false);
void ResetPrompt();
KeyResult HandleKey(const std::string& key_text,
    cpp_bus_driver::Es8311& es8311, void (*completion_vibration)());

}  // namespace audio_view

#endif  // T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_AUDIO_VIEW_H_
