/*******************************************************************************
 * Size: 20 px
 * Bpp: 1
 * Opts: --font assets\icon\material_symbols\MaterialSymbolsRounded.ttf -r 61785,0xF307-0xF30D --size 20 --bpp 1 --format lvgl --lv-include lvgl.h --lv-font-name lvgl_font_material_symbols_rounded_20 --no-compress -o examples\T-Echo-Lite-KeyShield\general_test\lvgl_font_material_symbols_rounded_20.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef LVGL_FONT_MATERIAL_SYMBOLS_ROUNDED_20
#define LVGL_FONT_MATERIAL_SYMBOLS_ROUNDED_20 1
#endif

#if LVGL_FONT_MATERIAL_SYMBOLS_ROUNDED_20

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+F159 "" */
    0x6, 0x0, 0xf, 0x0, 0xf, 0x80, 0xe, 0x40,
    0x6, 0x30, 0x7, 0x18, 0x3, 0x6, 0x1, 0x83,
    0x80, 0xc0, 0xe0, 0x60, 0x3c, 0x30, 0x7, 0xec,
    0x0, 0x76, 0x0, 0x31, 0x80, 0x30, 0x30, 0x70,
    0x7, 0xe0,

    /* U+F307 "" */
    0x7f, 0xff, 0x1f, 0xff, 0x33, 0xff, 0xe6, 0x7f,
    0xfc, 0xdf, 0xff, 0x9b, 0xff, 0xf3, 0x7f, 0xfe,
    0x6f, 0xff, 0xcc, 0xff, 0xf9, 0x8f, 0xff, 0xe0,

    /* U+F308 "" */
    0x7f, 0xfe, 0x3f, 0xf8, 0xcf, 0xfe, 0x33, 0xff,
    0x8d, 0xff, 0xe3, 0x7f, 0xf8, 0xdf, 0xfe, 0x37,
    0xff, 0x8c, 0xff, 0xe3, 0x1f, 0xff, 0x80,

    /* U+F309 "" */
    0x7f, 0xfe, 0x3f, 0xe0, 0xcf, 0xf8, 0x33, 0xfe,
    0xd, 0xff, 0x83, 0x7f, 0xe0, 0xdf, 0xf8, 0x37,
    0xfe, 0xc, 0xff, 0x83, 0x1f, 0xff, 0x80,

    /* U+F30A "" */
    0x7f, 0xff, 0x1f, 0xe0, 0x33, 0xfc, 0x6, 0x7f,
    0x80, 0xdf, 0xf0, 0x1b, 0xfe, 0x3, 0x7f, 0xc0,
    0x6f, 0xf8, 0xc, 0xff, 0x1, 0x8f, 0xff, 0xe0,

    /* U+F30B "" */
    0x7f, 0xfe, 0x3f, 0x0, 0xcf, 0xc0, 0x33, 0xf0,
    0xd, 0xfc, 0x3, 0x7f, 0x0, 0xdf, 0xc0, 0x37,
    0xf0, 0xc, 0xfc, 0x3, 0x1f, 0xff, 0x80,

    /* U+F30C "" */
    0x7f, 0xfe, 0x3c, 0x0, 0xcf, 0x0, 0x33, 0xc0,
    0xd, 0xf0, 0x3, 0x7c, 0x0, 0xdf, 0x0, 0x37,
    0xc0, 0xc, 0xf0, 0x3, 0x1f, 0xff, 0x80,

    /* U+F30D "" */
    0x7f, 0xff, 0x18, 0x0, 0x33, 0x0, 0x6, 0x60,
    0x0, 0xdc, 0x0, 0x1b, 0x80, 0x3, 0x70, 0x0,
    0x6e, 0x0, 0xc, 0xc0, 0x1, 0x8f, 0xff, 0xe0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 320, .box_w = 17, .box_h = 16, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 34, .adv_w = 320, .box_w = 19, .box_h = 10, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 58, .adv_w = 320, .box_w = 18, .box_h = 10, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 81, .adv_w = 320, .box_w = 18, .box_h = 10, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 104, .adv_w = 320, .box_w = 19, .box_h = 10, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 128, .adv_w = 320, .box_w = 18, .box_h = 10, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 151, .adv_w = 320, .box_w = 18, .box_h = 10, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 174, .adv_w = 320, .box_w = 19, .box_h = 10, .ofs_x = 1, .ofs_y = 5}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x1ae, 0x1af, 0x1b0, 0x1b1, 0x1b2, 0x1b3, 0x1b4
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 61785, .range_length = 437, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 8, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
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
const lv_font_t lvgl_font_material_symbols_rounded_20 = {
#else
lv_font_t lvgl_font_material_symbols_rounded_20 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = -2,             /*Baseline measured from the bottom of the line*/
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



#endif /*#if LVGL_FONT_MATERIAL_SYMBOLS_ROUNDED_20*/

