/**
 * @file lvgl_port.cpp
 * @brief general_test LVGL display port.
 */
#include "lvgl_port.h"

#include <algorithm>

#include <Arduino.h>

#include "Adafruit_EPD.h"
#include "lvgl.h"
#include "t_echo_lite_keyshield_config.h"

LV_FONT_DECLARE(lvgl_font_google_sans_flex_regular_13)
LV_FONT_DECLARE(lvgl_font_google_sans_flex_regular_14)
LV_FONT_DECLARE(lvgl_font_google_sans_flex_regular_28)
LV_FONT_DECLARE(lvgl_font_material_symbols_rounded_20)
LV_FONT_DECLARE(lvgl_font_material_symbols_rounded_32)

namespace lvgl_port {
namespace {

constexpr int32_t kDisplayWidth = SCREEN_HEIGHT;
constexpr int32_t kDisplayHeight = SCREEN_WIDTH;
constexpr int32_t kStatusBarHeight = 20;
constexpr int32_t kFooterHeight = 20;
constexpr int32_t kHomeVisibleLineCount = 10;
constexpr int32_t kHomeLineHeight = 13;
constexpr int32_t kHomeContentTop = kStatusBarHeight + 2;
constexpr int32_t kKeyboardContentTop = kStatusBarHeight + 4;
constexpr uint16_t kDrawBufferRows = 8;
constexpr size_t kDrawBufferSize =
    kDisplayWidth * kDrawBufferRows * sizeof(lv_color16_t);

constexpr const char* kBatteryAndroidIcons[] = {
    "\xEF\x8C\x8D", "\xEF\x8C\x8C", "\xEF\x8C\x8B", "\xEF\x8C\x8A",
    "\xEF\x8C\x89", "\xEF\x8C\x88", "\xEF\x8C\x87"};
constexpr const char* kBedtimeIcon = "\xEF\x85\x99";
constexpr const char* kMicIcon = "\xEE\x8C\x9D";
constexpr const char* kSpeakerIcon = "\xEE\x8C\xAD";

SPIClass custom_spi1(NRF_SPIM1, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 epd_display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC,
    SCREEN_RST, SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &custom_spi1, 8000000);

lv_display_t* lvgl_display = nullptr;
alignas(LV_DRAW_BUF_ALIGN) uint8_t lvgl_draw_buffer[kDrawBufferSize] = {0};
bool partial_refresh_base_map_ready = false;
uint8_t battery_percentage = 0;
bool sleep_mode = false;

void LvglFlushCallback(lv_display_t* disp, const lv_area_t* area,
    uint8_t* px_map) {
  const int32_t area_width = area->x2 - area->x1 + 1;
  const auto* color_map = reinterpret_cast<const lv_color16_t*>(px_map);

  for (int32_t y = area->y1; y <= area->y2; y++) {
    if (y < 0 || y >= kDisplayHeight) {
      continue;
    }

    for (int32_t x = area->x1; x <= area->x2; x++) {
      if (x < 0 || x >= kDisplayWidth) {
        continue;
      }

      const size_t pixel_index =
          (y - area->y1) * area_width + (x - area->x1);
      const uint16_t epd_color =
          lv_color16_luminance(color_map[pixel_index]) < 128 ? EPD_BLACK
                                                             : EPD_WHITE;
      epd_display.drawPixel(x, y, epd_color);
    }
  }

  lv_display_flush_ready(disp);
}

void PrepareScreen() {
  epd_display.fillScreen(EPD_WHITE);
  epd_display.clearBuffer();

  lv_obj_t* screen = lv_screen_active();
  lv_obj_clean(screen);
  lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN);
}

const char* GetBatteryIcon(uint8_t percentage) {
  const uint8_t icon_index = percentage >= 100 ? 6 : percentage / 17;
  return kBatteryAndroidIcons[icon_index > 6 ? 6 : icon_index];
}

void RenderStatusBar() {
  lv_obj_t* screen = lv_screen_active();

  lv_obj_t* divider = lv_obj_create(screen);
  lv_obj_remove_style_all(divider);
  lv_obj_set_size(divider, kDisplayWidth, 1);
  lv_obj_set_style_bg_color(divider, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(divider, LV_ALIGN_TOP_LEFT, 0, kStatusBarHeight - 1);

  lv_obj_t* battery_percentage_label = lv_label_create(screen);
  lv_label_set_text_fmt(battery_percentage_label, "%u%%", battery_percentage);
  lv_obj_set_style_text_color(
      battery_percentage_label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      battery_percentage_label, &lvgl_font_google_sans_flex_regular_14,
      LV_PART_MAIN);
  lv_obj_align(battery_percentage_label, LV_ALIGN_TOP_RIGHT, -6, 1);

  lv_obj_t* battery_icon_label = lv_label_create(screen);
  lv_label_set_text(battery_icon_label, GetBatteryIcon(battery_percentage));
  lv_obj_set_style_text_color(
      battery_icon_label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      battery_icon_label, &lvgl_font_material_symbols_rounded_20,
      LV_PART_MAIN);
  lv_obj_align_to(battery_icon_label, battery_percentage_label,
      LV_ALIGN_OUT_LEFT_MID, -4, 0);

  if (sleep_mode) {
    lv_obj_t* sleep_icon_label = lv_label_create(screen);
    lv_label_set_text(sleep_icon_label, kBedtimeIcon);
    lv_obj_set_style_text_color(
        sleep_icon_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        sleep_icon_label, &lvgl_font_material_symbols_rounded_20,
        LV_PART_MAIN);
    lv_obj_align(sleep_icon_label, LV_ALIGN_TOP_LEFT, 6, 0);
  }
}

void RenderFooter(const char* page_name, bool page_selected) {
  lv_obj_t* screen = lv_screen_active();

  lv_obj_t* page_frame = lv_obj_create(screen);
  lv_obj_remove_style_all(page_frame);
  lv_obj_set_size(page_frame, 76, 17);
  lv_obj_set_style_radius(page_frame, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_border_width(page_frame, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(page_frame, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_color(page_frame, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(
      page_frame, page_selected ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_align(page_frame, LV_ALIGN_BOTTOM_MID, 0, -1);

  lv_obj_t* page_label = lv_label_create(screen);
  lv_label_set_text(page_label, page_name == nullptr ? "" : page_name);
  lv_obj_set_style_text_color(page_label,
      page_selected ? lv_color_white() : lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      page_label, &lvgl_font_google_sans_flex_regular_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(page_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_width(page_label, 72);
  lv_obj_align_to(page_label, page_frame, LV_ALIGN_CENTER, 0, 0);
}

void RenderScrollbar(size_t scroll_index, size_t line_count) {
  if (line_count <= static_cast<size_t>(kHomeVisibleLineCount)) {
    return;
  }

  constexpr int32_t kBarTop = kHomeContentTop;
  constexpr int32_t kBarHeight =
      kDisplayHeight - kHomeContentTop - kFooterHeight - 2;
  constexpr int32_t kTrackWidth = 1;
  constexpr int32_t kThumbWidth = 3;
  constexpr int32_t kBarX = kDisplayWidth - 5;

  lv_obj_t* track = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(track);
  lv_obj_set_size(track, kTrackWidth, kBarHeight);
  lv_obj_set_style_bg_color(track, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(track, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(track, LV_ALIGN_TOP_LEFT, kBarX, kBarTop);

  const size_t max_scroll = line_count - kHomeVisibleLineCount;
  const int32_t thumb_height =
      std::max<int32_t>(12, (kBarHeight * kHomeVisibleLineCount) / line_count);
  const int32_t thumb_y =
      kBarTop + ((kBarHeight - thumb_height) * scroll_index) / max_scroll;

  lv_obj_t* thumb = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(thumb);
  lv_obj_set_size(thumb, kThumbWidth, thumb_height);
  lv_obj_set_style_bg_color(thumb, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(thumb, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(thumb, LV_ALIGN_TOP_LEFT, kBarX - 1, thumb_y);
}

void RenderNow() {
  lv_obj_invalidate(lv_screen_active());
  lv_refr_now(lvgl_display);
}

void RefreshFull() {
  partial_refresh_base_map_ready = false;
  epd_display.display(Adafruit_EPD::Update_Mode::FULL_REFRESH, true);
}

void RefreshFast(bool busy_enable = true) {
  partial_refresh_base_map_ready = false;
  epd_display.display(
      Adafruit_EPD::Update_Mode::FAST_REFRESH, true, busy_enable);
}

void RefreshPartial() {
  if (!partial_refresh_base_map_ready) {
    epd_display.setRAMValueBaseMap(Adafruit_EPD::Update_Mode::FAST_REFRESH);
    partial_refresh_base_map_ready = true;
  }
  epd_display.display(Adafruit_EPD::Update_Mode::PARTIAL_REFRESH, true);
}

void RenderCenteredText(const char* text, const char* page_name = nullptr,
    bool page_selected = false) {
  PrepareScreen();
  RenderStatusBar();
  if (page_name != nullptr) {
    RenderFooter(page_name, page_selected);
  }

  lv_obj_t* label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(label, text);
  lv_obj_set_width(label, kDisplayWidth - 16);
  lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label, &lvgl_font_google_sans_flex_regular_13, LV_PART_MAIN);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  RenderNow();
}

void RenderTextList(
    const std::vector<std::string>& text_list, const char* page_name,
    bool page_selected) {
  PrepareScreen();
  RenderStatusBar();
  RenderFooter(page_name, page_selected);

  std::string display_text;
  for (const auto& text : text_list) {
    display_text += "[" + text + "]\n";
  }

  lv_obj_t* label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(label, display_text.c_str());
  lv_obj_set_height(
      label, kDisplayHeight - kKeyboardContentTop - kFooterHeight);
  lv_obj_set_width(label, kDisplayWidth - 12);
  lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label, &lvgl_font_google_sans_flex_regular_13, LV_PART_MAIN);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 6, kKeyboardContentTop);

  RenderNow();
}

void RenderHomeScreen(const std::vector<std::string>& lines,
    size_t scroll_index, const char* page_name, bool page_selected) {
  PrepareScreen();
  RenderStatusBar();
  RenderFooter(page_name, page_selected);

  if (scroll_index >= lines.size()) {
    scroll_index = lines.empty() ? 0 : lines.size() - 1;
  }

  const size_t visible_count =
      std::min(lines.size() - std::min(scroll_index, lines.size()),
          static_cast<size_t>(kHomeVisibleLineCount));
  for (size_t i = 0; i < visible_count; i++) {
    lv_obj_t* line_label = lv_label_create(lv_screen_active());
    lv_label_set_long_mode(line_label, LV_LABEL_LONG_CLIP);
    lv_label_set_text(line_label, lines[scroll_index + i].c_str());
    lv_obj_set_width(line_label, kDisplayWidth - 16);
    lv_obj_set_style_text_color(line_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        line_label, &lvgl_font_google_sans_flex_regular_13, LV_PART_MAIN);
    lv_obj_align(line_label, LV_ALIGN_TOP_LEFT, 6,
        kHomeContentTop + static_cast<int32_t>(i) * kHomeLineHeight);
  }

  RenderScrollbar(scroll_index, lines.size());

  RenderNow();
}

void RenderAudioIconBox(
    int32_t x, const char* icon, const char* label, bool selected) {
  constexpr int32_t kBoxSize = 56;
  constexpr int32_t kIconY = 36;

  lv_obj_t* select_frame = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(select_frame);
  lv_obj_set_size(select_frame, kBoxSize + 8, kBoxSize + 8);
  lv_obj_set_style_radius(select_frame, 12, LV_PART_MAIN);
  lv_obj_set_style_border_width(select_frame, selected ? 2 : 0, LV_PART_MAIN);
  lv_obj_set_style_border_color(select_frame, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(select_frame, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_align(select_frame, LV_ALIGN_TOP_LEFT, x - 4, kIconY - 4);

  lv_obj_t* box = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(box);
  lv_obj_set_size(box, kBoxSize, kBoxSize);
  lv_obj_set_style_radius(box, 10, LV_PART_MAIN);
  lv_obj_set_style_border_width(box, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(box, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_color(box, selected ? lv_color_black() : lv_color_white(),
      LV_PART_MAIN);
  lv_obj_set_style_bg_opa(box, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(box, LV_ALIGN_TOP_LEFT, x, kIconY);

  lv_obj_t* icon_label = lv_label_create(lv_screen_active());
  lv_label_set_text(icon_label, icon);
  lv_obj_set_size(icon_label, kBoxSize, kBoxSize);
  lv_obj_set_style_text_color(icon_label,
      selected ? lv_color_white() : lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      icon_label, &lvgl_font_material_symbols_rounded_32, LV_PART_MAIN);
  lv_obj_set_style_text_align(icon_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_pad_top(icon_label, 14, LV_PART_MAIN);
  lv_obj_align_to(icon_label, box, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* text_label = lv_label_create(lv_screen_active());
  lv_label_set_text(text_label, label);
  lv_obj_set_style_text_color(text_label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      text_label, &lvgl_font_google_sans_flex_regular_13, LV_PART_MAIN);
  lv_obj_set_width(text_label, kBoxSize + 8);
  lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(text_label, box, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
}

void RenderAudioScreen(bool page_selected, bool mic_selected,
    const char* message, const char* page_name) {
  PrepareScreen();
  RenderStatusBar();
  RenderFooter(page_name, page_selected);

  const bool show_icon_selection = page_selected;
  RenderAudioIconBox(30, kMicIcon, "Mic", show_icon_selection && mic_selected);
  RenderAudioIconBox(
      106, kSpeakerIcon, "Speaker", show_icon_selection && !mic_selected);

  lv_obj_t* message_label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(message_label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(message_label, message == nullptr ? "" : message);
  lv_obj_set_width(message_label, kDisplayWidth - 16);
  lv_obj_set_style_text_color(message_label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      message_label, &lvgl_font_google_sans_flex_regular_13, LV_PART_MAIN);
  lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(message_label, LV_ALIGN_TOP_MID, 0, 130);

  RenderNow();
}

}  // namespace

void Init() {
  BeginDisplay();

  lv_init();
  lvgl_display = lv_display_create(kDisplayWidth, kDisplayHeight);
  lv_display_set_default(lvgl_display);
  lv_display_set_color_format(lvgl_display, LV_COLOR_FORMAT_RGB565);
  lv_display_set_buffers(lvgl_display, lvgl_draw_buffer, nullptr,
      sizeof(lvgl_draw_buffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(lvgl_display, LvglFlushCallback);
}

void BeginDisplay() {
  epd_display.begin();
  epd_display.setRotation(1);
  ResetPartialRefresh();
}

void EndDisplay() { epd_display.end(); }

void Tick(uint32_t elapsed_ms) { lv_tick_inc(elapsed_ms); }

void ResetPartialRefresh() { partial_refresh_base_map_ready = false; }

void SetBatteryPercentage(uint8_t percentage) {
  battery_percentage = percentage > 100 ? 100 : percentage;
}

void SetSleepMode(bool enable) { sleep_mode = enable; }

void ShowBootScreen() {
  PrepareScreen();

  lv_obj_t* logo = lv_label_create(lv_screen_active());
  lv_label_set_text(logo, "LILYGO");
  lv_obj_set_style_text_color(logo, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      logo, &lvgl_font_google_sans_flex_regular_28, LV_PART_MAIN);
  lv_obj_set_style_text_align(logo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(logo, LV_ALIGN_CENTER, 0, -18);

  RenderNow();
  RefreshFull();
}

void ShowHomeScreen(const std::vector<std::string>& lines, size_t scroll_index,
    const char* page_name, bool page_selected, bool busy_enable) {
  RenderHomeScreen(lines, scroll_index, page_name, page_selected);
  RefreshFast(busy_enable);
}

void ShowCenteredText(const char* text) {
  RenderCenteredText(text);
  RefreshFast();
}

void ShowTextList(const std::vector<std::string>& text_list,
    const char* page_name, bool page_selected, bool partial_refresh,
    bool busy_enable) {
  if (text_list.empty()) {
    RenderCenteredText("Please enter the text", page_name, page_selected);
    RefreshFast(busy_enable);
    return;
  }

  RenderTextList(text_list, page_name, page_selected);
  if (partial_refresh) {
    RefreshPartial();
  } else {
    RefreshFast(busy_enable);
  }
}

void ShowAudioScreen(bool page_selected, bool mic_selected, const char* message,
    const char* page_name, bool busy_enable) {
  RenderAudioScreen(page_selected, mic_selected, message, page_name);
  RefreshFast(busy_enable);
}

}  // namespace lvgl_port
