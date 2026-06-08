/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-05 09:38:40
 * @License: GPL 3.0
 */
#include "lvgl_port.h"

#include <Arduino.h>

#include <atomic>
#include <algorithm>

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
constexpr uint16_t kMaxPartialRefreshCount = 10;
constexpr uint16_t kDrawBufferRows = 8;
constexpr size_t kDrawBufferSize =
    kDisplayWidth * kDrawBufferRows * sizeof(lv_color16_t);

constexpr const char* kBatteryAndroidIcons[] = {"\xEF\x8C\x8D", "\xEF\x8C\x8C",
    "\xEF\x8C\x8B", "\xEF\x8C\x8A", "\xEF\x8C\x89", "\xEF\x8C\x88",
    "\xEF\x8C\x87"};
constexpr const char* kBluetoothIcon = "\xEE\x86\xA7";
constexpr const char* kBluetoothConnectedIcon = "\xEE\x86\xA8";
constexpr const char* kBedtimeIcon = "\xEF\x85\x99";
constexpr const char* kMicIcon = "\xEE\x8C\x9D";
constexpr const char* kSpeakerIcon = "\xEE\x8C\xAD";

SPIClass custom_spi1(NRF_SPIM1, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 epd_display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC, SCREEN_RST,
    SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &custom_spi1, 8000000);

lv_display_t* lvgl_display = nullptr;
alignas(LV_DRAW_BUF_ALIGN) uint8_t lvgl_draw_buffer[kDrawBufferSize] = {0};
bool partial_refresh_base_map_ready = false;
uint16_t partial_refresh_count = 0;
uint8_t battery_percentage = 0;
bool ble_connected = false;
bool sleep_mode = false;
std::atomic<bool> display_busy{false};

/**
 * @brief 将 LVGL 刷新的像素写入电子纸帧缓冲区。
 * @param disp 正在刷新的 LVGL 显示对象。
 * @param area LVGL 提供的像素区域。
 * @param px_map 该区域的 RGB565 像素数据。
 */
void LvglFlushCallback(
    lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
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

      const size_t pixel_index = (y - area->y1) * area_width + (x - area->x1);
      const uint16_t epd_color =
          lv_color16_luminance(color_map[pixel_index]) < 128 ? EPD_BLACK
                                                             : EPD_WHITE;
      epd_display.drawPixel(x, y, epd_color);
    }
  }

  lv_display_flush_ready(disp);
}

/**
 * @brief 渲染页面前清空电子纸和 LVGL 屏幕对象。
 */
void PrepareScreen() {
  epd_display.fillScreen(EPD_WHITE);
  epd_display.clearBuffer();

  lv_obj_t* screen = lv_screen_active();
  lv_obj_clean(screen);
  lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN);
}

/**
 * @brief 根据电量百分比获取 Material Symbols 电池图标。
 * @param percentage 0 到 100 的电量百分比。
 * @return 状态栏标签使用的 UTF-8 图标文本。
 */
const char* GetBatteryIcon(uint8_t percentage) {
  const uint8_t icon_index = percentage >= 100 ? 6 : percentage / 17;
  return kBatteryAndroidIcons[icon_index > 6 ? 6 : icon_index];
}

/**
 * @brief 渲染包含电量和休眠状态的状态栏。
 */
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
  lv_obj_set_style_text_font(battery_percentage_label,
      &lvgl_font_google_sans_flex_regular_14, LV_PART_MAIN);
  lv_obj_align(battery_percentage_label, LV_ALIGN_TOP_RIGHT, -6, 1);

  lv_obj_t* battery_icon_label = lv_label_create(screen);
  lv_label_set_text(battery_icon_label, GetBatteryIcon(battery_percentage));
  lv_obj_set_style_text_color(
      battery_icon_label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      battery_icon_label, &lvgl_font_material_symbols_rounded_20, LV_PART_MAIN);
  lv_obj_align_to(battery_icon_label, battery_percentage_label,
      LV_ALIGN_OUT_LEFT_MID, -4, 0);

  lv_obj_t* bluetooth_icon_label = lv_label_create(screen);
  lv_label_set_text(bluetooth_icon_label,
      ble_connected ? kBluetoothConnectedIcon : kBluetoothIcon);
  lv_obj_set_style_text_color(
      bluetooth_icon_label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(bluetooth_icon_label,
      &lvgl_font_material_symbols_rounded_20, LV_PART_MAIN);
  lv_obj_align_to(
      bluetooth_icon_label, battery_icon_label, LV_ALIGN_OUT_LEFT_MID, -4, 0);

  if (sleep_mode) {
    lv_obj_t* sleep_icon_label = lv_label_create(screen);
    lv_label_set_text(sleep_icon_label, kBedtimeIcon);
    lv_obj_set_style_text_color(
        sleep_icon_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        sleep_icon_label, &lvgl_font_material_symbols_rounded_20, LV_PART_MAIN);
    lv_obj_align(sleep_icon_label, LV_ALIGN_TOP_LEFT, 6, 0);
  }
}

/**
 * @brief 渲染页面底部页签。
 * @param page_name 底部显示的页面名称。
 * @param page_selected 页面被选中时为 true。
 */
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

/**
 * @brief 主页内容超出可见区域时渲染滚动条。
 * @param scroll_index 第一条可见文本行的索引。
 * @param line_count 主页文本总行数。
 */
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

/**
 * @brief 强制 LVGL 将当前活动屏幕渲染到帧缓冲区。
 */
void RenderNow() {
  lv_obj_invalidate(lv_screen_active());
  lv_refr_now(lvgl_display);
}

/**
 * @brief 执行电子纸全局刷新并重置局刷基础底图状态。
 */
void RefreshFull() {
  while (display_busy.exchange(true)) { delay(1); }
  partial_refresh_base_map_ready = false;
  partial_refresh_count = 0;
  epd_display.display(Adafruit_EPD::Update_Mode::FULL_REFRESH, true);
  display_busy = false;
}

/**
 * @brief 执行电子纸全屏快刷并重置局刷基础底图状态。
 */
void RefreshFastCleanup() {
  partial_refresh_base_map_ready = false;
  partial_refresh_count = 0;
  epd_display.display(Adafruit_EPD::Update_Mode::FAST_REFRESH, true);
}

/**
 * @brief 必要时先准备基础底图，然后执行电子纸局部刷新。
 */
void RefreshPartial() {
  if (!partial_refresh_base_map_ready) {
    epd_display.setRAMValueBaseMap(Adafruit_EPD::Update_Mode::FAST_REFRESH);
    partial_refresh_base_map_ready = true;
  }
  epd_display.display(Adafruit_EPD::Update_Mode::PARTIAL_REFRESH, true);
  partial_refresh_count++;
}

/**
 * @brief 根据局部刷新次数选择局刷或清残影快刷（互斥保护）。
 */
void RefreshDisplay() {
  while (display_busy.exchange(true)) { delay(1); }
  if (partial_refresh_count >= kMaxPartialRefreshCount) {
    RefreshFastCleanup();
  }
  RefreshPartial();
  display_busy = false;
}

/**
 * @brief 在空白页面上渲染居中文本。
 * @param text 屏幕中央显示的文本。
 * @param page_name 可选的底部页面名称。
 * @param page_selected 页面被选中时为 true。
 */
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

/**
 * @brief 渲染带方括号的键盘输入文本列表。
 * @param text_list 列表中显示的文本项。
 * @param page_name 底部显示的页面名称。
 * @param page_selected 页面被选中时为 true。
 */
void RenderTextList(const std::vector<std::string>& text_list,
    const char* page_name, bool page_selected) {
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

/**
 * @brief 渲染可滚动主页。
 * @param lines 主页文本行。
 * @param scroll_index 第一条可见文本行的索引。
 * @param page_name 底部显示的页面名称。
 * @param page_selected 页面被选中时为 true。
 */
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

/**
 * @brief 渲染 SX1262 页面上的普通文本标签。
 * @param text 标签文本。
 * @param x 标签左侧坐标。
 * @param y 标签顶部坐标。
 * @param width 标签宽度。
 */
void RenderSx1262TextLabel(
    const char* text, int32_t x, int32_t y, int32_t width) {
  lv_obj_t* label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
  lv_label_set_text(label, text == nullptr ? "" : text);
  lv_obj_set_width(label, width);
  lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label, &lvgl_font_google_sans_flex_regular_13, LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, x, y);
}

/**
 * @brief 渲染 SX1262 页面控件外侧的选定框。
 * @param x 选定框左侧坐标。
 * @param y 选定框顶部坐标。
 * @param width 选定框宽度。
 * @param height 选定框高度。
 * @param selected 控件被选中时为 true。
 */
void RenderSx1262SelectionFrame(
    int32_t x, int32_t y, int32_t width, int32_t height, bool selected) {
  if (!selected) {
    return;
  }

  lv_obj_t* frame = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(frame);
  lv_obj_set_size(frame, width, height);
  lv_obj_set_style_radius(frame, 4, LV_PART_MAIN);
  lv_obj_set_style_border_width(frame, 2, LV_PART_MAIN);
  lv_obj_set_style_border_color(frame, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(frame, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_align(frame, LV_ALIGN_TOP_LEFT, x, y);
}

/**
 * @brief 渲染 SX1262 频率输入框。
 * @param text 输入框显示文本。
 * @param selected 输入框被选中时为 true。
 * @param editing 输入框处于编辑反色状态时为 true。
 */
void RenderSx1262FrequencyInput(const char* text, bool selected, bool editing) {
  constexpr int32_t kInputX = 90;
  constexpr int32_t kInputY = 36;
  constexpr int32_t kInputWidth = 46;
  constexpr int32_t kInputHeight = 20;

  RenderSx1262SelectionFrame(
      kInputX - 3, kInputY - 3, kInputWidth + 6, kInputHeight + 6, selected);

  lv_obj_t* input = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(input);
  lv_obj_set_size(input, kInputWidth, kInputHeight);
  lv_obj_set_style_radius(input, 2, LV_PART_MAIN);
  lv_obj_set_style_border_width(input, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(input, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_color(
      input, editing ? lv_color_black() : lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(input, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_clear_flag(input, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(input, LV_ALIGN_TOP_LEFT, kInputX, kInputY);

  lv_obj_t* value_label = lv_label_create(input);
  lv_label_set_long_mode(value_label, LV_LABEL_LONG_CLIP);
  lv_label_set_text(value_label, text == nullptr ? "" : text);
  lv_obj_set_width(value_label, kInputWidth - 4);
  lv_obj_set_style_text_color(
      value_label, editing ? lv_color_white() : lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      value_label, &lvgl_font_google_sans_flex_regular_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(value_label, LV_ALIGN_CENTER, 0, 0);
}

/**
 * @brief 渲染 SX1262 带宽输入框。
 * @param text 输入框显示文本。
 * @param selected 输入框被选中时为 true。
 * @param editing 输入框处于编辑反色状态时为 true。
 */
void RenderSx1262BandwidthInput(const char* text, bool selected, bool editing) {
  constexpr int32_t kInputX = 90;
  constexpr int32_t kInputY = 62;
  constexpr int32_t kInputWidth = 46;
  constexpr int32_t kInputHeight = 20;

  RenderSx1262SelectionFrame(
      kInputX - 3, kInputY - 3, kInputWidth + 6, kInputHeight + 6, selected);

  lv_obj_t* input = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(input);
  lv_obj_set_size(input, kInputWidth, kInputHeight);
  lv_obj_set_style_radius(input, 2, LV_PART_MAIN);
  lv_obj_set_style_border_width(input, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(input, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_color(
      input, editing ? lv_color_black() : lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(input, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_clear_flag(input, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(input, LV_ALIGN_TOP_LEFT, kInputX, kInputY);

  lv_obj_t* value_label = lv_label_create(input);
  lv_label_set_long_mode(value_label, LV_LABEL_LONG_CLIP);
  lv_label_set_text(value_label, text == nullptr ? "" : text);
  lv_obj_set_width(value_label, kInputWidth - 4);
  lv_obj_set_style_text_color(
      value_label, editing ? lv_color_white() : lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      value_label, &lvgl_font_google_sans_flex_regular_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(value_label, LV_ALIGN_CENTER, 0, 0);
}

/**
 * @brief 渲染 SX1262 自动发送开关。
 * @param enabled 自动发送打开时为 true。
 * @param selected 开关被选中时为 true。
 */
void RenderSx1262AutoSendSwitch(bool enabled, bool selected) {
  constexpr int32_t kSwitchX = 90;
  constexpr int32_t kSwitchY = 88;
  constexpr int32_t kSwitchWidth = 34;
  constexpr int32_t kSwitchHeight = 18;

  RenderSx1262SelectionFrame(kSwitchX - 3, kSwitchY - 3, kSwitchWidth + 6,
      kSwitchHeight + 6, selected);

  lv_obj_t* switch_obj = lv_switch_create(lv_screen_active());
  lv_obj_set_size(switch_obj, kSwitchWidth, kSwitchHeight);
  lv_obj_set_style_radius(switch_obj, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_border_width(switch_obj, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(switch_obj, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_color(switch_obj, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(switch_obj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(switch_obj, lv_color_black(), LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(switch_obj, LV_OPA_TRANSP, LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(
      switch_obj, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_bg_color(switch_obj, lv_color_white(), LV_PART_KNOB);
  lv_obj_set_style_bg_opa(switch_obj, LV_OPA_COVER, LV_PART_KNOB);
  lv_obj_set_style_border_width(switch_obj, 1, LV_PART_KNOB);
  lv_obj_set_style_border_color(switch_obj, lv_color_black(), LV_PART_KNOB);
  if (enabled) {
    lv_obj_add_state(switch_obj, LV_STATE_CHECKED);
  }
  lv_obj_align(switch_obj, LV_ALIGN_TOP_LEFT, kSwitchX, kSwitchY);
}

/**
 * @brief 渲染 SX1262 LoRa 页面。
 * @param state SX1262 LoRa 页面显示状态。
 * @param page_name 底部显示的页面名称。
 */
void RenderSx1262LoraScreen(
    const Sx1262LoraScreenState& state, const char* page_name) {
  PrepareScreen();
  RenderStatusBar();
  RenderFooter(page_name, state.page_selected);

  RenderSx1262TextLabel("[sx1262 lora]", 6, 23, kDisplayWidth - 12);
  RenderSx1262TextLabel("frequency:", 6, 40, 80);
  RenderSx1262FrequencyInput(state.frequency_text,
      state.page_selected && state.frequency_selected,
      state.page_selected && state.frequency_editing);
  RenderSx1262TextLabel("mhz", 144, 40, 28);

  RenderSx1262TextLabel("bandwidth:", 6, 66, 80);
  RenderSx1262BandwidthInput(state.bandwidth_text,
      state.page_selected && state.bandwidth_selected,
      state.page_selected && state.bandwidth_editing);
  RenderSx1262TextLabel("khz", 144, 66, 28);

  RenderSx1262TextLabel("auto send:", 6, 92, 80);
  RenderSx1262AutoSendSwitch(
      state.auto_send_enabled, state.page_selected && state.auto_send_selected);
  RenderSx1262TextLabel(state.auto_send_enabled ? "on" : "off", 144, 92, 24);

  RenderSx1262TextLabel("[rx]", 6, 114, 40);
  RenderSx1262TextLabel("data:", 6, 129, 35);
  RenderSx1262TextLabel(state.rx_data, 43, 129, kDisplayWidth - 49);
  RenderSx1262TextLabel(state.rssi_text, 6, 141, 84);
  RenderSx1262TextLabel(state.snr_text, 96, 141, 84);

  RenderNow();
}

/**
 * @brief 渲染音频页面上的一个可选图标框。
 * @param x 图标框左侧坐标。
 * @param icon Material Symbols 图标文本。
 * @param label 图标下方的文本标签。
 * @param selected 图标框被选中时为 true。
 */
void RenderAudioIconBox(int32_t x, const char* icon, const char* label,
    bool selected, bool action_running) {
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
  const bool highlight = selected && action_running;
  lv_obj_set_style_bg_color(
      box, highlight ? lv_color_black() : lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(box, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(box, LV_ALIGN_TOP_LEFT, x, kIconY);

  lv_obj_t* icon_label = lv_label_create(lv_screen_active());
  lv_label_set_text(icon_label, icon);
  lv_obj_set_size(icon_label, kBoxSize, kBoxSize);
  lv_obj_set_style_text_color(icon_label,
      highlight ? lv_color_white() : lv_color_black(), LV_PART_MAIN);
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
  lv_obj_align_to(text_label, box, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
}

/**
 * @brief 渲染音频测试页面。
 * @param page_selected 页面被选中时为 true。
 * @param mic_selected 麦克风操作被选中时为 true。
 * @param message 页面显示的状态文本。
 * @param page_name 底部显示的页面名称。
 */
void RenderAudioScreen(bool page_selected, bool mic_selected,
    const char* message, const char* page_name, bool action_running) {
  PrepareScreen();
  RenderStatusBar();
  RenderFooter(page_name, page_selected);

  const bool show_icon_selection = page_selected;
  RenderAudioIconBox(
      30, kMicIcon, "Mic", show_icon_selection && mic_selected, action_running);
  RenderAudioIconBox(106, kSpeakerIcon, "Speaker",
      show_icon_selection && !mic_selected, action_running);

  lv_obj_t* message_label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(message_label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(message_label, message == nullptr ? "" : message);
  lv_obj_set_width(message_label, kDisplayWidth - 16);
  lv_obj_set_style_text_color(message_label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      message_label, &lvgl_font_google_sans_flex_regular_13, LV_PART_MAIN);
  lv_obj_set_style_text_align(
      message_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
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

void ResetPartialRefresh() {
  partial_refresh_base_map_ready = false;
  partial_refresh_count = 0;
}

void SetBatteryPercentage(uint8_t percentage) {
  battery_percentage = percentage > 100 ? 100 : percentage;
}

void SetBleConnected(bool connected) { ble_connected = connected; }

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
  RefreshDisplay();
}

void ShowHomeScreen(const std::vector<std::string>& lines, size_t scroll_index,
    const char* page_name, bool page_selected, bool partial_refresh,
    bool busy_enable) {
  RenderHomeScreen(lines, scroll_index, page_name, page_selected);
  RefreshDisplay();
}

void ShowCenteredText(const char* text) {
  RenderCenteredText(text);
  RefreshDisplay();
}

void ShowTextList(const std::vector<std::string>& text_list,
    const char* page_name, bool page_selected, bool partial_refresh,
    bool busy_enable) {
  if (text_list.empty()) {
    RenderCenteredText("Please enter the text", page_name, page_selected);
    RefreshDisplay();
    return;
  }

  RenderTextList(text_list, page_name, page_selected);
  RefreshDisplay();
}

void ShowAudioScreen(bool page_selected, bool mic_selected, const char* message,
    const char* page_name, bool partial_refresh, bool busy_enable,
    bool action_running) {
  RenderAudioScreen(
      page_selected, mic_selected, message, page_name, action_running);
  RefreshDisplay();
}

void ShowSx1262LoraScreen(const Sx1262LoraScreenState& state,
    const char* page_name, bool partial_refresh, bool busy_enable) {
  RenderSx1262LoraScreen(state, page_name);
  RefreshDisplay();
}

/**
 * @brief 渲染 GPS 测试页面。
 */
void RenderGpsScreen(const GpsScreenState& state, const char* page_name) {
  PrepareScreen();
  RenderStatusBar();
  RenderFooter(page_name, state.page_selected);

  constexpr int32_t kLineHeight = 13;
  int32_t y = kStatusBarHeight + 4;

  auto AddLine = [&y](const char* text, int32_t x = 6) {
    lv_obj_t* label = lv_label_create(lv_screen_active());
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_label_set_text(label, text == nullptr ? "" : text);
    lv_obj_set_width(label, kDisplayWidth - x - 6);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        label, &lvgl_font_google_sans_flex_regular_13, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, x, y);
    y += kLineHeight;
  };

  AddLine("[gps]");

  if (state.fix_time_text != nullptr) {
    AddLine(state.fix_time_text);
  }

  y += 2;

  if (!state.module_found) {
    AddLine("GPS module not found");
  } else if (!state.has_fix) {
    AddLine("Waiting for fix...");
    if (state.satellites_text != nullptr) {
      AddLine(state.satellites_text);
    }
    if (state.time_text != nullptr) {
      AddLine(state.time_text);
    }
  } else {
    if (state.latitude_text != nullptr) {
      AddLine(state.latitude_text);
    }
    if (state.longitude_text != nullptr) {
      AddLine(state.longitude_text);
    }
    if (state.satellites_text != nullptr) {
      AddLine(state.satellites_text);
    }
    if (state.cn0_text != nullptr) {
      AddLine(state.cn0_text);
    }
    if (state.dop_text != nullptr) {
      AddLine(state.dop_text);
    }
    if (state.speed_text != nullptr) {
      AddLine(state.speed_text);
    }
    if (state.time_text != nullptr) {
      AddLine(state.time_text);
    }
  }

  RenderNow();
}

void ShowGpsScreen(const GpsScreenState& state, const char* page_name,
    bool partial_refresh, bool busy_enable) {
  RenderGpsScreen(state, page_name);
  RefreshDisplay();
}

void RenderImuScreen(const ImuScreenState& state, const char* page_name) {
  PrepareScreen();
  RenderStatusBar();
  RenderFooter(page_name, state.page_selected);

  constexpr int32_t kLineHeight = 13;
  int32_t y = kStatusBarHeight + 4;

  auto AddLine = [&y](const char* text) {
    lv_obj_t* label = lv_label_create(lv_screen_active());
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_label_set_text(label, text == nullptr ? "" : text);
    lv_obj_set_width(label, kDisplayWidth - 12);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        label, &lvgl_font_google_sans_flex_regular_13, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 6, y);
    y += kLineHeight;
  };

  AddLine("[imu]");
  y += 2;

  if (!state.module_found) {
    AddLine("IMU module not found");
  } else {
    if (state.pitch_text != nullptr) AddLine(state.pitch_text);
    if (state.roll_text != nullptr) AddLine(state.roll_text);
    if (state.yaw_text != nullptr) AddLine(state.yaw_text);
    if (state.temp_text != nullptr) AddLine(state.temp_text);
    AddLine("");
    AddLine("Press Center to refresh");
  }

  RenderNow();
}

void ShowImuScreen(const ImuScreenState& state, const char* page_name,
    bool partial_refresh, bool busy_enable) {
  RenderImuScreen(state, page_name);
  RefreshDisplay();
}

}  // namespace lvgl_port
