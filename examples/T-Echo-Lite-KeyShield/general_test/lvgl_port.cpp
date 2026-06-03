/**
 * @file lvgl_port.cpp
 * @brief general_test LVGL display port.
 */
#include "lvgl_port.h"

#include <Arduino.h>

#include "Adafruit_EPD.h"
#include "lvgl.h"
#include "t_echo_lite_keyshield_config.h"

LV_FONT_DECLARE(google_sans_flex_14)
LV_FONT_DECLARE(google_sans_flex_28)

namespace lvgl_port {
namespace {

constexpr int32_t kDisplayWidth = SCREEN_HEIGHT;
constexpr int32_t kDisplayHeight = SCREEN_WIDTH;
constexpr uint16_t kDrawBufferRows = 8;
constexpr size_t kDrawBufferSize =
    kDisplayWidth * kDrawBufferRows * sizeof(lv_color16_t);

SPIClass custom_spi1(NRF_SPIM1, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 epd_display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC,
    SCREEN_RST, SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &custom_spi1, 8000000);

lv_display_t* lvgl_display = nullptr;
alignas(LV_DRAW_BUF_ALIGN) uint8_t lvgl_draw_buffer[kDrawBufferSize] = {0};
bool partial_refresh_base_map_ready = false;

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

void RenderCenteredText(const char* text) {
  PrepareScreen();

  lv_obj_t* label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(label, text);
  lv_obj_set_width(label, kDisplayWidth - 16);
  lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, &google_sans_flex_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  RenderNow();
}

void RenderTextList(const std::vector<std::string>& text_list) {
  PrepareScreen();

  std::string display_text;
  for (const auto& text : text_list) {
    display_text += "[" + text + "]\n";
  }

  lv_obj_t* label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(label, display_text.c_str());
  lv_obj_set_width(label, kDisplayWidth - 12);
  lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, &google_sans_flex_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 6, 8);

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

void ShowBootScreen(const char* build_time) {
  PrepareScreen();

  lv_obj_t* logo = lv_label_create(lv_screen_active());
  lv_label_set_text(logo, "LILYGO");
  lv_obj_set_style_text_color(logo, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(logo, &google_sans_flex_28, LV_PART_MAIN);
  lv_obj_set_style_text_align(logo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(logo, LV_ALIGN_CENTER, 0, -18);

  lv_obj_t* build_time_label = lv_label_create(lv_screen_active());
  lv_label_set_text(build_time_label, build_time);
  lv_obj_set_style_text_color(build_time_label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      build_time_label, &google_sans_flex_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(
      build_time_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(build_time_label, logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

  RenderNow();
  RefreshFull();
}

void ShowCenteredText(const char* text) {
  RenderCenteredText(text);
  RefreshFast();
}

void ShowTextList(const std::vector<std::string>& text_list,
    bool partial_refresh) {
  if (text_list.empty()) {
    RenderCenteredText("Please enter the text");
    RefreshFast();
    return;
  }

  RenderTextList(text_list);
  if (partial_refresh) {
    RefreshPartial();
  } else {
    RefreshFast(false);
  }
}

}  // namespace lvgl_port
