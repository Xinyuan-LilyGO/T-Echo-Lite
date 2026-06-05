/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:38:56
 * @License: GPL 3.0
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
