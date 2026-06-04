/**
 * @file keyboard_view.cpp
 * @brief general_test keyboard page state.
 */
#include "keyboard_view.h"

namespace keyboard_view {
namespace {

constexpr size_t kMaxCurrentTextCount = 8;
std::vector<std::string> current_text;

}  // namespace

const std::vector<std::string>& GetTextList() { return current_text; }

void AddText(const std::string& text) {
  if (current_text.size() >= kMaxCurrentTextCount) {
    current_text.erase(current_text.begin());
  }
  current_text.push_back(text);
}

void Clear() { current_text.clear(); }

}  // namespace keyboard_view
