/*
 * @Description: Flash test
 * @Author: LILYGO_L
 * @Date: 2024-08-06 14:03:06
 * @LastEditTime: 2025-06-05 14:56:10
 * @License: GPL 3.0
 */
#include <SPI.h>
#include "Adafruit_SPIFlash.h"
#include "t_echo_lite_config.h"

// SPI
SPIClass Custom_SPI(NRF_SPIM3, ZD25WQ32C_MISO, ZD25WQ32C_SCLK, ZD25WQ32C_MOSI);
Adafruit_FlashTransport_SPI flashTransport(ZD25WQ32C_CS, Custom_SPI);

Adafruit_SPIFlash flash(&flashTransport);

SPIFlash_Device_t ZD25WQ32C =
    {
        total_size : (1UL << 22), /* 4 MiB */
        start_up_time_us : 12000,
        manufacturer_id : 0xBA,
        memory_type : 0x60,
        capacity : 0x16,
        max_clock_speed_mhz : 104,
        quad_enable_bit_mask : 0x02,
        has_sector_protection : false,
        supports_fast_read : true,
        supports_qspi : true,
        supports_qspi_writes : true,
        write_status_register_split : false,
        single_status_byte : false,
        is_fram : false,
    };

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    Serial.println("Adafruit Serial Flash Info example");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    Custom_SPI.setClockDivider(SPI_CLOCK_DIV2); // dual frequency 32MHz
    while (flash.begin(&ZD25WQ32C) == false)
    {
        Serial.println("Flash initialization failed");
        delay(1000);
    }
    Serial.println("Flash initialization successful");
}

void loop()
{
    Serial.print("JEDEC ID: 0x");
    Serial.println(flash.getJEDECID(), HEX);
    Serial.print("Flash size: ");
    Serial.print(flash.size() / 1024);
    Serial.println(" KB");

    delay(1000);
}
