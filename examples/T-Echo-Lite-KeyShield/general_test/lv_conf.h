/**
 * @file lv_conf.h
 * @brief general_test LVGL minimal configuration.
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16

#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

#define LV_USE_OS LV_OS_NONE
#define LV_USE_LOG 0

#define LV_USE_DRAW_SW 1
#define LV_USE_DRAW_SW_COMPLEX_GRADIENTS 0

#define LV_USE_ANIMATION 0
#define LV_USE_FLEX 1
#define LV_USE_GRID 0

#define LV_FONT_MONTSERRAT_14 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_CUSTOM_DECLARE \
  LV_FONT_DECLARE(lvgl_font_google_sans_flex_regular_13) \
  LV_FONT_DECLARE(lvgl_font_google_sans_flex_regular_14) \
  LV_FONT_DECLARE(lvgl_font_google_sans_flex_regular_28) \
  LV_FONT_DECLARE(lvgl_font_material_symbols_rounded_20) \
  LV_FONT_DECLARE(lvgl_font_material_symbols_rounded_32)
#define LV_FONT_DEFAULT &lvgl_font_google_sans_flex_regular_14
#define LV_USE_LABEL 1

#endif  // LV_CONF_H
