/*
 * @Description: t_echo_lite_config
 * @Author: LILYGO
 * @Date: 2024-12-06 14:37:43
 * @LastEditTime: 2025-08-19 17:58:38
 * @License: GPL 3.0
 */
#pragma once

#define _PINNUM(port, pin) ((port) * 32 + (pin))

////////////////////////////////////////////////// gpio config //////////////////////////////////////////////////

// External 1x4p
#define EXT_1x4P_1_IO_0_25 _PINNUM(0, 25)
#define EXT_1x4P_1_IO_0_23 _PINNUM(0, 23)
#define EXT_1x4P_2_IO_1_2 _PINNUM(1, 2)
#define EXT_1x4P_2_IO_1_4 _PINNUM(1, 4)

// External 1x7p
#define EXT_1X7P_IO_1_13 _PINNUM(1, 13)
#define EXT_1X7P_IO_1_15 _PINNUM(1, 15)
#define EXT_1X7P_IO_0_29 _PINNUM(0, 29)
#define EXT_1X7P_IO_1_10 _PINNUM(1, 10)
#define EXT_1X7P_IO_1_11 _PINNUM(1, 11)

// External 2x5p
#define EXT_2X5P_1_IO_1_13 _PINNUM(1, 13)
#define EXT_2X5P_1_IO_1_15 _PINNUM(1, 15)
#define EXT_2X5P_1_IO_1_3 _PINNUM(1, 3)
#define EXT_2X5P_1_IO_0_16 _PINNUM(0, 16)
#define EXT_2X5P_1_IO_0_29 _PINNUM(0, 29)
#define EXT_2X5P_1_IO_1_10 _PINNUM(1, 10)
#define EXT_2X5P_1_IO_1_11 _PINNUM(1, 11)
#define EXT_2X5P_2_IO_1_6 _PINNUM(1, 6)
#define EXT_2X5P_2_IO_0_25 _PINNUM(0, 25)
#define EXT_2X5P_2_IO_1_2 _PINNUM(1, 2)
#define EXT_2X5P_2_IO_0_10 _PINNUM(0, 10)
#define EXT_2X5P_2_IO_0_23 _PINNUM(0, 23)
#define EXT_2X5P_2_IO_0_9 _PINNUM(0, 9)
#define EXT_2X5P_2_IO_1_4 _PINNUM(1, 4)

// IIC
#define IIC_1_SDA _PINNUM(1, 4)
#define IIC_1_SCL _PINNUM(1, 2)

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
#define LED_3 _PINNUM(1, 14)

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

// Battery
#define BATTERY_MEASUREMENT_CONTROL _PINNUM(0, 31)
#define BATTERY_ADC_DATA _PINNUM(0, 2)

// RT9080
#define RT9080_EN _PINNUM(0, 30)

// GPS T-Echo-Lite-L76K
#define GPS_WAKE_UP EXT_1X7P_IO_1_13
#define GPS_1PPS EXT_1X7P_IO_1_15
#define GPS_UART_TX EXT_1X7P_IO_0_29
#define GPS_UART_RX EXT_1X7P_IO_1_10
#define GPS_RT9080_EN EXT_1X7P_IO_1_11

// ICM20948
#define ICM20948_ADDRESS 0x68
#define ICM20948_SDA IIC_1_SDA
#define ICM20948_SCL IIC_1_SCL
#define ICM20948_INT _PINNUM(0, 16)

////////////////////////////////////////////////// gpio config //////////////////////////////////////////////////
