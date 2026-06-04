/*******************************************************************************
 * Size: 13 px
 * Bpp: 1
 * Opts: --font assets\font\google_sans_flex\GoogleSansFlex_24pt-Regular.ttf -r 0x20-0x7E --size 13 --bpp 1 --format lvgl --lv-include lvgl.h --lv-font-name lvgl_font_google_sans_flex_regular_13 --no-compress -o examples\T-Echo-Lite-KeyShield\general_test\lvgl_font_google_sans_flex_regular_13.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef LVGL_FONT_GOOGLE_SANS_FLEX_REGULAR_13
#define LVGL_FONT_GOOGLE_SANS_FLEX_REGULAR_13 1
#endif

#if LVGL_FONT_GOOGLE_SANS_FLEX_REGULAR_13

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfe, 0x40,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x12, 0x12, 0x14, 0x7f, 0x24, 0x24, 0xfe, 0x24,
    0x24, 0x28,

    /* U+0024 "$" */
    0x21, 0xea, 0xe8, 0xe1, 0xc3, 0x89, 0xa7, 0xb7,
    0x88,

    /* U+0025 "%" */
    0x60, 0xa4, 0x49, 0x22, 0x58, 0x64, 0x2, 0x61,
    0xa4, 0x49, 0x22, 0x50, 0x60,

    /* U+0026 "&" */
    0x30, 0x91, 0x3, 0x6, 0x12, 0x22, 0xc7, 0xc4,
    0xf4,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0xd, 0x29, 0x24, 0x99, 0x32,

    /* U+0029 ")" */
    0x11, 0x22, 0x49, 0x2d, 0x68,

    /* U+002A "*" */
    0x25, 0x5c, 0xa4, 0x0,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x10,

    /* U+002C "," */
    0xe8,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x12, 0x22, 0x24, 0x44, 0x88,

    /* U+0030 "0" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x44,
    0x70,

    /* U+0031 "1" */
    0x3e, 0x92, 0x49, 0x24,

    /* U+0032 "2" */
    0x74, 0x42, 0x11, 0x88, 0x88, 0xc7, 0xc0,

    /* U+0033 "3" */
    0x39, 0x10, 0x41, 0x38, 0x30, 0x51, 0x4c, 0xe0,

    /* U+0034 "4" */
    0x8, 0x62, 0x8a, 0x49, 0x28, 0xbf, 0x8, 0x20,

    /* U+0035 "5" */
    0x79, 0x4, 0x30, 0xbb, 0x30, 0x61, 0xcd, 0xe0,

    /* U+0036 "6" */
    0x10, 0x42, 0x18, 0x7b, 0x38, 0x61, 0xcd, 0xe0,

    /* U+0037 "7" */
    0xfc, 0x10, 0x86, 0x10, 0xc2, 0x18, 0x41, 0x0,

    /* U+0038 "8" */
    0x7b, 0x38, 0x73, 0x7b, 0x38, 0x61, 0xcd, 0xe0,

    /* U+0039 "9" */
    0x7b, 0x38, 0x61, 0xcd, 0xe1, 0x84, 0x20, 0x80,

    /* U+003A ":" */
    0x82,

    /* U+003B ";" */
    0x86,

    /* U+003C "<" */
    0xc, 0xcc, 0x38, 0x18, 0x10,

    /* U+003D "=" */
    0xfc, 0x0, 0x3f,

    /* U+003E ">" */
    0x81, 0x81, 0xc6, 0xe2, 0x0,

    /* U+003F "?" */
    0x76, 0x42, 0x11, 0x10, 0x80, 0x31, 0x80,

    /* U+0040 "@" */
    0x1e, 0x18, 0x65, 0xda, 0xcd, 0xa1, 0x68, 0x5b,
    0x35, 0x7a, 0x60, 0x7, 0xc0,

    /* U+0041 "A" */
    0x8, 0xe, 0x5, 0x6, 0x82, 0x61, 0x11, 0xf8,
    0x86, 0x41, 0x60, 0x80,

    /* U+0042 "B" */
    0xfa, 0x38, 0x61, 0x8f, 0xe8, 0x61, 0x87, 0xe0,

    /* U+0043 "C" */
    0x1e, 0x63, 0x40, 0x80, 0x80, 0x80, 0x80, 0x40,
    0x63, 0x1e,

    /* U+0044 "D" */
    0xf9, 0x1a, 0x1c, 0x18, 0x30, 0x60, 0xc3, 0x8d,
    0xf0,

    /* U+0045 "E" */
    0xfc, 0x21, 0x8, 0x7e, 0x10, 0x87, 0xc0,

    /* U+0046 "F" */
    0xfc, 0x21, 0x8, 0x7e, 0x10, 0x84, 0x0,

    /* U+0047 "G" */
    0x1e, 0x18, 0x64, 0x2, 0x0, 0x80, 0x21, 0xf8,
    0x5, 0x3, 0x61, 0x87, 0x80,

    /* U+0048 "H" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xe0, 0xc1, 0x83,
    0x4,

    /* U+0049 "I" */
    0xff, 0xc0,

    /* U+004A "J" */
    0x4, 0x10, 0x41, 0x4, 0x10, 0x51, 0x4c, 0xe0,

    /* U+004B "K" */
    0x85, 0x1a, 0x64, 0x8a, 0x1a, 0x36, 0x44, 0x85,
    0xc,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0x87, 0xc0,

    /* U+004D "M" */
    0xc1, 0xe0, 0xf0, 0x74, 0x5a, 0x2d, 0xb6, 0x53,
    0x29, 0x8c, 0xc4, 0x40,

    /* U+004E "N" */
    0xc3, 0x87, 0x8d, 0x19, 0x33, 0x62, 0xc7, 0x87,
    0x4,

    /* U+004F "O" */
    0x3e, 0x31, 0x90, 0x50, 0x18, 0xc, 0x6, 0x2,
    0x82, 0x63, 0x1f, 0x0,

    /* U+0050 "P" */
    0xfa, 0x38, 0x61, 0x8f, 0xe8, 0x20, 0x82, 0x0,

    /* U+0051 "Q" */
    0x3e, 0x31, 0x90, 0x50, 0x18, 0xc, 0x6, 0x12,
    0x8e, 0x63, 0x1f, 0x80, 0x40,

    /* U+0052 "R" */
    0xfa, 0x38, 0x61, 0x8f, 0xe9, 0x22, 0x8a, 0x10,

    /* U+0053 "S" */
    0x7b, 0x38, 0x30, 0x70, 0x60, 0x61, 0xcd, 0xe0,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20,

    /* U+0055 "U" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x44,
    0x70,

    /* U+0056 "V" */
    0xc1, 0x43, 0x42, 0x62, 0x24, 0x24, 0x24, 0x18,
    0x18, 0x18,

    /* U+0057 "W" */
    0xc6, 0x14, 0x63, 0x46, 0x24, 0x52, 0x49, 0x22,
    0x92, 0x29, 0xc3, 0x8c, 0x30, 0xc1, 0xc,

    /* U+0058 "X" */
    0x42, 0x62, 0x24, 0x1c, 0x18, 0x18, 0x3c, 0x24,
    0x42, 0xc3,

    /* U+0059 "Y" */
    0x82, 0x89, 0x31, 0x43, 0x82, 0x4, 0x8, 0x10,
    0x20,

    /* U+005A "Z" */
    0xfc, 0x30, 0x86, 0x10, 0x86, 0x10, 0xc3, 0xf0,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x70,

    /* U+005C "\\" */
    0x88, 0x44, 0x42, 0x22, 0x21,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0xf0,

    /* U+005E "^" */
    0x30, 0xc4, 0xb3,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0x78, 0x17, 0xe1, 0x86, 0x37, 0x40,

    /* U+0062 "b" */
    0x82, 0x8, 0x2e, 0xce, 0x18, 0x61, 0xce, 0xe0,

    /* U+0063 "c" */
    0x79, 0x18, 0x20, 0x83, 0x13, 0x80,

    /* U+0064 "d" */
    0x4, 0x10, 0x5d, 0xce, 0x18, 0x61, 0xcd, 0xd0,

    /* U+0065 "e" */
    0x38, 0x8a, 0x17, 0xe8, 0x8, 0x8e, 0x0,

    /* U+0066 "f" */
    0x34, 0x4f, 0x44, 0x44, 0x44,

    /* U+0067 "g" */
    0x77, 0x38, 0x61, 0x87, 0x37, 0x41, 0xcd, 0xe0,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xc6, 0x18, 0x61, 0x86, 0x10,

    /* U+0069 "i" */
    0x9f, 0xc0,

    /* U+006A "j" */
    0x20, 0x12, 0x49, 0x24, 0x9c,

    /* U+006B "k" */
    0x82, 0x8, 0x22, 0x92, 0x8e, 0x24, 0x9a, 0x20,

    /* U+006C "l" */
    0xff, 0xc0,

    /* U+006D "m" */
    0xf7, 0x66, 0x62, 0x31, 0x18, 0x8c, 0x46, 0x22,

    /* U+006E "n" */
    0xbb, 0x18, 0x61, 0x86, 0x18, 0x40,

    /* U+006F "o" */
    0x38, 0x8a, 0xc, 0x18, 0x28, 0x8e, 0x0,

    /* U+0070 "p" */
    0xbb, 0x38, 0x61, 0x87, 0x3b, 0xa0, 0x82, 0x0,

    /* U+0071 "q" */
    0x77, 0x38, 0x61, 0x87, 0x37, 0x41, 0x4, 0x10,

    /* U+0072 "r" */
    0xbd, 0x88, 0x88, 0x80,

    /* U+0073 "s" */
    0x74, 0x60, 0xe0, 0xc5, 0xc0,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x44, 0x70,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x37, 0x40,

    /* U+0076 "v" */
    0x84, 0x89, 0x33, 0x42, 0x86, 0x4, 0x0,

    /* U+0077 "w" */
    0x8c, 0x53, 0x34, 0xc9, 0x4a, 0x52, 0x8c, 0xc3,
    0x10,

    /* U+0078 "x" */
    0xc5, 0x23, 0x8c, 0x39, 0x2c, 0x40,

    /* U+0079 "y" */
    0x84, 0x89, 0x31, 0x42, 0x86, 0x4, 0x18, 0x20,
    0x40,

    /* U+007A "z" */
    0xf8, 0xcc, 0x44, 0x63, 0xe0,

    /* U+007B "{" */
    0x29, 0x25, 0x12, 0x49, 0x10,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0x89, 0x24, 0x52, 0x49, 0x40,

    /* U+007E "~" */
    0x66, 0x70
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 46, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 67, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 67, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 5, .adv_w = 133, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 15, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 24, .adv_w = 173, .box_w = 10, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 130, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 37, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 47, .adv_w = 68, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 52, .adv_w = 68, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 57, .adv_w = 89, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 61, .adv_w = 116, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 65, .adv_w = 49, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 66, .adv_w = 70, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 67, .adv_w = 49, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 63, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 76, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 109, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 111, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 123, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 117, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 116, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 103, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 115, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 116, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 149, .adv_w = 49, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 49, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 151, .adv_w = 116, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 156, .adv_w = 116, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 159, .adv_w = 116, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 164, .adv_w = 99, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 184, .box_w = 10, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 184, .adv_w = 139, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 125, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 153, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 146, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 114, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 110, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 164, .box_w = 10, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 145, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 50, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 110, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 130, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 278, .adv_w = 105, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 285, .adv_w = 180, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 147, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 171, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 119, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 171, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 339, .adv_w = 121, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 114, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 111, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 136, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 131, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 196, .box_w = 12, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 132, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 408, .adv_w = 124, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 119, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 425, .adv_w = 79, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 430, .adv_w = 63, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 79, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 440, .adv_w = 97, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 443, .adv_w = 116, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 444, .adv_w = 93, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 445, .adv_w = 111, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 451, .adv_w = 125, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 459, .adv_w = 113, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 125, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 117, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 480, .adv_w = 78, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 485, .adv_w = 124, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 493, .adv_w = 116, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 501, .adv_w = 47, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 503, .adv_w = 47, .box_w = 3, .box_h = 13, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 508, .adv_w = 105, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 516, .adv_w = 43, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 182, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 526, .adv_w = 116, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 532, .adv_w = 124, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 539, .adv_w = 125, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 547, .adv_w = 125, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 555, .adv_w = 77, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 559, .adv_w = 98, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 564, .adv_w = 75, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 569, .adv_w = 116, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 106, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 582, .adv_w = 161, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 591, .adv_w = 101, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 597, .adv_w = 106, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 606, .adv_w = 99, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 611, .adv_w = 81, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 616, .adv_w = 49, .box_w = 1, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 618, .adv_w = 81, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 623, .adv_w = 116, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 3}
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
    0, 0, 0, -7, -7, 0, 0, -9,
    -17, -9, -4, 0, -5, -25, 0, -2,
    -2, -5, -6, 5, 3, -7, 0, 0,
    0, 4, 5, 5, 2, 0, -3, -3,
    -8, -2, 4, 3, 0, 0, -3, 0,
    -7, -7, 0, 0, -9, -17, -9, -4,
    0, -5, -25, 0, -2, 0, -3, -10,
    0, 0, -3, -5, 0, 0, -2, -2,
    2, -3, 5, 0, 0, -3, 0, 0,
    0, -4, -3, 3, -26, -11, 8, 3,
    -4, 0, 8, 0, -7, -4, -5, -10,
    0, -10, -3, -7, -2, -5, -26, 0,
    -9, 0, 0, 0, -11, -6, 0, -2,
    11, 3, -3, -2, 2, 0, -3, -10,
    0, -10, -3, -7, -2, -5, -26, 0,
    -9, 0, 0, 0, -11, -6, 0, -2,
    -9, -5, -18, -9, -3, 0, 0, 0,
    -12, -12, 5, 0, 0, -7, -7, -23,
    0, -4, 3, -5, -3, 0, 0, 0,
    -2, -3, -4, 2, 0, 0, 0, 0,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, 2, -3, 0, 0,
    0, 0, -2, 0, 0, 0, -3, 0,
    -2, 4, -3, 2, -2, 0, 0, -4,
    0, 5, 0, 3, 0, -3, 2, -5,
    0, 0, -5, 0, 3, 0, -2, -2,
    0, 0, -3, 0, -2, -3, 3, -4,
    -3, 0, -3, -2, 0, 0, -4, 0,
    0, -4, -2, -6, -9, 0, 2, -4,
    0, 2, -5, 0, 7, -6, -24, -6,
    -23, -14, 0, 0, 0, -2, -15, -4,
    -15, 0, -2, -2, -2, -9, 2, -4,
    4, 2, -17, 3, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, 0, 4,
    0, 0, -17, -18, -12, 4, 0, 0,
    -4, 0, 0, 0, 0, 0, -15, -2,
    0, 0, -2, -2, 0, 0, -2, 0,
    -8, 0, -2, -4, 0, 0, 2, -14,
    -10, 6, -1, 0, 2, 0, 0, 0,
    -3, -3, -2, 4, 0, 0, -7, -15,
    -9, -3, -4, 2, 0, 3, -5, 5,
    -3, -16, -16, -14, -3, -11, 0, 0,
    0, -7, -1, -30, -30, -9, -14, -8,
    -4, -4, -17, -2, -5, 0, 0, -9,
    -7, -7, 5, -3, 5, 0, 0, 0,
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
const lv_font_t lvgl_font_google_sans_flex_regular_13 = {
#else
lv_font_t lvgl_font_google_sans_flex_regular_13 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 14,          /*The maximum line height required by the font*/
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



#endif /*#if LVGL_FONT_GOOGLE_SANS_FLEX_REGULAR_13*/

