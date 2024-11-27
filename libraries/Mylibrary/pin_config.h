/*
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-08-16 14:24:03
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-11-13 15:37:59
 * @License: GPL 3.0
 */
#pragma once

#define _PINNUM(port, pin) ((port) * 32 + (pin))

// ZD25WQ32CEIGR SPI
#define ZD25WQ32C_CS _PINNUM(0, 12)
#define ZD25WQ32C_SCLK _PINNUM(0, 4)
#define ZD25WQ32C_MOSI _PINNUM(0, 6)
#define ZD25WQ32C_MISO _PINNUM(0, 8)
#define ZD25WQ32C_IO0 _PINNUM(0, 6)
#define ZD25WQ32C_IO1 _PINNUM(0, 8)
#define ZD25WQ32C_IO2 _PINNUM(1, 9)
#define ZD25WQ32C_IO3 _PINNUM(0, 26)

// LED
#define LED_1 _PINNUM(1, 7)
#define LED_2 _PINNUM(1, 5)

// GDEM0122T16
#define SCREEN_WIDTH 176
#define SCREEN_HEIGHT 192
#define SCREEN_BS1 _PINNUM(1, 12)
#define SCREEN_BUSY _PINNUM(0, 3)
#define SCREEN_RST _PINNUM(0, 28)
#define SCREEN_DC _PINNUM(0, 21)
#define SCREEN_CS _PINNUM(0, 22)
#define SCREEN_SCLK _PINNUM(0, 19)
#define SCREEN_MOSI _PINNUM(0, 20)
#define SCREEN_SRAM_CS -1
#define SCREEN_MISO -1

// Lora S62F(SX1262)
#define SX1262_CS _PINNUM(0, 11)
#define SX1262_RST _PINNUM(0, 7)
#define SX1262_SCLK _PINNUM(0, 13)
#define SX1262_MOSI _PINNUM(0, 15)
#define SX1262_MISO _PINNUM(0, 17)
#define SX1262_BUSY _PINNUM(0, 14)
#define SX1262_INT _PINNUM(1, 8)
#define SX1262_DIO1 _PINNUM(1, 8)
#define SX1262_DIO2 _PINNUM(0, 5)
#define SX1262_RF_VC1 _PINNUM(0, 27)
#define SX1262_RF_VC2 _PINNUM(1, 1)

// BOOT
#define nRF52840_BOOT _PINNUM(0, 24)

// SH1.0
#define SH1_0_1_IO25 _PINNUM(0, 25)
#define SH1_0_1_IO23 _PINNUM(0, 23)
#define SH1_0_2_IO2 _PINNUM(1, 2)
#define SH1_0_2_IO4 _PINNUM(1, 4)

// Battery
#define BATTERY_MEASUREMENT_CONTROL _PINNUM(0, 31)
#define BATTERY_ADC_DATA _PINNUM(0, 2)

// RT9080
#define RT9080_EN _PINNUM(0, 30)

// GPS L76K
#define GPS_UART_RX _PINNUM(1, 13)
#define GPS_UART_TX _PINNUM(1, 15)
#define GPS_1PPS _PINNUM(0, 29)
#define GPS_WAKE_UP _PINNUM(1, 10)

// ICM20948
#define ICM20948_ADDRESS 0x68
#define ICM20948_SDA _PINNUM(1, 4)
#define ICM20948_SCL _PINNUM(1, 2)
#define ICM20948_INT _PINNUM(0, 16)
