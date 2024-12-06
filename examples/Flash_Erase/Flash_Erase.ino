/*
 * @Description: Flash erase test
 * @Author: LILYGO_L
 * @Date: 2024-08-07 15:09:38
 * @LastEditTime: 2024-11-27 09:12:22
 * @License: GPL 3.0
 */
#include <SPI.h>
#include "Adafruit_SPIFlash.h"
#include "pin_config.h"

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
    while (!Serial)
    {
        delay(100); // wait for native usb
    }
    Serial.println("Ciallo");

    Serial.println("Adafruit Serial Flash Info example");

    Custom_SPI.setClockDivider(SPI_CLOCK_DIV2); // 二分频 32Mhz
    while (flash.begin(&ZD25WQ32C) == false)
    {
        Serial.println("Flash initialization failed");
        delay(1000);
    }
    Serial.println("Flash initialization successful");

    Serial.print("Flash chip JEDEC ID: 0x");
    Serial.println(flash.getJEDECID(), HEX);

    // Wait for user to send OK to continue.
    // Increase timeout to print message less frequently.
    // Serial.setTimeout(30000);
    do
    {
        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
                       "!!!!!!!!!!");
        Serial.println("This sketch will ERASE ALL DATA on the flash chip!");
        Serial.println("Type OK (all caps) and press enter to continue.");
        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
                       "!!!!!!!!!!");
        delay(1000);
    } while (!Serial.find((char *)"OK"));

    Serial.println("Erasing flash chip in 10 seconds...");
    Serial.println(
        "Note you will see stat and other debug output printed repeatedly.");
    Serial.println(
        "Let it run for ~30 seconds until the flash erase is finished.");
    Serial.println("An error or success message will be printed when complete.");

    if (!flash.eraseChip())
    {
        Serial.println("Failed to erase chip!");
    }

    flash.waitUntilReady();
    Serial.println("Successfully erased chip!");
}

void loop()
{
    delay(1000);
}
