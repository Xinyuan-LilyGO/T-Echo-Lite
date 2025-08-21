/*
 * @Description: t_echo_lite_keyshield_config
 * @Author: LILYGO
 * @Date: 2024-12-06 14:37:43
 * @LastEditTime: 2025-08-21 18:12:15
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
        "Yes", "*", "0", "#", "Null", "Null", "Null", "Null", "Null", "Null",
        "No", "7", "8", "9", "Null", "Null", "Null", "Null", "Null", "Null",
        "Down", "4", "5", "6", "Null", "Null", "Null", "Null", "Null", "Null",
        "Center", "1", "2", "3", "Null", "Null", "Null", "Null", "Null", "Null",
        "Up", "Esc", "Home", "Mail", "Null", "Null", "Null", "Null", "Null", "Null"};

////////////////////////////////////////////////// other define config //////////////////////////////////////////////////
