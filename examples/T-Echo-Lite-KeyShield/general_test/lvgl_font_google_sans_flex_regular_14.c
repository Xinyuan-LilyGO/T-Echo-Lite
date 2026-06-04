/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --font assets\font\google_sans_flex\GoogleSansFlex_24pt-Regular.ttf -r 0x20-0x7E --size 14 --bpp 1 --format lvgl --lv-include lvgl.h --lv-font-name lvgl_font_google_sans_flex_regular_14 --no-compress -o examples\T-Echo-Lite-KeyShield\general_test\lvgl_font_google_sans_flex_regular_14.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef LVGL_FONT_GOOGLE_SANS_FLEX_REGULAR_14
#define LVGL_FONT_GOOGLE_SANS_FLEX_REGULAR_14 1
#endif

#if LVGL_FONT_GOOGLE_SANS_FLEX_REGULAR_14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfe, 0x60,

    /* U+0022 "\"" */
    0xb6, 0xd0,

    /* U+0023 "#" */
    0x12, 0x9, 0x4, 0x8f, 0xe1, 0x21, 0xb0, 0x90,
    0xfe, 0x24, 0x12, 0x9, 0x0,

    /* U+0024 "$" */
    0x11, 0xe9, 0xe4, 0x93, 0xc3, 0x87, 0x16, 0x5d,
    0xde, 0x10, 0x40,

    /* U+0025 "%" */
    0x70, 0x91, 0x32, 0x24, 0x45, 0x7, 0x20, 0x8,
    0x2, 0x70, 0x51, 0x12, 0x26, 0x44, 0x87, 0x0,

    /* U+0026 "&" */
    0x38, 0x48, 0x40, 0x60, 0x60, 0xd0, 0x98, 0x8a,
    0x86, 0xc6, 0x7a,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0x2d, 0x29, 0x24, 0x91, 0x26, 0x40,

    /* U+0029 ")" */
    0x99, 0x22, 0x49, 0x25, 0x2d, 0x0,

    /* U+002A "*" */
    0x25, 0x7e, 0xe5, 0x0,

    /* U+002B "+" */
    0x20, 0x82, 0x3f, 0x20, 0x82, 0x0,

    /* U+002C "," */
    0xf8,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x11, 0x22, 0x22, 0x44, 0x4c, 0x80,

    /* U+0030 "0" */
    0x3c, 0x42, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x42, 0x42, 0x3c,

    /* U+0031 "1" */
    0x3e, 0x92, 0x49, 0x24, 0x80,

    /* U+0032 "2" */
    0x7b, 0x30, 0x41, 0xc, 0x21, 0xc, 0x63, 0xf,
    0xc0,

    /* U+0033 "3" */
    0x39, 0x34, 0x41, 0xc, 0xe0, 0xc1, 0xc5, 0x37,
    0x80,

    /* U+0034 "4" */
    0x4, 0x18, 0x70, 0xa2, 0x4c, 0x91, 0x7f, 0x4,
    0x8, 0x10,

    /* U+0035 "5" */
    0xfa, 0x8, 0x20, 0xfb, 0x30, 0x41, 0x87, 0x27,
    0x0,

    /* U+0036 "6" */
    0x8, 0x30, 0xc1, 0x7, 0x98, 0xa0, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+0037 "7" */
    0xfc, 0x10, 0x82, 0x10, 0x42, 0x8, 0x61, 0x4,
    0x0,

    /* U+0038 "8" */
    0x7b, 0x38, 0x61, 0xcd, 0xec, 0xe1, 0x87, 0x37,
    0x80,

    /* U+0039 "9" */
    0x39, 0x8a, 0xc, 0x1c, 0x6f, 0x83, 0xc, 0x30,
    0x40,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0xf0, 0xf, 0x80,

    /* U+003C "<" */
    0x4, 0x66, 0x30, 0x30, 0x30, 0x0,

    /* U+003D "=" */
    0xfc, 0x0, 0x3f,

    /* U+003E ">" */
    0x81, 0x81, 0x83, 0x33, 0x0, 0x0,

    /* U+003F "?" */
    0x7b, 0x10, 0x41, 0x8, 0x42, 0x8, 0x0, 0xc3,
    0x0,

    /* U+0040 "@" */
    0x1f, 0x6, 0x11, 0x1, 0x67, 0xb9, 0x33, 0x22,
    0x64, 0x4c, 0x99, 0xce, 0xc8, 0x0, 0x80, 0xf,
    0x80,

    /* U+0041 "A" */
    0xc, 0xe, 0x5, 0x2, 0xc3, 0x21, 0x10, 0x8c,
    0xfe, 0x41, 0x20, 0xf0, 0x20,

    /* U+0042 "B" */
    0xf9, 0x1a, 0x14, 0x28, 0xdf, 0x21, 0x41, 0x83,
    0xf, 0xf0,

    /* U+0043 "C" */
    0x1e, 0x10, 0xd0, 0x10, 0x8, 0x4, 0x2, 0x1,
    0x0, 0x40, 0x10, 0xc7, 0x80,

    /* U+0044 "D" */
    0xf8, 0x86, 0x82, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x82, 0x86, 0xf8,

    /* U+0045 "E" */
    0xfe, 0x8, 0x20, 0x83, 0xf8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0x83, 0xf8, 0x20, 0x82, 0x8,
    0x0,

    /* U+0047 "G" */
    0x1e, 0x8, 0x64, 0x2, 0x0, 0x80, 0x21, 0xf8,
    0x6, 0x3, 0x40, 0x88, 0x61, 0xe0,

    /* U+0048 "H" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xe0, 0xc1, 0x83,
    0x6, 0x8,

    /* U+0049 "I" */
    0xff, 0xe0,

    /* U+004A "J" */
    0x4, 0x10, 0x41, 0x4, 0x10, 0x41, 0x87, 0x37,
    0x80,

    /* U+004B "K" */
    0x87, 0x1a, 0x24, 0x8b, 0x1e, 0x36, 0x44, 0x8d,
    0xa, 0x8,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0x84, 0x3e,

    /* U+004D "M" */
    0xc0, 0xf0, 0x3e, 0x1e, 0x85, 0xa1, 0x6c, 0xd9,
    0x26, 0x49, 0x8c, 0x63, 0x18, 0xc4,

    /* U+004E "N" */
    0xc1, 0xc1, 0xe1, 0xb1, 0x91, 0x99, 0x8d, 0x85,
    0x87, 0x83, 0x81,

    /* U+004F "O" */
    0x1e, 0x8, 0x44, 0xa, 0x1, 0x80, 0x60, 0x18,
    0x6, 0x1, 0x40, 0x98, 0x41, 0xe0,

    /* U+0050 "P" */
    0xfa, 0x38, 0x61, 0x86, 0x3f, 0xa0, 0x82, 0x8,
    0x0,

    /* U+0051 "Q" */
    0x1e, 0x8, 0x44, 0xa, 0x1, 0x80, 0x60, 0x18,
    0x6, 0x9, 0x43, 0x98, 0x41, 0xe8, 0x2,

    /* U+0052 "R" */
    0xfa, 0x38, 0x61, 0x86, 0x2f, 0xa4, 0x8a, 0x28,
    0x40,

    /* U+0053 "S" */
    0x7b, 0x38, 0x20, 0x60, 0xe0, 0xc1, 0x87, 0x37,
    0x80,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+0055 "U" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+0056 "V" */
    0xc1, 0x20, 0x90, 0x4c, 0x42, 0x21, 0x10, 0xd0,
    0x28, 0x14, 0xc, 0x2, 0x0,

    /* U+0057 "W" */
    0xc2, 0xa, 0x18, 0xd1, 0xc4, 0x8a, 0x26, 0x51,
    0x16, 0x58, 0xa2, 0x85, 0x14, 0x28, 0xe0, 0xc3,
    0x4, 0x10,

    /* U+0058 "X" */
    0x41, 0x31, 0x9, 0x82, 0x81, 0xc0, 0xc0, 0x70,
    0x68, 0x22, 0x31, 0xb0, 0x40,

    /* U+0059 "Y" */
    0x83, 0x8d, 0x11, 0x43, 0x82, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+005A "Z" */
    0xfe, 0xc, 0x10, 0x61, 0x82, 0xc, 0x30, 0x41,
    0x83, 0xf8,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x49, 0xc0,

    /* U+005C "\\" */
    0x8c, 0x44, 0x42, 0x22, 0x21, 0x10,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x93, 0xc0,

    /* U+005E "^" */
    0x10, 0xc2, 0x92, 0xc4,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0xd0,

    /* U+0061 "a" */
    0x7b, 0x30, 0x5f, 0x86, 0x18, 0xdd,

    /* U+0062 "b" */
    0x81, 0x2, 0x5, 0xcc, 0x50, 0x60, 0xc1, 0x83,
    0x8a, 0xe0,

    /* U+0063 "c" */
    0x38, 0x8a, 0x4, 0x8, 0x10, 0x91, 0x1c,

    /* U+0064 "d" */
    0x2, 0x4, 0x9, 0xd4, 0x70, 0x60, 0xc1, 0x82,
    0x8c, 0xe8,

    /* U+0065 "e" */
    0x38, 0x8a, 0xf, 0xf8, 0x10, 0x11, 0x9e,

    /* U+0066 "f" */
    0x3a, 0x11, 0xe4, 0x21, 0x8, 0x42, 0x10,

    /* U+0067 "g" */
    0x3a, 0x8e, 0xc, 0x18, 0x30, 0x51, 0x9d, 0x3,
    0x89, 0xf0,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xce, 0x18, 0x61, 0x86, 0x18,
    0x40,

    /* U+0069 "i" */
    0x9f, 0xe0,

    /* U+006A "j" */
    0x20, 0x12, 0x49, 0x24, 0x93, 0x80,

    /* U+006B "k" */
    0x82, 0x8, 0x22, 0x9a, 0xce, 0x3c, 0x9a, 0x28,
    0xc0,

    /* U+006C "l" */
    0xff, 0xe0,

    /* U+006D "m" */
    0xb9, 0xd9, 0xce, 0x10, 0xc2, 0x18, 0x43, 0x8,
    0x61, 0xc, 0x21,

    /* U+006E "n" */
    0xbb, 0x38, 0x61, 0x86, 0x18, 0x61,

    /* U+006F "o" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x51, 0x1c,

    /* U+0070 "p" */
    0xb9, 0x8a, 0xc, 0x18, 0x30, 0x71, 0x5c, 0x81,
    0x2, 0x0,

    /* U+0071 "q" */
    0x3a, 0x8e, 0xc, 0x18, 0x30, 0x51, 0x9d, 0x2,
    0x4, 0x8,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88, 0x88,

    /* U+0073 "s" */
    0x74, 0x61, 0xc3, 0x86, 0x2e,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x44, 0x43,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x1c, 0xdd,

    /* U+0076 "v" */
    0xc2, 0x8d, 0x13, 0x22, 0x85, 0xe, 0x8,

    /* U+0077 "w" */
    0x84, 0x69, 0x89, 0x29, 0x25, 0x26, 0xa8, 0x63,
    0xc, 0x61, 0x8c,

    /* U+0078 "x" */
    0xc4, 0x88, 0xa1, 0xc1, 0x5, 0x1b, 0x62,

    /* U+0079 "y" */
    0xc2, 0x89, 0x11, 0x62, 0x87, 0x4, 0x8, 0x10,
    0x40, 0x80,

    /* U+007A "z" */
    0xfc, 0x31, 0x84, 0x21, 0x8c, 0x3f,

    /* U+007B "{" */
    0x34, 0x44, 0x44, 0x48, 0x44, 0x44, 0x43,

    /* U+007C "|" */
    0xff, 0xfe,

    /* U+007D "}" */
    0xc2, 0x22, 0x22, 0x21, 0x22, 0x22, 0x2c,

    /* U+007E "~" */
    0x65, 0x38
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 50, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 72, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 72, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 5, .adv_w = 143, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 135, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 29, .adv_w = 186, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 140, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 40, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 57, .adv_w = 73, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 63, .adv_w = 73, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 69, .adv_w = 95, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 73, .adv_w = 125, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 79, .adv_w = 53, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 80, .adv_w = 75, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 81, .adv_w = 53, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 67, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 82, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 118, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 132, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 125, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 111, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 124, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 125, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 178, .adv_w = 53, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 53, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 183, .adv_w = 125, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 189, .adv_w = 125, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 192, .adv_w = 125, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 198, .adv_w = 107, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 198, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 224, .adv_w = 150, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 135, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 164, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 260, .adv_w = 157, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 123, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 118, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 176, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 156, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 313, .adv_w = 54, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 118, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 140, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 334, .adv_w = 113, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 193, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 158, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 366, .adv_w = 184, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 380, .adv_w = 129, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 184, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 404, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 413, .adv_w = 123, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 422, .adv_w = 120, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 432, .adv_w = 146, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 442, .adv_w = 141, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 455, .adv_w = 211, .box_w = 13, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 142, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 486, .adv_w = 133, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 496, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 85, .box_w = 3, .box_h = 14, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 512, .adv_w = 67, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 85, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 524, .adv_w = 104, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 528, .adv_w = 125, .box_w = 7, .box_h = 1, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 529, .adv_w = 100, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 530, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 536, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 546, .adv_w = 121, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 553, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 563, .adv_w = 126, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 570, .adv_w = 84, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 577, .adv_w = 133, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 587, .adv_w = 125, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 596, .adv_w = 50, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 598, .adv_w = 50, .box_w = 3, .box_h = 14, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 604, .adv_w = 113, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 613, .adv_w = 47, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 615, .adv_w = 196, .box_w = 11, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 626, .adv_w = 125, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 632, .adv_w = 133, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 639, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 649, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 659, .adv_w = 83, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 663, .adv_w = 105, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 668, .adv_w = 81, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 673, .adv_w = 125, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 679, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 686, .adv_w = 174, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 697, .adv_w = 109, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 704, .adv_w = 114, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 714, .adv_w = 107, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 720, .adv_w = 88, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 727, .adv_w = 53, .box_w = 1, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 729, .adv_w = 88, .box_w = 4, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 736, .adv_w = 125, .box_w = 7, .box_h = 2, .ofs_x = 1, .ofs_y = 4}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    2, 10,
    2, 18,
    3, 2,
    3, 4,
    3, 7,
    3, 9,
    3, 10,
    3, 15,
    3, 16,
    3, 21,
    3, 23,
    3, 25,
    3, 33,
    3, 64,
    3, 92,
    4, 9,
    4, 10,
    4, 15,
    4, 16,
    4, 18,
    4, 24,
    4, 62,
    5, 18,
    5, 19,
    5, 20,
    6, 19,
    6, 20,
    6, 21,
    6, 23,
    6, 24,
    7, 3,
    7, 8,
    7, 10,
    7, 18,
    7, 21,
    7, 22,
    7, 23,
    7, 24,
    7, 62,
    8, 2,
    8, 4,
    8, 7,
    8, 9,
    8, 10,
    8, 15,
    8, 16,
    8, 21,
    8, 23,
    8, 25,
    8, 33,
    8, 64,
    8, 92,
    9, 2,
    9, 3,
    9, 4,
    9, 7,
    9, 8,
    9, 17,
    9, 18,
    9, 21,
    9, 23,
    9, 26,
    9, 32,
    9, 33,
    9, 62,
    9, 92,
    9, 94,
    10, 3,
    10, 8,
    10, 15,
    10, 16,
    10, 17,
    10, 32,
    10, 62,
    10, 94,
    11, 11,
    11, 15,
    11, 16,
    11, 18,
    11, 19,
    11, 21,
    11, 23,
    11, 24,
    11, 26,
    12, 18,
    12, 20,
    12, 24,
    13, 3,
    13, 7,
    13, 8,
    13, 9,
    13, 11,
    13, 15,
    13, 17,
    13, 18,
    13, 20,
    13, 21,
    13, 22,
    13, 23,
    13, 25,
    13, 26,
    13, 32,
    13, 33,
    13, 92,
    14, 14,
    14, 17,
    14, 18,
    14, 19,
    14, 20,
    14, 21,
    14, 24,
    15, 3,
    15, 7,
    15, 8,
    15, 9,
    15, 11,
    15, 15,
    15, 17,
    15, 18,
    15, 20,
    15, 21,
    15, 22,
    15, 23,
    15, 25,
    15, 26,
    15, 32,
    15, 33,
    15, 92,
    16, 4,
    16, 9,
    16, 15,
    16, 16,
    16, 17,
    16, 18,
    16, 19,
    16, 20,
    16, 21,
    16, 23,
    16, 24,
    16, 25,
    16, 26,
    16, 27,
    16, 28,
    16, 64,
    17, 10,
    17, 13,
    17, 14,
    17, 15,
    17, 16,
    17, 18,
    17, 19,
    17, 20,
    17, 24,
    17, 61,
    17, 64,
    17, 92,
    19, 5,
    19, 18,
    19, 19,
    19, 20,
    19, 21,
    19, 22,
    19, 23,
    19, 24,
    19, 25,
    19, 26,
    19, 61,
    19, 94,
    20, 5,
    20, 10,
    20, 11,
    20, 13,
    20, 14,
    20, 15,
    20, 16,
    20, 19,
    20, 20,
    20, 23,
    20, 24,
    20, 26,
    20, 61,
    20, 62,
    20, 64,
    20, 92,
    21, 3,
    21, 4,
    21, 6,
    21, 7,
    21, 8,
    21, 11,
    21, 13,
    21, 15,
    21, 18,
    21, 20,
    21, 21,
    21, 22,
    21, 23,
    21, 24,
    21, 25,
    21, 26,
    21, 32,
    21, 33,
    21, 61,
    21, 94,
    22, 4,
    22, 5,
    22, 6,
    22, 13,
    22, 15,
    22, 18,
    22, 24,
    22, 26,
    22, 32,
    23, 3,
    23, 4,
    23, 6,
    23, 8,
    23, 10,
    23, 11,
    23, 13,
    23, 15,
    23, 16,
    23, 18,
    23, 19,
    23, 21,
    23, 24,
    23, 26,
    23, 32,
    23, 61,
    23, 64,
    23, 92,
    24, 4,
    24, 5,
    24, 6,
    24, 7,
    24, 10,
    24, 11,
    24, 12,
    24, 13,
    24, 14,
    24, 15,
    24, 16,
    24, 17,
    24, 18,
    24, 19,
    24, 20,
    24, 21,
    24, 22,
    24, 23,
    24, 24,
    24, 25,
    24, 27,
    24, 28,
    24, 29,
    24, 32,
    24, 33,
    24, 61,
    24, 62,
    24, 64,
    24, 94,
    25, 3,
    25, 8,
    25, 13,
    25, 15,
    25, 19,
    25, 20,
    25, 21,
    25, 22,
    25, 24,
    25, 26,
    25, 61,
    26, 6,
    26, 10,
    26, 11,
    26, 13,
    26, 15,
    26, 16,
    26, 18,
    26, 19,
    26, 20,
    26, 21,
    26, 22,
    26, 23,
    26, 24,
    26, 25,
    26, 61,
    26, 64,
    27, 21,
    27, 24,
    27, 64,
    27, 94,
    28, 21,
    28, 24,
    28, 64,
    28, 94,
    29, 18,
    31, 18,
    31, 20,
    31, 21,
    31, 24,
    32, 9,
    32, 10,
    32, 11,
    32, 15,
    32, 16,
    32, 18,
    32, 21,
    32, 23,
    32, 24,
    32, 26,
    32, 27,
    32, 28,
    33, 3,
    33, 8,
    33, 10,
    33, 18,
    33, 21,
    33, 24,
    33, 62,
    33, 64,
    60, 4,
    60, 7,
    60, 9,
    60, 10,
    60, 18,
    60, 24,
    60, 33,
    60, 62,
    60, 92,
    61, 3,
    61, 8,
    61, 11,
    61, 17,
    61, 18,
    61, 23,
    61, 24,
    61, 25,
    61, 26,
    61, 61,
    64, 3,
    64, 8,
    64, 9,
    64, 11,
    64, 15,
    64, 16,
    64, 17,
    64, 18,
    64, 20,
    64, 21,
    64, 23,
    64, 24,
    64, 26,
    64, 27,
    64, 28,
    64, 62,
    92, 9,
    92, 10,
    92, 18,
    92, 19,
    92, 20,
    92, 24,
    92, 27,
    92, 28,
    92, 92,
    94, 3,
    94, 8,
    94, 10,
    94, 15,
    94, 17,
    94, 20,
    94, 23,
    94, 62,
    94, 92,
    94, 94
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    0, 0, 0, -7, -8, 0, 0, -10,
    -18, -9, -4, 0, -5, -27, 0, -2,
    -2, -6, -7, 5, 3, -7, 0, 0,
    0, 5, 5, 5, 2, 0, -3, -3,
    -9, -2, 5, 3, 0, 0, -3, 0,
    -7, -8, 0, 0, -10, -18, -9, -4,
    0, -5, -27, 0, -2, 0, -3, -11,
    0, 0, -3, -6, 0, 0, -2, -2,
    2, -3, 6, 0, 0, -3, 0, 0,
    0, -4, -3, 3, -28, -11, 9, 3,
    -5, 0, 9, 0, -8, -4, -5, -10,
    0, -10, -3, -8, -3, -5, -28, 0,
    -10, 0, 0, 0, -12, -6, 0, -2,
    12, 3, -3, -3, 2, 0, -4, -10,
    0, -10, -3, -8, -3, -5, -28, 0,
    -10, 0, 0, 0, -12, -6, 0, -2,
    -10, -5, -19, -10, -3, 0, 0, 0,
    -13, -13, 5, 0, 0, -7, -7, -25,
    0, -5, 3, -5, -3, 0, 0, 0,
    -2, -3, -5, 2, 0, 0, 0, 0,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, 2, -3, 0, 0,
    0, 0, -2, 0, 0, 0, -3, 0,
    -2, 4, -3, 2, -2, 0, 0, -4,
    0, 6, 0, 3, 0, -3, 2, -5,
    0, 0, -5, 0, 3, 0, -2, -2,
    0, 0, -3, 0, -2, -3, 3, -4,
    -3, 0, -3, -2, 0, 0, -4, 0,
    0, -4, -3, -6, -10, 0, 2, -4,
    0, 2, -6, 0, 8, -7, -26, -6,
    -25, -15, 0, 0, 0, -2, -17, -4,
    -17, 0, -2, -2, -2, -9, 2, -5,
    4, 3, -18, 3, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, 0, 4,
    0, 0, -18, -19, -13, 4, 0, 0,
    -4, 0, 0, 0, 0, 0, -16, -2,
    0, 0, -2, -2, 0, 0, -2, 0,
    -8, 0, -2, -5, 0, 0, 2, -15,
    -11, 6, -1, 0, 2, 0, 0, 0,
    -3, -3, -2, 4, 0, 0, -8, -16,
    -10, -3, -4, 2, 0, 3, -6, 6,
    -3, -17, -17, -15, -3, -11, 0, 0,
    0, -8, -1, -33, -33, -9, -15, -9,
    -4, -5, -19, -3, -5, 0, 0, -10,
    -8, -8, 5, -3, 6, 0, 0, 0,
    3, -2, -2, -2, 0, 0, -3, -2,
    2, 0, 0, -3, 2, -2
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 374,
    .glyph_ids_size = 0
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
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
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
const lv_font_t lvgl_font_google_sans_flex_regular_14 = {
#else
lv_font_t lvgl_font_google_sans_flex_regular_14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LVGL_FONT_GOOGLE_SANS_FLEX_REGULAR_14*/

