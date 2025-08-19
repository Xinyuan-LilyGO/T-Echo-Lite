/*
 * @Description: Sleep wake-up test
 * @Author: LILYGO_L
 * @Date: 2024-08-07 17:27:50
 * @LastEditTime: 2025-06-05 15:04:55
 * @License: GPL 3.0
 */
#include "Adafruit_EPD.h"
#include "t_echo_lite_config.h"
#include "RadioLib.h"
#include "wiring.h"
#include <bluefruit.h>
#include "Adafruit_SPIFlash.h"
#include "material_monochrome_176x192px.h"
#include "Display_Fonts.h"
#include "ICM20948_WE.h"

#define AUTOMATICALLY_ENTER_LIGHT_SLEEP_TIME 5000

struct Sleep_Operator
{
    using mode = enum {
        NOT_SLEEP,
        LIGHT_SLEEP,
        DEEP_SLEEP,
    };

    size_t cycletime_1 = 0;

    uint8_t current_mode = mode::NOT_SLEEP;
};

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

struct Button_Triggered_Operator
{
    using gesture = enum {
        NOT_ACTIVE,   // not active
        SINGLE_CLICK, // single click
        DOUBLE_CLICK, // double click
        LONG_PRESS,   // long press
    };
    const uint32_t button_number = nRF52840_BOOT;

    size_t cycletime_1 = 0;
    size_t cycletime_2 = 0;

    uint8_t current_state = gesture::NOT_ACTIVE;
    bool trigger_start_flag = false;
    bool trigger_flag = false;
    bool timing_flag = false;
    int8_t paragraph_triggered_level = -1;
    uint8_t high_triggered_count = 0;
    uint8_t low_triggered_count = 0;
    uint8_t paragraph_triggered_count = 0;

    volatile bool Interrupt_Flag = false;
};

SPIClass Custom_SPI_1(NRF_SPIM1, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC, SCREEN_RST,
                         SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &Custom_SPI_1, 8000000);

SPIClass Custom_SPI_3(NRF_SPIM3, SX1262_MISO, SX1262_SCLK, SX1262_MOSI);
SX1262 radio = new Module(SX1262_CS, SX1262_DIO1, SX1262_RST, SX1262_BUSY, Custom_SPI_3);

// QSPI
Adafruit_FlashTransport_QSPI flashTransport(ZD25WQ32C_SCLK, ZD25WQ32C_CS,
                                            ZD25WQ32C_IO0, ZD25WQ32C_IO1,
                                            ZD25WQ32C_IO2, ZD25WQ32C_IO3);
Adafruit_SPIFlash flash(&flashTransport);

Button_Triggered_Operator Button_Triggered_OP;
Sleep_Operator Sleep_OP;

ICM20948_WE myIMU = ICM20948_WE(ICM20948_ADDRESS);

bool Key_Scanning(void)
{
    if (Button_Triggered_OP.trigger_start_flag == false)
    {
        if (digitalRead(Button_Triggered_OP.button_number) == LOW) // low level trigger
        {
            Button_Triggered_OP.trigger_start_flag = true;

            Serial.println("Press button to trigger start");
            Button_Triggered_OP.high_triggered_count = 0;
            Button_Triggered_OP.low_triggered_count = 0;
            Button_Triggered_OP.paragraph_triggered_count = 0;
            Button_Triggered_OP.paragraph_triggered_level = -1;
            Button_Triggered_OP.trigger_flag = true;
            Button_Triggered_OP.timing_flag = true;
        }
    }
    if (Button_Triggered_OP.timing_flag == true)
    {
        Button_Triggered_OP.cycletime_2 = millis() + 1000; // timing 1000ms off
        Button_Triggered_OP.timing_flag = false;
    }
    if (Button_Triggered_OP.trigger_flag == true)
    {
        if (millis() > Button_Triggered_OP.cycletime_1)
        {
            if (digitalRead(Button_Triggered_OP.button_number) == HIGH)
            {
                Button_Triggered_OP.high_triggered_count++;
                if (Button_Triggered_OP.paragraph_triggered_level != HIGH)
                {
                    Button_Triggered_OP.paragraph_triggered_count++;
                    Button_Triggered_OP.paragraph_triggered_level = HIGH;
                }
            }
            else if (digitalRead(Button_Triggered_OP.button_number) == LOW)
            {
                Button_Triggered_OP.low_triggered_count++;
                if (Button_Triggered_OP.paragraph_triggered_level != LOW)
                {
                    Button_Triggered_OP.paragraph_triggered_count++;
                    Button_Triggered_OP.paragraph_triggered_level = LOW;
                }
            }
            Button_Triggered_OP.cycletime_1 = millis() + 50;
        }

        if (Button_Triggered_OP.timing_flag == false)
        {
            if (millis() > Button_Triggered_OP.cycletime_2)
            {
                Serial.print("end\n");
                Serial.printf("high_triggered_count: %d\n", Button_Triggered_OP.high_triggered_count);
                Serial.printf("low_triggered_count: %d\n", Button_Triggered_OP.low_triggered_count);
                Serial.printf("paragraph_triggered_count: %d\n", Button_Triggered_OP.paragraph_triggered_count);

                Button_Triggered_OP.trigger_flag = false;
                Button_Triggered_OP.trigger_start_flag = false;

                if ((Button_Triggered_OP.paragraph_triggered_count == 2)) // single click
                {
                    Button_Triggered_OP.current_state = Button_Triggered_OP.gesture::SINGLE_CLICK;
                    return true;
                }
                else if ((Button_Triggered_OP.paragraph_triggered_count == 4)) // double click
                {
                    Button_Triggered_OP.current_state = Button_Triggered_OP.gesture::DOUBLE_CLICK;
                    return true;
                }
                else if ((Button_Triggered_OP.paragraph_triggered_count == 1)) // long press
                {
                    Button_Triggered_OP.current_state = Button_Triggered_OP.gesture::LONG_PRESS;
                    return true;
                }
            }
        }
    }

    return false;
}

void System_Sleep(bool mode)
{
    if (mode == true)
    {
        Serial.end();
        detachInterrupt(nRF52840_BOOT);
        pinMode(LED_1, INPUT);
        pinMode(LED_2, INPUT);
        pinMode(LED_3, INPUT);
        delay(1000);
        display.end();
        pinMode(SCREEN_BS1, INPUT);
        delay(3000);

        radio.sleep();
        Custom_SPI_3.end();
        pinMode(SX1262_MISO, INPUT);
        pinMode(SX1262_MOSI, INPUT);
        pinMode(SX1262_SCLK, INPUT);
        pinMode(SX1262_CS, INPUT);
        pinMode(SX1262_DIO1, INPUT);
        pinMode(SX1262_RST, INPUT);
        pinMode(SX1262_BUSY, INPUT);
        flashTransport.runCommand(0xB9); // Flash Deep Sleep
        flash.end();
        pinMode(ZD25WQ32C_SCLK, INPUT);
        pinMode(ZD25WQ32C_CS, INPUT);
        pinMode(ZD25WQ32C_IO0, INPUT);
        pinMode(ZD25WQ32C_IO1, INPUT);
        pinMode(ZD25WQ32C_IO2, INPUT);
        pinMode(ZD25WQ32C_IO3, INPUT);
        delay(3000);

        Serial2.end();
        pinMode(GPS_1PPS, INPUT);
        digitalWrite(GPS_WAKE_UP, LOW);
        pinMode(GPS_WAKE_UP, INPUT_PULLDOWN);
        delay(3000);

        myIMU.sleep(true);
        Wire.end();
        pinMode(ICM20948_SDA, INPUT);
        pinMode(ICM20948_SCL, INPUT);
        delay(3000);

        digitalWrite(GPS_RT9080_EN, LOW);
        pinMode(GPS_RT9080_EN, INPUT_PULLDOWN);
        digitalWrite(RT9080_EN, LOW);
        pinMode(RT9080_EN, INPUT_PULLDOWN);
        delay(3000);
    }
    else
    {
        pinMode(RT9080_EN, OUTPUT);
        digitalWrite(RT9080_EN, HIGH);
        pinMode(GPS_RT9080_EN, OUTPUT);
        digitalWrite(GPS_RT9080_EN, HIGH);

        Serial.begin(115200);
        pinMode(SCREEN_BS1, OUTPUT);
        digitalWrite(SCREEN_BS1, LOW);
        display.begin();
        display.setRotation(1);
        Custom_SPI_3.begin();
        Custom_SPI_3.setClockDivider(SPI_CLOCK_DIV2);
        radio.standby();
        flash.begin();
        flashTransport.setClockSpeed(32000000UL, 0);
        flashTransport.runCommand(0xAB); // Exit deep sleep mode
        pinMode(LED_1, OUTPUT);
        pinMode(LED_2, OUTPUT);
        pinMode(LED_3, OUTPUT);
        digitalWrite(LED_1, LOW);
        digitalWrite(LED_2, LOW);
        digitalWrite(LED_3, LOW);

        attachInterrupt(
            nRF52840_BOOT,
            []
            {
                if (Sleep_OP.current_mode == Sleep_OP.mode::LIGHT_SLEEP)
                {
                    Button_Triggered_OP.Interrupt_Flag = true;
                }
            },
            FALLING);

        Serial2.setPins(GPS_UART_RX, GPS_UART_TX);
        Serial2.begin(9600);

        pinMode(GPS_1PPS, INPUT);
        pinMode(GPS_WAKE_UP, OUTPUT);
        digitalWrite(GPS_WAKE_UP, HIGH);

        Wire.setPins(ICM20948_SDA, ICM20948_SCL);
        Wire.begin();

        myIMU.init();

        myIMU.autoOffsets();

        myIMU.setAccRange(ICM20948_ACC_RANGE_2G);
        myIMU.setAccDLPF(ICM20948_DLPF_6);

        myIMU.readSensor();
    }
}

void setup()
{
    Serial.begin(115200);
    // while (!Serial)
    // {
    //     delay(100);
    // }
    Serial.println("Ciallo");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    pinMode(SCREEN_BS1, OUTPUT);
    digitalWrite(SCREEN_BS1, LOW);

    pinMode(nRF52840_BOOT, INPUT_PULLUP);
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);

    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);

    attachInterrupt(
        nRF52840_BOOT,
        []
        {
            if (Sleep_OP.current_mode == Sleep_OP.mode::LIGHT_SLEEP)
            {
                Button_Triggered_OP.Interrupt_Flag = true;
            }
        },
        FALLING);

    display.begin();
    display.setRotation(1);
    display.setFont(&FreeSans9pt7b);

    // initialize SX1262 with default settings
    Serial.println("[SX1262] Initializing ... ");
    Custom_SPI_3.begin();
    Custom_SPI_3.setClockDivider(SPI_CLOCK_DIV2);
    while (radio.begin() != RADIOLIB_ERR_NONE)
    {
        Serial.println("SX1262 initialization failed");
        delay(1000);
    }

    Serial.println("SX1262 initialization successful");

    if (flash.begin(&ZD25WQ32C) == false)
    // while (flash.begin(&ZD25WQ16B_2) == false)
    {
        Serial.println("Flash initialization failed");
        delay(1000);
    }

    Serial.println("Flash initialization successful");

    flashTransport.setClockSpeed(32000000UL, 0);

    Serial2.setPins(GPS_UART_RX, GPS_UART_TX);
    Serial2.begin(9600);

    pinMode(GPS_1PPS, INPUT);
    pinMode(GPS_WAKE_UP, OUTPUT);
    digitalWrite(GPS_WAKE_UP, HIGH);

    Wire.setPins(ICM20948_SDA, ICM20948_SCL);
    Wire.begin();

    while (myIMU.init() == false)
    {
        Serial.println("ICM20948 initialization failed");
        delay(1000);
    }

    Serial.println("ICM20948 initialization successful");

    myIMU.autoOffsets();

    myIMU.setAccRange(ICM20948_ACC_RANGE_2G);
    myIMU.setAccDLPF(ICM20948_DLPF_6);

    myIMU.readSensor();

    // Enable the protocol stack
    //  if (Bluefruit.begin() == false)
    //  {
    //      Serial.println("BLE initialization failed");
    //  }
    //  Serial.println("BLE initialization successful");

    display.fillScreen(EPD_WHITE);
    display.setCursor(10, 60);
    display.setTextColor(EPD_BLACK);
    display.setTextSize(1);

    display.print("Sleep_Wake_Up test");

    display.display(display.update_mode::FULL_REFRESH, true);

    // System_Sleep(true);
    // systemOff(nRF52840_BOOT, LOW);

    // while (1)
    // {
    //     // waitForEvent();
    //     // // The delay is necessary, and it will not enter the sleep state without it.
    //     // delay(1000);
    // }

    Sleep_OP.cycletime_1 = millis() + AUTOMATICALLY_ENTER_LIGHT_SLEEP_TIME;
}

void loop()
{
    if (millis() > Sleep_OP.cycletime_1)
    {
        if (Sleep_OP.current_mode != Sleep_OP.mode::LIGHT_SLEEP)
        {
            Serial.println("Light Sleep");

            display.fillScreen(EPD_WHITE);
            display.setCursor(10, 60);
            display.setTextColor(EPD_BLACK);
            display.setTextSize(1);
            display.print("Light Sleep");
            display.display(display.update_mode::FAST_REFRESH, true);

            display.fillScreen(EPD_WHITE);
            display.drawBitmap(0, 0, gImage_1, 192, 176, EPD_BLACK);
            display.setTextColor(EPD_WHITE);
            display.setFont(&Org_01);
            display.setTextSize(1);
            display.setCursor(25, 90);
            display.print("MCU: nRF52840");
            display.setCursor(25, 100);
            display.print("Screen: GDEM0122T61");
            display.setCursor(25, 110);
            display.print("LoRa: SX1262");
            display.setCursor(25, 120);
            display.print("Flash: ZD25WQ32C(4MB)");
            display.setFont(&FreeSans9pt7b);
            display.setTextSize(1);
            display.setCursor(25, 145);
            display.print("Light sleep on");
            display.display(display.update_mode::FULL_REFRESH, true);

            Sleep_OP.current_mode = Sleep_OP.mode::LIGHT_SLEEP;
            System_Sleep(true);
        }
    }

    if (Sleep_OP.current_mode == Sleep_OP.mode::LIGHT_SLEEP)
    {
        waitForEvent();
        // The delay is necessary, and it will not enter the sleep state without it.
        delay(1000);

        if (digitalRead(nRF52840_BOOT) == LOW)
        {
            System_Sleep(false);

            Serial.println("Awakening");

            display.fillScreen(EPD_WHITE);
            display.setCursor(10, 60);
            display.setTextColor(EPD_BLACK);
            display.setFont(&FreeSans9pt7b);
            display.setTextSize(1);
            display.print("Awakening");
            display.display(display.update_mode::FULL_REFRESH, true);

            Sleep_OP.current_mode = Sleep_OP.mode::NOT_SLEEP;
            Button_Triggered_OP.Interrupt_Flag = false;
            Sleep_OP.cycletime_1 = millis() + AUTOMATICALLY_ENTER_LIGHT_SLEEP_TIME;
        }
    }
    else
    {
        if (Key_Scanning() == true)
        {
            display.fillScreen(EPD_WHITE);
            display.setCursor(10, 60);
            display.setTextColor(EPD_BLACK);
            display.setTextSize(1);

            switch (Button_Triggered_OP.current_state)
            {
            case Button_Triggered_OP.gesture::SINGLE_CLICK:
                Serial.println("Key triggered: SINGLE_CLICK");

                display.print("1.SINGLE_CLICK");
                display.display(display.update_mode::FAST_REFRESH, true);

                Sleep_OP.cycletime_1 = millis() + AUTOMATICALLY_ENTER_LIGHT_SLEEP_TIME;

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::DOUBLE_CLICK:
                Serial.println("Key triggered: DOUBLE_CLICK");

                display.print("2.DOUBLE_CLICK");
                display.display(display.update_mode::FAST_REFRESH, true);

                Sleep_OP.cycletime_1 = millis() + AUTOMATICALLY_ENTER_LIGHT_SLEEP_TIME;

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::LONG_PRESS:
                Serial.println("Key triggered: LONG_PRESS");

                display.print("3.LONG_PRESS");
                display.setCursor(10, 100);
                display.print("Deep Sleep");
                display.display(display.update_mode::FAST_REFRESH, true);

                Sleep_OP.current_mode = Sleep_OP.mode::DEEP_SLEEP;

                System_Sleep(true);
                systemOff(nRF52840_BOOT, LOW);

                while (1)
                {
                }
                // delay(1000);
                break;

            default:
                break;
            }
        }
    }
}
