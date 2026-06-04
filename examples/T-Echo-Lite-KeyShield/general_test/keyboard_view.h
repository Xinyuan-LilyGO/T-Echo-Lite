/**
 * @file keyboard_view.h
 * @brief general_test keyboard page state.
 */
#ifndef T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_KEYBOARD_VIEW_H_
#define T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_KEYBOARD_VIEW_H_

#include <stddef.h>

#include <string>
#include <vector>

namespace keyboard_view {

const std::vector<std::string>& GetTextList();
void AddText(const std::string& text);
void Clear();

}  // namespace keyboard_view

#endif  // T_ECHO_LITE_KEYSHIELD_GENERAL_TEST_KEYBOARD_VIEW_H_
