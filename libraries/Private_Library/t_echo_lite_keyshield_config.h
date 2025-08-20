/*
 * @Description: t_echo_lite_keyshield_config
 * @Author: LILYGO
 * @Date: 2024-12-06 14:37:43
 * @LastEditTime: 2025-08-20 15:45:51
 * @License: GPL 3.0
 */
#pragma once

#include "t_echo_lite_config.h"
#include <string>

////////////////////////////////////////////////// gpio config //////////////////////////////////////////////////

// TCA8418
#define TCA8418_SDA IIC_1_SCL
#define TCA8418_SCL IIC_1_SDA
#define TCA8418_INT EXT_2X5P_1_IO_1_3

////////////////////////////////////////////////// gpio config //////////////////////////////////////////////////

////////////////////////////////////////////////// other define config //////////////////////////////////////////////////

// TCA8418
#define TCA8418_IIC_ADDRESS 0x34
#define TCA8418_KEYPAD_SCAN_WIDTH 4
#define TCA8418_KEYPAD_SCAN_HEIGHT 5
// TCA8418键盘按键映射
const std::string Tca8418_Map[] =
    {
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
        "Esc", "Esc", "1", "2", "3", "4", "5", "6", "7", "8",
        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
        "Caps", "A", "S", "D", "F", "G", "H", "J", "K", "L",
        "Alt", "Z", "X", "C", "V", "B", "N", "M", "Ctrl", "Up",
        "Fn", "Win", "Shift", "Tab", "Space", "Space", "Space", "Fn", "Left", "Down",
        "F11", "9", "Del", "Enter", "Record", "Enter", "0", "Right"};

////////////////////////////////////////////////// other define config //////////////////////////////////////////////////
