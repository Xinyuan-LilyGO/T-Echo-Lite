/*
 * @Description: T-Echo Lite factory original factory testing
 * @Author: LILYGO_L
 * @Date: 2024-08-07 17:27:50
 * @LastEditTime: 2025-03-28 16:04:04
 * @License: GPL 3.0
 */
#include "Adafruit_EPD.h"
#include "pin_config.h"
#include "material_monochrome_176x192px.h"
#include "Display_Fonts.h"
#include "RadioLib.h"
#include <bluefruit.h>
#include "wiring.h"
#include "Adafruit_SPIFlash.h"
#include "TinyGPSPlus.h"
#include "ICM20948_WE.h"

#define SOFTWARE_NAME "Original_Test"
#define SOFTWARE_LASTEDITTIME "202503281556"
#define BOARD_VERSION "V1.0"

#define AUTOMATICALLY_ENTER_LIGHT_SLEEP_TIME 5000

static const uint32_t Local_MAC[2] =
    {
        NRF_FICR->DEVICEID[0],
        NRF_FICR->DEVICEID[1],
};

size_t CycleTime = 0;
size_t CycleTime_2 = 0;
size_t CycleTime_3 = 0;
size_t CycleTime_4 = 0;
size_t CycleTime_5 = 0;

bool Battery_Measurement_Control_Flag = false;

bool GPS_Info_Update_Flag = false;

bool ICM20948_Initialization_Successful_Flag = false;

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
        NOT_ACTIVE,   // 未激活
        SINGLE_CLICK, // 单击
        DOUBLE_CLICK, // 双击
        LONG_PRESS,   // 长按
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

struct SX1262_Operator
{
    using state = enum {
        UNCONNECTED, // 未连接
        CONNECTED,   // 已连接
        CONNECTING,  // 正在连接
    };

    using mode = enum {
        LORA, // 普通LoRa模式
        FSK,  // 普通FSK模式
    };

    struct
    {
        float value = 850.0;
        bool change_flag = false;
    } frequency;
    struct
    {
        float value = 125.0;
        bool change_flag = false;
    } bandwidth;
    struct
    {
        uint8_t value = 9;
        bool change_flag = false;
    } spreading_factor;
    struct
    {
        uint8_t value = 7;
        bool change_flag = false;
    } coding_rate;
    struct
    {
        uint8_t value = RADIOLIB_SX126X_SYNC_WORD_PRIVATE;
        bool change_flag = false;
    } sync_word;
    struct
    {
        int8_t value = 22;
        bool change_flag = false;
    } output_power;
    struct
    {
        float value = 140;
        bool change_flag = false;
    } current_limit;
    struct
    {
        int16_t value = 16;
        bool change_flag = false;
    } preamble_length;
    struct
    {
        bool value = false;
        bool change_flag = false;
    } crc;

    struct
    {
        struct
        {
            bool send_flag = false;
            bool receive_flag = false;
        } led;

        uint32_t mac[2] = {0};
        uint32_t send_data = 0;
        uint32_t receive_data = 0;
        uint8_t connection_flag = state::UNCONNECTED;
        bool send_flag = false;
        uint8_t error_count = 11;
    } device_1;

    uint8_t current_mode = mode::LORA;

    volatile bool operation_flag = false;
    bool initialization_flag = false;
    bool mode_change_flag = false;

    uint8_t send_package[16] = {'M', 'A', 'C', ':',
                                (uint8_t)(Local_MAC[1] >> 24), (uint8_t)(Local_MAC[1] >> 16),
                                (uint8_t)(Local_MAC[1] >> 8), (uint8_t)(Local_MAC[1]),
                                (uint8_t)(Local_MAC[0] >> 24), (uint8_t)(Local_MAC[0] >> 16),
                                (uint8_t)(Local_MAC[0] >> 8), (uint8_t)Local_MAC[0],
                                0, 0, 0, 0};

    float receive_rssi = 0;
    float receive_snr = 0;
};

struct Display_Refresh_Operator
{
    struct
    {
        bool transmission_fast_refresh_flag = false;
    } sx1262_test;
};

struct BLE_Uart_Operator
{
    using state = enum {
        UNCONNECTED, // 未连接
        CONNECTED,   // 已连接
    };

    struct
    {
        bool state_flag = state::UNCONNECTED;
        bool trigger_flag = false;
        char device_name[32] = {'\0'};
    } connection;

    struct
    {
        uint8_t receive_data[100] = {'\0'};
    } transmission;

    bool initialization_flag = false;
    bool transmission_fast_refresh_flag = false;
};

/* UART Serivce: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
 * UART RXD    : 6E400002-B5A3-F393-E0A9-E50E24DCCA9E
 * UART TXD    : 6E400003-B5A3-F393-E0A9-E50E24DCCA9E
 */

// BLE Service
BLEDfu bledfu;   // OTA DFU service
BLEDis bledis;   // device information
BLEUart bleuart; // uart over ble

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
SX1262_Operator SX1262_OP;
Display_Refresh_Operator Display_Refresh_OP;
BLE_Uart_Operator BLE_Uart_OP;
Sleep_Operator Sleep_OP;

// The TinyGPSPlus object
TinyGPSPlus gps;

ICM20948_WE myIMU = ICM20948_WE(ICM20948_ADDRESS);

void Dio1_Action_Interrupt(void)
{
    // we sent or received a packet, set the flag
    SX1262_OP.operation_flag = true;
}

void Set_SX1262_RF_Transmitter_Switch(bool status)
{
    if (status == true)
    {
        digitalWrite(SX1262_RF_VC1, HIGH); // 发送
        digitalWrite(SX1262_RF_VC2, LOW);
    }
    else
    {
        digitalWrite(SX1262_RF_VC1, LOW); // 接收
        digitalWrite(SX1262_RF_VC2, HIGH);
    }
}

bool Key_Scanning(void)
{
    if (Button_Triggered_OP.trigger_start_flag == false)
    {
        if (digitalRead(Button_Triggered_OP.button_number) == LOW) // 低电平触发
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
        Button_Triggered_OP.cycletime_2 = millis() + 1000; // 计时1000ms关闭
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

                if ((Button_Triggered_OP.paragraph_triggered_count == 2)) // 单击
                {
                    Button_Triggered_OP.current_state = Button_Triggered_OP.gesture::SINGLE_CLICK;
                    return true;
                }
                else if ((Button_Triggered_OP.paragraph_triggered_count == 4)) // 双击
                {
                    Button_Triggered_OP.current_state = Button_Triggered_OP.gesture::DOUBLE_CLICK;
                    return true;
                }
                else if ((Button_Triggered_OP.paragraph_triggered_count == 1)) // 长按
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
        // delay(3000);

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
        // delay(3000);

        Serial2.end();
        pinMode(GPS_1PPS, INPUT);
        digitalWrite(GPS_WAKE_UP, LOW);
        pinMode(GPS_WAKE_UP, INPUT_PULLDOWN);
        // delay(3000);

        myIMU.sleep(true);
        Wire.end();
        pinMode(ICM20948_SDA, INPUT);
        pinMode(ICM20948_SCL, INPUT);
        // delay(3000);

        digitalWrite(GPS_RT9080_EN, LOW);
        pinMode(GPS_RT9080_EN, INPUT_PULLDOWN);
        digitalWrite(RT9080_EN, LOW);
        pinMode(RT9080_EN, INPUT_PULLDOWN);
        // delay(3000);
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

bool SX1262_Set_Default_Parameters(String *assertion)
{
    if (radio.setFrequency(SX1262_OP.frequency.value) == RADIOLIB_ERR_INVALID_FREQUENCY)
    {
        *assertion = "Failed to set frequency value";
        return false;
    }
    if (radio.setBandwidth(SX1262_OP.bandwidth.value) == RADIOLIB_ERR_INVALID_BANDWIDTH)
    {
        *assertion = "Failed to set bandwidth value";
        return false;
    }
    if (radio.setOutputPower(SX1262_OP.output_power.value) == RADIOLIB_ERR_INVALID_OUTPUT_POWER)
    {
        *assertion = "Failed to set output_power value";
        return false;
    }
    if (radio.setCurrentLimit(SX1262_OP.current_limit.value) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT)
    {
        *assertion = "Failed to set current_limit value";
        return false;
    }
    if (radio.setPreambleLength(SX1262_OP.preamble_length.value) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH)
    {
        *assertion = "Failed to set preamble_length value";
        return false;
    }
    if (radio.setCRC(SX1262_OP.crc.value) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION)
    {
        *assertion = "Failed to set crc value";
        return false;
    }
    if (SX1262_OP.current_mode == SX1262_OP.mode::LORA)
    {
        if (radio.setSpreadingFactor(SX1262_OP.spreading_factor.value) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR)
        {
            *assertion = "Failed to set spreading_factor value";
            return false;
        }
        if (radio.setCodingRate(SX1262_OP.coding_rate.value) == RADIOLIB_ERR_INVALID_CODING_RATE)
        {
            *assertion = "Failed to set coding_rate value";
            return false;
        }
        if (radio.setSyncWord(SX1262_OP.sync_word.value) != RADIOLIB_ERR_NONE)
        {
            *assertion = "Failed to set sync_word value";
            return false;
        }
    }
    else
    {
    }
    return true;
}

void GFX_Print_SX1262_Transmission_Refresh_Info(void)
{
    display.fillRect(0, 91 - 5, SCREEN_HEIGHT, 21, EPD_WHITE);
    display.setCursor(20, 91);

    if (SX1262_OP.device_1.connection_flag == SX1262_OP.state::CONNECTED)
    {
        display.printf("[Connect]: Connected");

        display.setCursor(20, 98);
        display.printf("[Connecting MAC 0]: %u", SX1262_OP.device_1.mac[0]);

        display.setCursor(20, 105);
        display.printf("[Connecting MAC 1]: %u", SX1262_OP.device_1.mac[1]);

        display.fillRect(0, 126 - 5, SCREEN_HEIGHT, 7, EPD_WHITE);
        display.setCursor(20, 126);
        display.printf("[Send Data]: %u", SX1262_OP.device_1.send_data);

        display.fillRect(0, 147 - 5, SCREEN_HEIGHT, 21, EPD_WHITE);
        display.setCursor(20, 147);
        display.printf("[Receive Data]: %u", SX1262_OP.device_1.receive_data);
        display.setCursor(20, 154);
        display.printf("[Receive RSSI]: %.1f dBm", SX1262_OP.receive_rssi);
        display.setCursor(20, 161);
        display.printf("[Receive SNR]: %.1f dB", SX1262_OP.receive_snr);
    }
    else if (SX1262_OP.device_1.connection_flag == SX1262_OP.state::CONNECTING)
    {
        display.printf("[Connect]: Connecting");

        display.fillRect(0, 126 - 5, SCREEN_HEIGHT, 7, EPD_WHITE);
        display.setCursor(20, 126);
        display.printf("[Send Data]: %u", SX1262_OP.device_1.send_data);

        display.fillRect(0, 147 - 5, SCREEN_HEIGHT, 7, EPD_WHITE);
        display.setCursor(20, 147);
        display.printf("[Receive Data]: null");
    }
    else if (SX1262_OP.device_1.connection_flag == SX1262_OP.state::UNCONNECTED)
    {
        display.printf("[Connect]: Unconnected");

        display.fillRect(0, 126 - 5, SCREEN_HEIGHT, 7, EPD_WHITE);
        display.setCursor(20, 126);
        display.printf("[Send Data]: null");

        display.fillRect(0, 147 - 5, SCREEN_HEIGHT, 21, EPD_WHITE);
        display.setCursor(20, 147);
        display.printf("[Receive Data]: null");
    }
}

void GFX_Print_SX1262_Info_Loop(void)
{
    if (SX1262_OP.initialization_flag == true)
    {
        if (Display_Refresh_OP.sx1262_test.transmission_fast_refresh_flag == true)
        {
            // if (millis() > CycleTime)
            // {
            //     CycleTime = millis() + 1000;
            Display_Refresh_OP.sx1262_test.transmission_fast_refresh_flag = false;
            GFX_Print_SX1262_Transmission_Refresh_Info();
            display.display(display.update_mode::FAST_REFRESH, true);
            // }
        }

        // if (SX1262_OP.device_1.connection_flag == SX1262_OP.state::CONNECTED)
        // {
        if (SX1262_OP.device_1.send_flag == true)
        {
            if (millis() > CycleTime_2)
            {
                SX1262_OP.device_1.send_flag = false;

                SX1262_OP.send_package[12] = (uint8_t)(SX1262_OP.device_1.send_data >> 24);
                SX1262_OP.send_package[13] = (uint8_t)(SX1262_OP.device_1.send_data >> 16);
                SX1262_OP.send_package[14] = (uint8_t)(SX1262_OP.device_1.send_data >> 8);
                SX1262_OP.send_package[15] = (uint8_t)SX1262_OP.device_1.send_data;

                // send another one
                Serial.println("[SX1262] Sending another packet ... ");

                digitalWrite(LED_1, LOW); // 开灯
                SX1262_OP.device_1.led.send_flag = true;
                CycleTime_4 = millis() + 50; // 到点自动关灯

                Set_SX1262_RF_Transmitter_Switch(true);
                radio.transmit(SX1262_OP.send_package, 16);
                Set_SX1262_RF_Transmitter_Switch(false);
                radio.startReceive();
            }
        }
        // }

        if (SX1262_OP.device_1.led.send_flag == true)
        {
            if (millis() > CycleTime_4)
            {
                SX1262_OP.device_1.led.send_flag = false;
                digitalWrite(LED_1, HIGH); // 关灯
            }
        }
        if (SX1262_OP.device_1.led.receive_flag == true)
        {
            if (millis() > CycleTime_5)
            {
                SX1262_OP.device_1.led.receive_flag = false;
                digitalWrite(LED_2, HIGH); // 关灯
            }
        }

        if (SX1262_OP.operation_flag == true)
        {
            SX1262_OP.operation_flag = false;

            uint8_t receive_package[16] = {'\0'};
            if (radio.readData(receive_package, 16) == RADIOLIB_ERR_NONE)
            {

                if ((receive_package[0] == 'M') &&
                    (receive_package[1] == 'A') &&
                    (receive_package[2] == 'C') &&
                    (receive_package[3] == ':'))
                {
                    uint32_t temp_mac[2];
                    temp_mac[0] =
                        ((uint32_t)receive_package[8] << 24) |
                        ((uint32_t)receive_package[9] << 16) |
                        ((uint32_t)receive_package[10] << 8) |
                        (uint32_t)receive_package[11];
                    temp_mac[1] =
                        ((uint32_t)receive_package[4] << 24) |
                        ((uint32_t)receive_package[5] << 16) |
                        ((uint32_t)receive_package[6] << 8) |
                        (uint32_t)receive_package[7];

                    if ((temp_mac[0] != Local_MAC[0]) && (temp_mac[1] != Local_MAC[1]))
                    {
                        SX1262_OP.device_1.mac[0] = temp_mac[0];
                        SX1262_OP.device_1.mac[1] = temp_mac[1];
                        SX1262_OP.device_1.receive_data =
                            ((uint32_t)receive_package[12] << 24) |
                            ((uint32_t)receive_package[13] << 16) |
                            ((uint32_t)receive_package[14] << 8) |
                            (uint32_t)receive_package[15];

                        // packet was successfully received
                        Serial.printf("[SX1262] Received packet\n");

                        // print data of the packet
                        for (int i = 0; i < 16; i++)
                        {
                            Serial.printf("[SX1262] Data[%d]: %#X\n", i, receive_package[i]);
                        }

                        // print RSSI (Received Signal Strength Indicator)
                        SX1262_OP.receive_rssi = radio.getRSSI();
                        Serial.printf("[SX1262] RSSI: %.1f dBm\n", SX1262_OP.receive_rssi);

                        // print SNR (Signal-to-Noise Ratio)
                        SX1262_OP.receive_snr = radio.getSNR();
                        Serial.printf("[SX1262] SNR: %.1f dB", SX1262_OP.receive_snr);

                        SX1262_OP.device_1.send_data = SX1262_OP.device_1.receive_data + 1;

                        SX1262_OP.device_1.send_flag = true;
                        SX1262_OP.device_1.connection_flag = SX1262_OP.state::CONNECTED;
                        Display_Refresh_OP.sx1262_test.transmission_fast_refresh_flag = true;

                        digitalWrite(LED_2, LOW); // 开灯
                        SX1262_OP.device_1.led.receive_flag = true;

                        // 清除错误计数看门狗
                        SX1262_OP.device_1.error_count = 0;
                        CycleTime_5 = millis() + 50; // 到点自动关灯
                        CycleTime_2 = millis() + 8000;
                    }
                }
            }
        }

        if (millis() > CycleTime_3)
        {
            SX1262_OP.device_1.error_count++;
            if (SX1262_OP.device_1.error_count == 11)
            {
                Display_Refresh_OP.sx1262_test.transmission_fast_refresh_flag = true;
            }
            if (SX1262_OP.device_1.error_count > 10)
            {
                SX1262_OP.device_1.error_count = 11;
                SX1262_OP.device_1.send_data = 0;
                SX1262_OP.device_1.connection_flag = SX1262_OP.state::UNCONNECTED;
            }
            CycleTime_3 = millis() + 2000;
        }
    }
}

bool SX1262_Initialization(void)
{
    Custom_SPI_3.begin();
    Custom_SPI_3.setClockDivider(SPI_CLOCK_DIV2);

    int16_t state = -1;
    if (SX1262_OP.current_mode == SX1262_OP.mode::LORA)
    {
        state = radio.begin();
    }
    else
    {
        state = radio.beginFSK();
    }

    if (state == RADIOLIB_ERR_NONE)
    {
        String temp_str;
        if (SX1262_Set_Default_Parameters(&temp_str) == false)
        {
            Serial.printf("SX1262 Failed to set default parameters\n");
            Serial.printf("SX1262 assertion: %s\n", temp_str.c_str());
            return false;
        }
        if (radio.startReceive() != RADIOLIB_ERR_NONE)
        {
            Serial.printf("SX1262 Failed to start receive\n");
            return false;
        }
    }
    else
    {
        Serial.printf("SX1262 initialization failed\n");
        Serial.printf("Error code: %d\n", state);
        return false;
    }

    Serial.printf("SX1262 initialization successful\n");

    return true;
}

void GFX_Print_SX1262_Info(void)
{
    display.fillScreen(EPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(1);
    display.setCursor(35, 15);
    display.printf("SX1262 Info");

    display.setFont(&Org_01);
    display.setCursor(20, 70);
    display.printf("[Local MAC 0]: %u", Local_MAC[0]);
    display.setCursor(20, 77);
    display.printf("[Local MAC 1]: %u", Local_MAC[1]);

    display.setCursor(10, 119);
    display.printf("<------------Send Info------------> ");

    display.setCursor(9, 140);
    display.printf("<-----------Receive Info-----------> ");
}

void GFX_Print_SX1262_Init_Successful_Refresh_Info(void)
{
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 40, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(20, 28);
    display.printf("[Status]: Init successful");

    display.setCursor(20, 35);
    if (SX1262_OP.current_mode == SX1262_OP.mode::LORA)
    {
        display.printf("[Mode]: LoRa");
    }
    else
    {
        display.printf("[Mode]: FSK");
    }

    display.setCursor(20, 42);
    display.printf("[Frequency]: %.1f MHz", SX1262_OP.frequency.value);

    display.setCursor(20, 49);
    display.printf("[Bandwidth]: %.1f KHz", SX1262_OP.bandwidth.value);

    display.setCursor(20, 56);
    display.printf("[Output Power]: %d dBm", SX1262_OP.output_power.value);
}

void GFX_Print_SX1262_Init_Failed_Refresh_Info(void)
{
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 40, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(20, 28);
    display.printf("[Status]: Init failed");
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
    // Get the reference to current connection
    BLEConnection *connection = Bluefruit.Connection(conn_handle);

    char central_name[32] = {0};
    connection->getPeerName(central_name, sizeof(central_name));

    Serial.print("Connected to ");
    Serial.println(central_name);

    strcpy(BLE_Uart_OP.connection.device_name, central_name);

    BLE_Uart_OP.connection.state_flag = BLE_Uart_OP.state::CONNECTED;
    BLE_Uart_OP.connection.trigger_flag = true;
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
    (void)conn_handle;
    (void)reason;

    Serial.println();
    Serial.print("Disconnected, reason = 0x");
    Serial.println(reason, HEX);
    BLE_Uart_OP.connection.state_flag = BLE_Uart_OP.state::UNCONNECTED;
    BLE_Uart_OP.connection.trigger_flag = true;
    BLE_Uart_OP.transmission_fast_refresh_flag = true;
}

void startAdv(void)
{
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();

    // Include bleuart 128-bit uuid
    Bluefruit.Advertising.addService(bleuart);

    // Secondary Scan Response packet (optional)
    // Since there is no room for 'Name' in Advertising packet
    Bluefruit.ScanResponse.addName();

    /* Start Advertising
     * - Enable auto advertising if disconnected
     * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     * - Timeout for fast mode is 30 seconds
     * - Start(timeout) with timeout = 0 will advertise forever (until connected)
     *
     * For recommended advertising interval
     * https://developer.apple.com/library/content/qa/qa1931/_index.html
     */
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
    Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
}

bool BLE_Uart_Initialization(void)
{
    // Setup the BLE LED to be enabled on CONNECT
    // Note: This is actually the default behavior, but provided
    // here in case you want to control this LED manually via PIN 19
    Bluefruit.autoConnLed(true);

    // Config the peripheral connection with maximum bandwidth
    // more SRAM required by SoftDevice
    // Note: All config***() function must be called before begin()
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

    if (Bluefruit.begin() == false)
    {
        Serial.println("BLE initialization failed");
        return false;
    }
    Serial.println("BLE initialization successful");

    Bluefruit.setTxPower(8); // Check bluefruit.h for supported values
    // Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

    // To be consistent OTA DFU should be added first if it exists
    bledfu.begin();

    // Configure and Start Device Information Service
    bledis.setManufacturer("LILYGO Industries");
    bledis.setModel("T-Echo-Lite");
    bledis.begin();

    // Configure and Start BLE Uart Service
    bleuart.begin();

    // Set up and start advertising
    startAdv();

    Serial.println("Please use the BLE debugging tool to connect to the development board.");
    Serial.println("Once connected, enter character(s) that you wish to send");

    return true;
}

void GFX_Print_BLE_Uart_Info(void)
{
    display.fillScreen(EPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(1);
    display.setCursor(25, 15);
    display.printf("BLE Uart Info");

    display.setFont(&Org_01);
    display.setCursor(10, 105);
    display.printf("<-----------Receive Info-----------> ");
}

void GFX_Print_BLE_Uart_Init_Successful_Refresh_Info(void)
{
    display.setFont(&Org_01);
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 40, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 28);
    display.printf("[Status]: Init successful");

    display.setCursor(10, 35);
    display.printf("[BLE Name]: T-Echo-Lite-nRF52840");
    display.setCursor(10, 49);
    display.printf("[Uart Serivce]: 6E400001......");
    display.setCursor(10, 56);
    display.printf("[Uart RX]: 6E400002......");
    display.setCursor(10, 63);
    display.printf("[Uart TX]: 6E400003......");

    display.setFont(&Org_01);
    display.setTextSize(1);
    display.setCursor(10, 77);
    display.printf("[Local MAC]: %s", getMcuUniqueID());
}

void GFX_Print_BLE_Uart_Init_Failed_Refresh_Info(void)
{
    display.setFont(&Org_01);
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 40, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 28);
    display.printf("[Status]: Init failed");
}

void GFX_Print_BLE_Uart_Connect_Info(void)
{
    display.fillRect(0, 91 - 5, SCREEN_HEIGHT, 14, EPD_WHITE);
    display.setCursor(10, 91);

    if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::CONNECTED)
    {
        display.printf("[Connect]: Connected");
    }
    else if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::UNCONNECTED)
    {
        display.printf("[Connect]: Unconnected");
    }
}

void GFX_Print_BLE_Uart_Transmission_Refresh_Info(void)
{
    display.setFont(&Org_01);
    display.fillRect(0, 112 - 5, SCREEN_HEIGHT, 42, EPD_WHITE);

    display.setCursor(10, 112);
    if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::CONNECTED)
    {
        display.printf("[Receive Data]: ");

        display.setCursor(20, 119);
        display.print("[");
        display.write(BLE_Uart_OP.connection.device_name,
                      strlen(BLE_Uart_OP.connection.device_name));
        display.print("]-> ");
        display.write(BLE_Uart_OP.transmission.receive_data,
                      strlen((const char *)BLE_Uart_OP.transmission.receive_data));
    }
    else if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::UNCONNECTED)
    {
        display.printf("[Receive Data]: null");
    }
}

void GFX_Print_BLE_Uart_Info_Loop(void)
{
    if (BLE_Uart_OP.connection.trigger_flag == true)
    {
        GFX_Print_BLE_Uart_Connect_Info();
        display.display(display.update_mode::FAST_REFRESH, true);

        BLE_Uart_OP.connection.trigger_flag = false;
    }

    if (BLE_Uart_OP.transmission_fast_refresh_flag == true)
    {
        GFX_Print_BLE_Uart_Transmission_Refresh_Info();
        display.display(display.update_mode::FAST_REFRESH, true);
        BLE_Uart_OP.transmission_fast_refresh_flag = false;
    }

    if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::CONNECTED)
    {
        if (millis() > CycleTime)
        {
            uint8_t send_buffer[100];
            size_t size = sprintf((char *)send_buffer,
                                  "BLE Test \n[T-Echo-Lite MAC ID]: %s\n[MCU Running time]: %u s\n",
                                  getMcuUniqueID(), (unsigned int)(millis() / 1000));
            bleuart.write(send_buffer, size);
            CycleTime = millis() + 1000;
        }

        // Forward data from HW Serial to BLEUART
        while (Serial.available())
        {
            // Delay to wait for enough input, since we have a limited transmission buffer
            delay(2);

            size_t temp_size = Serial.available();
            uint8_t temp_buf[temp_size];
            Serial.readBytes(temp_buf, temp_size);
            bleuart.write(temp_buf, temp_size);
        }

        // Forward from BLEUART to HW Serial
        while (bleuart.available())
        {
            memset(BLE_Uart_OP.transmission.receive_data, '\0', 100);
            // size_t temp_size = bleuart.available();
            // uint8_t temp_buf[temp_size];
            bleuart.read(BLE_Uart_OP.transmission.receive_data, 100);

            Serial.print("[");
            Serial.write(BLE_Uart_OP.connection.device_name,
                         strlen(BLE_Uart_OP.connection.device_name));
            Serial.print("]-> ");
            Serial.write(BLE_Uart_OP.transmission.receive_data,
                         strlen((const char *)BLE_Uart_OP.transmission.receive_data));
            BLE_Uart_OP.transmission_fast_refresh_flag = true;
        }
    }
}

void GFX_Print_Flash_Info(void)
{
    display.fillScreen(EPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(1);
    display.setCursor(45, 15);
    display.printf("Flash Info");
}

void GFX_Print_Battery_Info(void)
{
    display.fillScreen(EPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(1);
    display.setCursor(30, 15);
    display.printf("Battery Info");
}

void GFX_Print_GPS_Info_Loop(void)
{
    if (Serial2.available() > 0)
    {
        if (millis() > CycleTime_2)
        {
            // Serial.printf("%c", Serial2.read());
            Serial.printf("Serial2.available successfully\n");

            CycleTime_2 = millis() + 1000;
        }

        if (gps.encode(Serial2.read()))
        {
            display.setFont(&Org_01);
            display.fillRect(0, 28 - 5, SCREEN_WIDTH, 100, EPD_WHITE);
            display.setTextSize(1);
            display.setCursor(10, 28);

            Serial.println("GPS Initialization Successful");

            display.printf("[Status]: Initialization Successful");

            display.setCursor(10, 42);
            if (gps.location.isValid())
            {
                double lng = gps.location.lng();
                double lat = gps.location.lat();

                Serial.printf("Coordinate LNG: %f LAT: %f \n", lng, lat);
                display.printf("[Coordinate]: ");
                display.setCursor(20, 49);
                display.printf("LNG: %f ", lng);
                display.setCursor(20, 56);
                display.printf("LAT: %f", lat);
            }
            else
            {
                Serial.printf("Coordinate :Waiting to obtain......\n");
                display.printf("[Coordinate]: Waiting to obtain......");
            }

            display.setCursor(10, 70);
            if (gps.date.isValid())
            {
                uint16_t year = gps.date.year();
                uint8_t month = gps.date.month();
                uint8_t day = gps.date.day();

                Serial.printf("Data Year: %d Month: %f Day: %d\n", year, month, day);

                display.printf("[Data]: %d , %d , %d", year, month, day);
            }
            else
            {
                Serial.printf("Data : Waiting to obtain......\n");
                display.printf("[Data]: Waiting to obtain......");
            }

            display.setCursor(10, 77);
            if (gps.time.isValid())
            {
                uint8_t hour = gps.time.hour();
                uint8_t minute = gps.time.minute();
                uint8_t second = gps.time.second();
                uint8_t centisecond = gps.time.centisecond();

                if ((hour + 8) == 24)
                {
                    hour = 0;
                }
                else if ((hour + 8) > 24)
                {
                    hour = (hour + 8) - 24;
                }
                else
                {
                    hour += 8;
                }

                Serial.printf("Time: %d:%d:%d:%d\n", hour, minute, second, centisecond);

                display.printf("[Time]: %d:%d:%d:%d", hour, minute, second, centisecond);
            }
            else
            {
                Serial.printf("Time: Waiting to obtain......\n");
                display.printf("[Time]: Waiting to obtain......");
            }

            GPS_Info_Update_Flag = true;
        }
    }

    if ((millis() > CycleTime) && (GPS_Info_Update_Flag == true))
    {
        display.display(display.update_mode::FAST_REFRESH, true);

        GPS_Info_Update_Flag = false;

        CycleTime = millis() + 5000;
    }
}

void GFX_Print_ICM20948_Info_Loop(void)
{
    display.setFont(&Org_01);
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 40, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 28);

    if (ICM20948_Initialization_Successful_Flag == true)
    {
        Serial.println("ICM20948 Initialization Successful");

        display.printf("[Status]: Initialization Successful");

        myIMU.readSensor();
        float pitch = myIMU.getPitch();
        float roll = myIMU.getRoll();

        xyzFloat magValues = myIMU.getMagValues();
        float yaw = atan2(magValues.y, magValues.x) * (180.0 / M_PI); // 计算航向角

        display.setCursor(10, 42);
        Serial.printf("Pitch: %.6f", pitch);
        display.printf("[Pitch]: %.6f", pitch);

        display.setCursor(10, 49);
        Serial.printf("Roll: %.6f", roll);
        display.printf("[Roll]: %.6f", roll);

        display.setCursor(10, 56);
        Serial.printf("Yaw: %.6f", yaw);
        display.printf("[Yaw]: %.6f", yaw);
    }
    else
    {
        Serial.println("ICM20948 initialization failed");

        display.printf("[Status]: Initialization failed");
    }

    display.display(display.update_mode::FAST_REFRESH, true);
}

void GFX_Print_GPS_Info(void)
{
    display.fillScreen(EPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(1);
    display.setCursor(50, 15);
    display.printf("GPS Info");

    display.setFont(&Org_01);
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 100, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 28);

    display.printf("[Status]: Waiting for connection......");
}

void GFX_Print_ICM20948_Info(void)
{
    display.fillScreen(EPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(1);
    display.setCursor(20, 15);
    display.printf("ICM20948 Info");
}

void GFX_Print_Battery_Info_Loop(void)
{
    display.setFont(&Org_01);
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 40, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 28);

    if (Battery_Measurement_Control_Flag == true)
    {
        Serial.println("Battery measurement switch ON");

        uint32_t adc_value_temp = analogRead(BATTERY_ADC_DATA);

        display.printf("[Status]: Battery switch ON");
        display.setCursor(10, 42);
        display.printf("[ADC Value]: %u", adc_value_temp);
        display.setCursor(10, 49);
        display.printf("[ADC Voltage]: %.03f V", ((float)adc_value_temp * ((3000.0 / 4096.0))) / 1000.0);
        display.setCursor(10, 56);
        display.printf("[Battery Voltage]: %.03f V", (((float)adc_value_temp * ((3000.0 / 4096.0))) / 1000.0) * 2.0);
    }
    else
    {
        Serial.println("Battery measurement switch OFF");

        uint32_t adc_value_temp = analogRead(BATTERY_ADC_DATA);

        display.printf("[Status]: Battery switch OFF");
        display.setCursor(10, 42);
        display.printf("[ADC Value]: %u V", adc_value_temp);
        display.setCursor(10, 49);
        display.printf("[ADC Voltage]: %.03f V", ((float)adc_value_temp * ((3000.0 / 4096.0))) / 1000.0);
        display.setCursor(10, 56);
        display.printf("[Battery Voltage]: %.03f V", (((float)adc_value_temp * ((3000.0 / 4096.0))) / 1000.0) * 2.0);
    }

    display.display(display.update_mode::FAST_REFRESH, true);
}

void GFX_Print_Finish(void)
{
    display.fillScreen(EPD_WHITE);
    display.setTextColor(EPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(2);

    display.setCursor(SCREEN_WIDTH / 4 + 5, SCREEN_HEIGHT / 2 - 15);
    display.printf("Finish");
}

void GFX_Print_Start(void)
{
    display.fillScreen(EPD_WHITE);
    display.setTextColor(EPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(2);

    display.setCursor(SCREEN_WIDTH / 4 + 5, SCREEN_HEIGHT / 2 - 15);
    display.printf("Start");
}

void GFX_Print_TEST(String s)
{
    display.fillScreen(EPD_WHITE);
    display.setTextColor(EPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(2);

    display.setCursor(SCREEN_WIDTH / 4 + 5, SCREEN_HEIGHT / 4 - 15);
    display.printf("TEST");

    display.setFont(&FreeMono9pt7b);
    display.setCursor(20, SCREEN_HEIGHT / 4 + 10);
    display.setTextSize(1);
    display.print(s);

    display.setFont(&Org_01);
    display.setCursor(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 + 40);
    display.setTextSize(4);
    display.printf("3");
    display.display(display.update_mode::FULL_REFRESH, true);
    // delay(200);
    display.fillRect(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 + 20, 30, 40, EPD_WHITE);
    display.setCursor(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 + 40);
    display.printf("2");
    // display.display(display.update_mode::PARTIAL_REFRESH, true);
    // display.display(display.update_mode::PARTIAL_REFRESH, true);
    display.display(display.update_mode::FULL_REFRESH, true);
    // delay(200);
    display.fillRect(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 + 20, 30, 30, EPD_WHITE);
    display.setCursor(SCREEN_WIDTH / 2 + 3, SCREEN_HEIGHT / 2 + 40);
    display.printf("1");
    // display.display(display.update_mode::PARTIAL_REFRESH, true);
    // display.display(display.update_mode::PARTIAL_REFRESH, true);
    display.display(display.update_mode::FULL_REFRESH, true);
    // delay(200);
}

void Original_Test_6()
{
    GFX_Print_TEST("Solar charging, battery testing and inspection");

    // 关闭LED
    pinMode(LED_1, OUTPUT);
    digitalWrite(LED_1, HIGH);

    // 测量电池
    pinMode(BATTERY_ADC_DATA, INPUT);
    pinMode(BATTERY_MEASUREMENT_CONTROL, OUTPUT);
    digitalWrite(BATTERY_MEASUREMENT_CONTROL, LOW); // 关闭电池电压测量

    // Set the analog reference to 3.0V (default = 3.6V)
    analogReference(AR_INTERNAL_3_0);
    // Set the resolution to 12-bit (0..4095)
    analogReadResolution(12); // Can be 8, 10, 12 or 14

    GFX_Print_Battery_Info();

    GFX_Print_Battery_Info_Loop();

    CycleTime = millis() + 5000;
}

void Original_Test_1()
{
    GFX_Print_TEST("E-Ink screen test");

    display.fillScreen(EPD_BLACK);
    display.display(display.update_mode::FULL_REFRESH, true);

    delay(2000);

    display.fillScreen(EPD_WHITE);
    display.display(display.update_mode::FULL_REFRESH, true);

    delay(2000);

    GFX_Print_Finish();
    display.display(display.update_mode::FAST_REFRESH, true);
}

void Original_Test_2()
{
    GFX_Print_TEST("LED test");

    GFX_Print_Start();
    display.display(display.update_mode::FULL_REFRESH, true);

    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);
    delay(1000);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, HIGH);
    delay(1000);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);
    delay(1000);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, HIGH);
    delay(1000);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);
    delay(1000);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, HIGH);
    delay(1000);

    GFX_Print_Finish();
    display.display(display.update_mode::FAST_REFRESH, true);
}

void Original_Test_3()
{
    GFX_Print_TEST("SX1262 callback distance test");

    GFX_Print_SX1262_Info();
    if (SX1262_Initialization() == true)
    {
        GFX_Print_SX1262_Init_Successful_Refresh_Info();
        SX1262_OP.initialization_flag = true;
    }
    else
    {
        GFX_Print_SX1262_Init_Failed_Refresh_Info();
        SX1262_OP.initialization_flag = false;
    }
    display.display(display.update_mode::FULL_REFRESH, true);
    display.setFont(&Org_01);

    Display_Refresh_OP.sx1262_test.transmission_fast_refresh_flag = true;
}

void Original_Test_4()
{
    GFX_Print_TEST("BLE Uart test");

    GFX_Print_BLE_Uart_Info();
    if (BLE_Uart_OP.initialization_flag == true)
    {
        GFX_Print_BLE_Uart_Init_Successful_Refresh_Info();
    }
    else
    {
        if (BLE_Uart_Initialization() == true)
        {
            GFX_Print_BLE_Uart_Init_Successful_Refresh_Info();
            BLE_Uart_OP.initialization_flag = true;
        }
        else
        {
            GFX_Print_BLE_Uart_Init_Failed_Refresh_Info();
            BLE_Uart_OP.initialization_flag = false;
        }
    }

    GFX_Print_BLE_Uart_Connect_Info();
    GFX_Print_BLE_Uart_Transmission_Refresh_Info();
    display.display(display.update_mode::FULL_REFRESH, true);
}

void Original_Test_5()
{
    GFX_Print_TEST("Flash test");

    GFX_Print_Flash_Info();

    display.setFont(&Org_01);
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 40, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 28);

    if (flash.begin(&ZD25WQ32C) == true)
    // if (flash.begin(&ZD25WQ16B_2) == true)
    {
        Serial.println("Flash initialization successful");

        display.printf("[Status]: Init successful");
        display.setCursor(10, 42);
        display.printf("[Name]: ZD25WQ32C");
        display.setCursor(10, 49);
        display.printf("[Size]: 4 MiB");
        display.setCursor(10, 56);
        display.printf("[Manufacturer ID]: 0xBA");

        flashTransport.setClockSpeed(32000000UL, 0);
    }
    else
    {
        Serial.println("Flash initialization failed");

        display.printf("[Status]: initialization failed");
    }

    display.display(display.update_mode::FULL_REFRESH, true);
}

void Original_Test_7()
{
    GFX_Print_TEST("GPS test");

    Serial2.setPins(GPS_UART_RX, GPS_UART_TX);
    Serial2.begin(9600);

    pinMode(GPS_RT9080_EN, OUTPUT);
    digitalWrite(GPS_RT9080_EN, HIGH);

    pinMode(GPS_1PPS, INPUT);
    pinMode(GPS_WAKE_UP, OUTPUT);
    digitalWrite(GPS_WAKE_UP, HIGH);

    GFX_Print_GPS_Info();

    display.display(display.update_mode::FAST_REFRESH, true);

    GFX_Print_GPS_Info_Loop();
}

void Original_Test_8()
{
    GFX_Print_TEST("ICM20948 test");

    if ((myIMU.init() == false) || (myIMU.initMagnetometer() == false))
    {
        Serial.println("ICM20948 initialization failed");
        ICM20948_Initialization_Successful_Flag = false;
    }
    else
    {
        Serial.println("ICM20948 initialization successful");
        ICM20948_Initialization_Successful_Flag = true;

        myIMU.autoOffsets();

        myIMU.setAccRange(ICM20948_ACC_RANGE_2G);
        myIMU.setAccDLPF(ICM20948_DLPF_6);
        myIMU.setMagOpMode(AK09916_CONT_MODE_20HZ);
    }

    GFX_Print_ICM20948_Info();

    GFX_Print_ICM20948_Info_Loop();

    CycleTime = millis() + 5000;
}

void Original_Test_Loop()
{
    Original_Test_6();

    while (1)
    {
        bool temp = false;

        if (millis() > CycleTime)
        {
            GFX_Print_Battery_Info_Loop();

            CycleTime = millis() + 5000;
        }

        if (Key_Scanning() == true)
        {
            switch (Button_Triggered_OP.current_state)
            {
            case Button_Triggered_OP.gesture::SINGLE_CLICK:
                Serial.println("Key triggered: SINGLE_CLICK");

                Battery_Measurement_Control_Flag = !Battery_Measurement_Control_Flag;

                if (Battery_Measurement_Control_Flag == true)
                {
                    digitalWrite(LED_1, LOW);
                    digitalWrite(BATTERY_MEASUREMENT_CONTROL, HIGH); // 开启电池电压测量

                    delay(1000);
                }
                else
                {
                    digitalWrite(LED_1, HIGH);
                    digitalWrite(BATTERY_MEASUREMENT_CONTROL, LOW); // 关闭电池电压测量

                    delay(1000);
                }

                delay(300);
                break;
            case Button_Triggered_OP.gesture::DOUBLE_CLICK:
                Serial.println("Key triggered: DOUBLE_CLICK");

                Original_Test_6();

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::LONG_PRESS:
                Serial.println("Key triggered: LONG_PRESS");
                temp = true;
                // delay(1000);
                break;

            default:
                break;
            }
        }

        if (temp == true)
        {
            digitalWrite(LED_1, HIGH);                      // 关闭LED灯
            digitalWrite(BATTERY_MEASUREMENT_CONTROL, LOW); // 关闭电池电压测量
            break;
        }
    }

    Original_Test_1();

    while (1)
    {
        bool temp = false;

        if (Key_Scanning() == true)
        {
            switch (Button_Triggered_OP.current_state)
            {
            case Button_Triggered_OP.gesture::SINGLE_CLICK:
                Serial.println("Key triggered: SINGLE_CLICK");

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::DOUBLE_CLICK:
                Serial.println("Key triggered: DOUBLE_CLICK");

                Original_Test_1();

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::LONG_PRESS:
                Serial.println("Key triggered: LONG_PRESS");
                temp = true;
                // delay(1000);
                break;

            default:
                break;
            }
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_2();

    while (1)
    {
        bool temp = false;

        if (Key_Scanning() == true)
        {
            switch (Button_Triggered_OP.current_state)
            {
            case Button_Triggered_OP.gesture::SINGLE_CLICK:
                Serial.println("Key triggered: SINGLE_CLICK");

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::DOUBLE_CLICK:
                Serial.println("Key triggered: DOUBLE_CLICK");

                Original_Test_2();

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::LONG_PRESS:
                Serial.println("Key triggered: LONG_PRESS");
                temp = true;
                // delay(1000);
                break;

            default:
                break;
            }
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_3();

    while (1)
    {
        bool temp = false;

        GFX_Print_SX1262_Info_Loop();

        if (Key_Scanning() == true)
        {
            switch (Button_Triggered_OP.current_state)
            {
            case Button_Triggered_OP.gesture::SINGLE_CLICK:
                Serial.println("Key triggered: SINGLE_CLICK");

                Display_Refresh_OP.sx1262_test.transmission_fast_refresh_flag = true;
                SX1262_OP.device_1.send_flag = true;
                SX1262_OP.device_1.connection_flag = SX1262_OP.state::CONNECTING;
                // 清除错误计数看门狗
                SX1262_OP.device_1.error_count = 0;
                CycleTime_2 = millis() + 1000;

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::DOUBLE_CLICK:
                Serial.println("Key triggered: DOUBLE_CLICK");

                Original_Test_3();

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::LONG_PRESS:
                Serial.println("Key triggered: LONG_PRESS");
                temp = true;
                // delay(1000);
                break;

            default:
                break;
            }
        }

        if (temp == true)
        {
            radio.sleep();
            break;
        }
    }

    Original_Test_4();

    while (1)
    {
        bool temp = false;

        GFX_Print_BLE_Uart_Info_Loop();

        if (Key_Scanning() == true)
        {
            switch (Button_Triggered_OP.current_state)
            {
            case Button_Triggered_OP.gesture::SINGLE_CLICK:
                Serial.println("Key triggered: SINGLE_CLICK");

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::DOUBLE_CLICK:
                Serial.println("Key triggered: DOUBLE_CLICK");

                Original_Test_4();

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::LONG_PRESS:
                Serial.println("Key triggered: LONG_PRESS");
                temp = true;
                // delay(1000);
                break;

            default:
                break;
            }
        }

        if (temp == true)
        {
            Bluefruit.Advertising.stop();

            digitalWrite(LED_1, HIGH);
            digitalWrite(LED_2, HIGH);
            digitalWrite(LED_3, HIGH);
            break;
        }
    }

    Original_Test_5();

    while (1)
    {
        bool temp = false;

        if (Key_Scanning() == true)
        {
            switch (Button_Triggered_OP.current_state)
            {
            case Button_Triggered_OP.gesture::SINGLE_CLICK:
                Serial.println("Key triggered: SINGLE_CLICK");

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::DOUBLE_CLICK:
                Serial.println("Key triggered: DOUBLE_CLICK");

                Original_Test_5();

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::LONG_PRESS:
                Serial.println("Key triggered: LONG_PRESS");
                temp = true;
                // delay(1000);
                break;

            default:
                break;
            }
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_7();

    while (1)
    {
        bool temp = false;

        GFX_Print_GPS_Info_Loop();

        if (Key_Scanning() == true)
        {
            switch (Button_Triggered_OP.current_state)
            {
            case Button_Triggered_OP.gesture::SINGLE_CLICK:
                Serial.println("Key triggered: SINGLE_CLICK");

                break;
            case Button_Triggered_OP.gesture::DOUBLE_CLICK:
                Serial.println("Key triggered: DOUBLE_CLICK");

                Original_Test_7();

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::LONG_PRESS:
                Serial.println("Key triggered: LONG_PRESS");
                temp = true;
                // delay(1000);
                break;

            default:
                break;
            }
        }

        if (temp == true)
        {
            // Serial2.end();

            pinMode(GPS_WAKE_UP, INPUT_PULLDOWN);
            break;
        }
    }

    Original_Test_8();

    while (1)
    {
        bool temp = false;

        if (millis() > CycleTime)
        {
            GFX_Print_ICM20948_Info_Loop();

            CycleTime = millis() + 5000;
        }

        if (Key_Scanning() == true)
        {
            switch (Button_Triggered_OP.current_state)
            {
            case Button_Triggered_OP.gesture::SINGLE_CLICK:
                Serial.println("Key triggered: SINGLE_CLICK");

                break;
            case Button_Triggered_OP.gesture::DOUBLE_CLICK:
                Serial.println("Key triggered: DOUBLE_CLICK");

                Original_Test_8();

                // delay(1000);
                break;
            case Button_Triggered_OP.gesture::LONG_PRESS:
                Serial.println("Key triggered: LONG_PRESS");
                temp = true;
                // delay(1000);
                break;

            default:
                break;
            }
        }

        if (temp == true)
        {
            myIMU.sleep(true);
            pinMode(ICM20948_SDA, INPUT);
            pinMode(ICM20948_SCL, INPUT);
            break;
        }
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
    Serial.println("[T-Echo-Lite_" + (String)BOARD_VERSION "][" + (String)SOFTWARE_NAME +
                   "]_firmware_" + (String)SOFTWARE_LASTEDITTIME);

    Serial2.setPins(GPS_UART_RX, GPS_UART_TX);
    Serial2.begin(9600);

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    pinMode(SCREEN_BS1, OUTPUT);
    digitalWrite(SCREEN_BS1, LOW);

    pinMode(SX1262_RF_VC1, OUTPUT);
    pinMode(SX1262_RF_VC2, OUTPUT);
    Set_SX1262_RF_Transmitter_Switch(false);

    pinMode(nRF52840_BOOT, INPUT_PULLUP);
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, HIGH);

    Wire.setPins(ICM20948_SDA, ICM20948_SCL);
    Wire.begin();

    radio.setDio1Action(Dio1_Action_Interrupt);

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

    display.fillScreen(EPD_WHITE);
    display.clearBuffer();
    display.display(display.update_mode::FULL_REFRESH, true);

    display.fillScreen(EPD_WHITE);
    display.drawBitmap(0, 0, gImage_1, 192, 176, EPD_BLACK);

    display.setTextColor(EPD_WHITE);
    display.setTextSize(1);
    display.setFont(&Org_01);
    display.setCursor(25, 90);
    display.print("MCU: nRF52840");
    display.setCursor(25, 100);
    display.print("Screen: GDEM0122T61");
    display.setCursor(25, 110);
    display.print("LoRa: SX1262");
    display.setCursor(25, 120);
    display.print("Flash: ZD25WQ32C(4MB)");
    display.setCursor(25, 130);
    display.print("GPS: L76K");
    display.setCursor(25, 140);
    display.print("IMU: ICM20948");

    display.setCursor(25, 150);
    display.print("Software: " + (String)SOFTWARE_NAME);
    display.setCursor(25, 160);
    display.print("LastEditTime: " + (String)SOFTWARE_LASTEDITTIME);

    display.display(display.update_mode::FULL_REFRESH, true);

    delay(3000);

    Original_Test_Loop();

    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);

    display.setFont(&FreeSans9pt7b);
    display.fillScreen(EPD_WHITE);
    display.setCursor(10, 60);
    display.setTextColor(EPD_BLACK);
    display.setTextSize(1);
    display.print("Sleep_Wake_Up test");
    display.display(display.update_mode::FULL_REFRESH, true);

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
            display.setCursor(25, 130);
            display.print("GPS: L76K");
            display.setCursor(25, 140);
            display.print("IMU: ICM20948");

            display.setCursor(25, 150);
            display.print("Software: " + (String)SOFTWARE_NAME);
            display.setCursor(25, 160);
            display.print("LastEditTime: " + (String)SOFTWARE_LASTEDITTIME);
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
