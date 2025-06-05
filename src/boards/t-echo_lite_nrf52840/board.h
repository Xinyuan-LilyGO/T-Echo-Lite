/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2024-08-08 14:56:48
 * @LastEditTime: 2024-10-16 11:45:57
 * @License: GPL 3.0
 */
/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ha Thach for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef T_ECHO_LITE_NRF52840_H
#define T_ECHO_LITE_NRF52840_H

#define _PINNUM(port, pin)    ((port)*32 + (pin))

// 启用LDO模式
#define ENABLE_DCDC_0 0
#define ENABLE_DCDC_1 0

#define UICR_REGOUT0_VALUE UICR_REGOUT0_VOUT_3V3

/*------------------------------------------------------------------*/
/* LED
 *------------------------------------------------------------------*/
#define LEDS_NUMBER         2
#define LED_PRIMARY_PIN     _PINNUM(1, 7)
#define LED_SECONDARY_PIN   _PINNUM(1, 5)
#define LED_STATE_ON        0

/*------------------------------------------------------------------*/
/* POWER
 *------------------------------------------------------------------*/
// RT9080
#define RT9080_EN _PINNUM(0, 30)

/*------------------------------------------------------------------*/
/* BUTTON
 *------------------------------------------------------------------*/
#define BUTTONS_NUMBER      2
//按下RST按键和BOOT按键之后松开RST按键进入本地DFU模式，不能进入蓝牙DFU模式
// #define BUTTON_1            _PINNUM(0, 24)
// #define BUTTON_2            _PINNUM(0, 12)
//双击RST按键进入本地DFU模式，按下RST按键和BOOT按键之后松开RST按键进入蓝牙DFU模式
#define BUTTON_1            _PINNUM(0, 24)
#define BUTTON_2            _PINNUM(0, 24)
#define BUTTON_PULL         NRF_GPIO_PIN_PULLUP

//--------------------------------------------------------------------+
// BLE OTA
//--------------------------------------------------------------------+
#define BLEDIS_MANUFACTURER    "LilyGo"
#define BLEDIS_MODEL           "T-Echo-Lite"

//--------------------------------------------------------------------+
// USB
//--------------------------------------------------------------------+

// Shared VID/PID with Feather nRF52840, will be disabled for building in the future
#define USB_DESC_VID           0x239A
#define USB_DESC_UF2_PID       0x00DA
#define USB_DESC_CDC_ONLY_PID  0x00DA

#define UF2_PRODUCT_NAME    "Lilygo"
#define UF2_BOARD_ID        "T-Echo-Lite-nRF52840"
#define UF2_INDEX_URL       "https://www.lilygo.cc/"

#endif // PCA10056_H
