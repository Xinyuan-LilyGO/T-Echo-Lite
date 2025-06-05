/*
 * @Description: Flash speed test
 * @Author: LILYGO_L
 * @Date: 2024-08-06 14:03:06
 * @LastEditTime: 2025-06-05 14:56:34
 * @License: GPL 3.0
 */
#include <SPI.h>
#include "Adafruit_SPIFlash.h"
#include "pin_config.h"

#define TEST_WHOLE_CHIP 1

#ifdef __AVR__
#define BUFSIZE 512
#else
#define BUFSIZE 4096
#endif

// 4 byte aligned buffer has best result with nRF QSPI
uint8_t bufwrite[BUFSIZE] __attribute__((aligned(4)));
uint8_t bufread[BUFSIZE] __attribute__((aligned(4)));

// SPI
//  SPIClass Custom_SPI(NRF_SPIM3, ZD25WQ32C_MISO, ZD25WQ32C_SCLK, ZD25WQ32C_MOSI);
//  Adafruit_FlashTransport_SPI flashTransport(ZD25WQ32C_CS, Custom_SPI);

// QSPI
Adafruit_FlashTransport_QSPI flashTransport(ZD25WQ32C_SCLK, ZD25WQ32C_CS,
                                            ZD25WQ32C_IO0, ZD25WQ32C_IO1,
                                            ZD25WQ32C_IO2, ZD25WQ32C_IO3);

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

// SPIFlash_Device_t ZD25WQ16B_2 =
//     {
//         total_size : (1UL << 21), /* 2 MiB */
//         start_up_time_us : 12000,
//         manufacturer_id : 0xBA,
//         memory_type : 0x60,
//         capacity : 0x15,
//         max_clock_speed_mhz : 33,
//         quad_enable_bit_mask : 0x02,
//         has_sector_protection : false,
//         supports_fast_read : true,
//         supports_qspi : true,
//         supports_qspi_writes : true,
//         write_status_register_split : false,
//         single_status_byte : false,
//         is_fram : false,
//     };

void print_speed(const char *text, uint32_t count, uint32_t ms)
{
    Serial.print(text);
    Serial.print(count);
    Serial.print(" bytes in ");
    Serial.print(ms / 1000.0F, 2);
    Serial.println(" seconds.");

    Serial.print("Speed: ");
    Serial.print((count / 1000.0F) / (ms / 1000.0F), 2);
    Serial.println(" KB/s.\r\n");
}

bool write_and_compare(uint8_t pattern)
{
    uint32_t ms;
    Serial.println("Erase chip");
    Serial.flush();

#if TEST_WHOLE_CHIP
    uint32_t const flash_sz = flash.size();
    flash.eraseChip();
#else
    uint32_t const flash_sz = 4096;
    flash.eraseSector(0);
#endif

    flash.waitUntilReady();

    // write all
    memset(bufwrite, (int)pattern, sizeof(bufwrite));
    Serial.print("Write flash with 0x");
    Serial.println(pattern, HEX);
    Serial.flush();
    ms = millis();

    for (uint32_t addr = 0; addr < flash_sz; addr += sizeof(bufwrite))
    {
        flash.writeBuffer(addr, bufwrite, sizeof(bufwrite));
    }

    uint32_t ms_write = millis() - ms;
    print_speed("Write ", flash_sz, ms_write);
    Serial.flush();

    // read and compare
    Serial.println("Read flash and compare");
    Serial.flush();
    uint32_t ms_read = 0;
    for (uint32_t addr = 0; addr < flash_sz; addr += sizeof(bufread))
    {
        memset(bufread, 0, sizeof(bufread));

        ms = millis();
        flash.readBuffer(addr, bufread, sizeof(bufread));
        ms_read += millis() - ms;

        if (memcmp(bufwrite, bufread, BUFSIZE))
        {
            Serial.print("Error: flash contents mismatched at address 0x");
            Serial.println(addr, HEX);
            for (uint32_t i = 0; i < sizeof(bufread); i++)
            {
                if (i != 0)
                    Serial.print(' ');
                if ((i % 16 == 0))
                {
                    Serial.println();
                    if (i < 0x100)
                        Serial.print('0');
                    if (i < 0x010)
                        Serial.print('0');
                    Serial.print(i, HEX);
                    Serial.print(": ");
                }

                if (bufread[i] < 0x10)
                    Serial.print('0');
                Serial.print(bufread[i], HEX);
            }

            Serial.println();
            return false;
        }
    }

    print_speed("Read  ", flash_sz, ms_read);
    Serial.flush();
    return true;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(100); // wait for native usb
    }
    Serial.println("Ciallo");

    Serial.println("Adafruit Serial Flash Info example");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    while (flash.begin(&ZD25WQ32C) == false)
    // while (flash.begin(&ZD25WQ16B_2) == false)
    {
        Serial.println("Flash initialization failed");
        delay(1000);
    }
    Serial.println("Flash initialization successful");

    // SPI
    // Custom_SPI.setClockDivider(SPI_CLOCK_DIV2); // dual frequency 32MHz

    // QSPI
    flashTransport.setClockSpeed(32000000UL, 0);

    pinMode(LED_1, OUTPUT);
    flash.setIndicator(LED_1, true);

    Serial.println("Adafruit Serial Flash Speed Test example");
    Serial.print("JEDEC ID: ");
    Serial.println(flash.getJEDECID(), HEX);
    Serial.print("Flash size: ");
    Serial.println(flash.size());
    Serial.flush();

    write_and_compare(0xAA);
    write_and_compare(0x55);

    Serial.println("Speed test is completed.");
    Serial.flush();
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
