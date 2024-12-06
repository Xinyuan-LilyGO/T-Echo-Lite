/*
 * @Description: Ink screen and SX1262 test
 * @Author: LILYGO_L
 * @Date: 2023-07-25 13:45:02
 * @LastEditTime: 2024-12-03 11:36:37
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "Adafruit_EPD.h"
#include "RadioLib.h"
#include "pin_config.h"
#include "Display_Fonts.h"

static const uint32_t Local_MAC[2] =
    {
        NRF_FICR->DEVICEID[0],
        NRF_FICR->DEVICEID[1],
};

static size_t CycleTime = 0;
static size_t CycleTime_2 = 0;
static size_t CycleTime_3 = 0;
static size_t CycleTime_4 = 0;
static size_t CycleTime_5 = 0;

struct Display_Refresh_Operator
{
    struct
    {
        bool transmission_fast_refresh_flag = false;
    } sx1262_test;
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
        float value = 914.8;
        bool change_flag = false;
    } frequency;
    struct
    {
        float value = 500.0;
        bool change_flag = false;
    } bandwidth;
    struct
    {
        uint8_t value = 12;
        bool change_flag = false;
    } spreading_factor;
    struct
    {
        uint8_t value = 8;
        bool change_flag = false;
    } coding_rate;
    struct
    {
        uint8_t value = 0xAB;
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

SX1262_Operator SX1262_OP;
Display_Refresh_Operator Display_Refresh_OP;

SPIClass Custom_SPI_0(NRF_SPIM0, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC, SCREEN_RST,
                         SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &Custom_SPI_0, 8000000);

SPIClass Custom_SPI_3(NRF_SPIM3, SX1262_MISO, SX1262_SCLK, SX1262_MOSI);
SX1262 radio = new Module(SX1262_CS, SX1262_DIO1, SX1262_RST, SX1262_BUSY, Custom_SPI_3);

void Dio1_Action_Interrupt(void)
{
    // we sent or received a packet, set the flag
    SX1262_OP.operation_flag = true;
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

void GFX_Print_TEST(String s)
{
    display.fillScreen(EPD_WHITE);
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
    display.display(display.update_mode::PARTIAL_REFRESH, true);
    display.display(display.update_mode::PARTIAL_REFRESH, true);
    // delay(200);
    display.fillRect(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 + 20, 30, 30, EPD_WHITE);
    display.setCursor(SCREEN_WIDTH / 2 + 3, SCREEN_HEIGHT / 2 + 40);
    display.printf("1");
    display.display(display.update_mode::PARTIAL_REFRESH, true);
    display.display(display.update_mode::PARTIAL_REFRESH, true);
    // delay(200);
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

                radio.transmit(SX1262_OP.send_package, 16);
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
                        Serial.printf("[SX1262] RSSI: %.1f dBm", SX1262_OP.receive_rssi);

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
                        CycleTime_2 = millis() + 100;
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
            CycleTime_3 = millis() + 1000;
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

void setup(void)
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(SCREEN_BS1, OUTPUT);
    digitalWrite(SCREEN_BS1, LOW);

    pinMode(nRF52840_BOOT, INPUT_PULLUP);
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);

    radio.setDio1Action(Dio1_Action_Interrupt);

    display.begin();
    display.setRotation(1);
    display.setTextColor(EPD_BLACK);

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

void loop()
{
    if (digitalRead(nRF52840_BOOT) == LOW)
    {
        Display_Refresh_OP.sx1262_test.transmission_fast_refresh_flag = true;
        SX1262_OP.device_1.send_flag = true;
        SX1262_OP.device_1.connection_flag = SX1262_OP.state::CONNECTING;
        // 清除错误计数看门狗
        SX1262_OP.device_1.error_count = 0;
        CycleTime_2 = millis() + 1000;
    }

    GFX_Print_SX1262_Info_Loop();
}