/*******************************************************************************
 * Size: 32 px
 * Bpp: 1
 * Opts: --font assets\icon\material_symbols\MaterialSymbolsRounded.ttf -r 58141,58157 --size 32 --bpp 1 --format lvgl --lv-include lvgl.h --lv-font-name lvgl_font_material_symbols_rounded_32 --no-compress -o examples\T-Echo-Lite-KeyShield\general_test\lvgl_font_material_symbols_rounded_32.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef LVGL_FONT_MATERIAL_SYMBOLS_ROUNDED_32
#define LVGL_FONT_MATERIAL_SYMBOLS_ROUNDED_32 1
#endif

#if LVGL_FONT_MATERIAL_SYMBOLS_ROUNDED_32

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+E31D "" */
    0x1, 0xe0, 0x0, 0xfe, 0x0, 0x73, 0x80, 0x1c,
    0xe0, 0x7, 0x38, 0x1, 0xce, 0x0, 0x73, 0x80,
    0x1c, 0xe0, 0x7, 0x38, 0x1, 0xce, 0x0, 0x73,
    0x80, 0x1c, 0xe0, 0xe7, 0x38, 0xf8, 0xfe, 0x7e,
    0x1e, 0x1d, 0xc0, 0x7, 0x70, 0x3, 0x8e, 0x1,
    0xc1, 0xe0, 0xf0, 0x3f, 0xf8, 0x7, 0xf8, 0x0,
    0x30, 0x0, 0xc, 0x0, 0x3, 0x0, 0x0, 0xc0,
    0x0,

    /* U+E32D "" */
    0x7f, 0xff, 0xe7, 0xff, 0xff, 0xf0, 0x0, 0x7f,
    0x2, 0x7, 0xf0, 0xf8, 0x7f, 0xf, 0x87, 0xf0,
    0xf8, 0x7f, 0xf, 0x87, 0xf0, 0x70, 0x7f, 0x0,
    0x7, 0xf0, 0x0, 0x7f, 0xf, 0x87, 0xf1, 0xfc,
    0x7f, 0x3f, 0xe7, 0xf3, 0xfe, 0x7f, 0x78, 0xf7,
    0xf7, 0x7, 0x7f, 0x70, 0x77, 0xf3, 0x6, 0x7f,
    0x38, 0xe7, 0xf3, 0xfe, 0x7f, 0x1f, 0xc7, 0xf0,
    0x20, 0x7f, 0x0, 0x7, 0x7f, 0xff, 0xf7, 0xff,
    0xfe
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 512, .box_w = 18, .box_h = 25, .ofs_x = 7, .ofs_y = 4},
    {.bitmap_index = 57, .adv_w = 512, .box_w = 20, .box_h = 26, .ofs_x = 6, .ofs_y = 3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x10
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 58141, .range_length = 17, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 2, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lvgl_font_material_symbols_rounded_32 = {
#else
lv_font_t lvgl_font_material_symbols_rounded_32 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 26,          /*The maximum line height required by the font*/
    .base_line = -3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 0,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LVGL_FONT_MATERIAL_SYMBOLS_ROUNDED_32*/

