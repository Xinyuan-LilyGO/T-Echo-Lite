/*
 * @Description: None
 * @Author: None
 * @Date: 2024-12-28 09:38:30
 * @LastEditTime: 2025-05-28 09:25:42
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "pin_config.h"
#include "Adafruit_SPIFlash.h"
#include "RadioLib.h"

#define SOFTWARE_NAME "nrf52840_module"
#define SOFTWARE_LASTEDITTIME "202505280926"
#define BOARD_VERSION "V1.0"

static const uint32_t Local_MAC[2] =
    {
        NRF_FICR->DEVICEID[0],
        NRF_FICR->DEVICEID[1],
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
        float value = 915.0;
        bool change_flag = false;
    } frequency;
    struct
    {
        float value = 125.0;
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

uint8_t Pin_Test[] =
    {
        _PINNUM(1, 13),
        _PINNUM(1, 15),
        _PINNUM(0, 29),
        _PINNUM(1, 10),
        _PINNUM(1, 11),
        _PINNUM(1, 12),
        _PINNUM(0, 3),
        _PINNUM(0, 28),
        _PINNUM(1, 7),
        _PINNUM(1, 5),
        _PINNUM(1, 3),
        _PINNUM(0, 10),
        _PINNUM(0, 9),
        _PINNUM(1, 6),
        _PINNUM(1, 4),
        _PINNUM(1, 2),
        _PINNUM(0, 25),
        _PINNUM(0, 23),
        _PINNUM(0, 21),
        _PINNUM(0, 19),
        _PINNUM(0, 16),
        _PINNUM(0, 20),
        _PINNUM(0, 22),
};

size_t CycleTime = 0;
size_t CycleTime_2 = 0;
size_t CycleTime_3 = 0;
size_t CycleTime_4 = 0;
size_t CycleTime_5 = 0;
size_t CycleTime_6 = 0;

// QSPI
Adafruit_FlashTransport_QSPI flashTransport(ZD25WQ32C_SCLK, ZD25WQ32C_CS,
                                            ZD25WQ32C_IO0, ZD25WQ32C_IO1,
                                            ZD25WQ32C_IO2, ZD25WQ32C_IO3);
Adafruit_SPIFlash flash(&flashTransport);

SPIClass Custom_SPI_3(NRF_SPIM3, SX1262_MISO, SX1262_SCLK, SX1262_MOSI);
SX1262 radio = new Module(SX1262_CS, SX1262_DIO1, SX1262_RST, SX1262_BUSY, Custom_SPI_3);

Button_Triggered_Operator Button_Triggered_OP;
SX1262_Operator SX1262_OP;

void Lora_Transmission_Interrupt(void)
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

void Serial_Print_SX1262_Info_Loop(void)
{
    if (SX1262_OP.initialization_flag == true)
    {
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
                for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
                {
                    digitalWrite(Pin_Test[i], LOW);
                } // 开灯
            }
        }
        if (SX1262_OP.device_1.led.receive_flag == true)
        {
            if (millis() > CycleTime_5)
            {
                SX1262_OP.device_1.led.receive_flag = false;
                for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
                {
                    digitalWrite(Pin_Test[i], LOW);
                } // 开灯
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
                        // for (int i = 0; i < 16; i++)
                        // {
                        //     Serial.printf("[SX1262] Data[%d]: %#X\n", i, receive_package[i]);
                        // }

                        Serial.printf("[SX1262] Mac0: %d\n", SX1262_OP.device_1.mac[0]);
                        Serial.printf("[SX1262] Mac1: %d\n", SX1262_OP.device_1.mac[1]);
                        Serial.printf("[SX1262] Data: %u\n", SX1262_OP.device_1.receive_data);

                        // print RSSI (Received Signal Strength Indicator)
                        SX1262_OP.receive_rssi = radio.getRSSI();
                        Serial.printf("[SX1262] RSSI: %.1f dBm\n", SX1262_OP.receive_rssi);

                        // print SNR (Signal-to-Noise Ratio)
                        SX1262_OP.receive_snr = radio.getSNR();
                        Serial.printf("[SX1262] SNR: %.1f dB", SX1262_OP.receive_snr);

                        SX1262_OP.device_1.send_data = SX1262_OP.device_1.receive_data + 1;

                        SX1262_OP.device_1.send_flag = true;
                        SX1262_OP.device_1.connection_flag = SX1262_OP.state::CONNECTED;

                        for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
                        {
                            digitalWrite(Pin_Test[i], HIGH);
                        } // 关灯
                        SX1262_OP.device_1.led.receive_flag = true;

                        CycleTime_5 = millis() + 50; // 到点自动关灯
                        CycleTime_2 = millis() + 8000;
                    }
                }
            }
        }
    }
}

void Original_Test_3()
{
    if (SX1262_Initialization() == true)
    {
        // Serial.println("SX1262 initialization successful");
        SX1262_OP.initialization_flag = true;
        for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
        {
            digitalWrite(Pin_Test[i], HIGH);
        }
        delay(500);
        for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
        {
            digitalWrite(Pin_Test[i], LOW);
        }
        delay(1000);
    }
    else
    {
        while (1)
        {
            Serial.println("SX1262 initialization failed");
            SX1262_OP.initialization_flag = false;
            for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
            {
                digitalWrite(Pin_Test[i], HIGH);
            }
            delay(1000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    uint8_t serial_init_count = 0;
    while (!Serial)
    {
        delay(100); // wait for native usb
        serial_init_count++;
        if (serial_init_count > 30)
        {
            break;
        }
    }
    Serial.println("Ciallo");
    Serial.println("[nRF52840_Module_" + (String)BOARD_VERSION "][" + (String)SOFTWARE_NAME +
                   "]_firmware_" + (String)SOFTWARE_LASTEDITTIME);

    pinMode(SX1262_RF_VC1, OUTPUT);
    pinMode(SX1262_RF_VC2, OUTPUT);
    Set_SX1262_RF_Transmitter_Switch(false);

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    pinMode(nRF52840_BOOT, INPUT_PULLUP);

    // 测量电池
    pinMode(BATTERY_ADC_DATA, INPUT);
    pinMode(BATTERY_MEASUREMENT_CONTROL, OUTPUT);
    digitalWrite(BATTERY_MEASUREMENT_CONTROL, HIGH); // 开启电池电压测量

    // Set the analog reference to 3.0V (default = 3.6V)
    analogReference(AR_INTERNAL_3_0);
    // Set the resolution to 12-bit (0..4095)
    analogReadResolution(12); // Can be 8, 10, 12 or 14

    for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
    {
        pinMode(Pin_Test[i], OUTPUT);
    }
    for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
    {
        digitalWrite(Pin_Test[i], LOW);
    }
    delay(1000);
    for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
    {
        digitalWrite(Pin_Test[i], HIGH);
    }
    delay(1000);
    for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
    {
        digitalWrite(Pin_Test[i], LOW);
    }
    delay(1000);
    for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
    {
        digitalWrite(Pin_Test[i], HIGH);
    }
    delay(1000);
    for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
    {
        digitalWrite(Pin_Test[i], LOW);
    }
    delay(2000);

    Serial.println("pin test finish");

    while (flash.begin(&ZD25WQ32C) == false)
    {
        Serial.println("Flash initialization failed");
        for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
        {
            digitalWrite(Pin_Test[i], HIGH);
        }
        delay(1000);
    }
    Serial.println("Flash initialization successful");

    for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
    {
        digitalWrite(Pin_Test[i], HIGH);
    }
    delay(500);
    for (uint8_t i = 0; i < sizeof(Pin_Test); i++)
    {
        digitalWrite(Pin_Test[i], LOW);
    }
    delay(1000);

    radio.setDio1Action(Lora_Transmission_Interrupt);

    Original_Test_3();
}

void loop()
{
    Serial_Print_SX1262_Info_Loop();

    if (Key_Scanning() == true)
    {
        switch (Button_Triggered_OP.current_state)
        {
        case Button_Triggered_OP.gesture::SINGLE_CLICK:
            Serial.println("Key triggered: SINGLE_CLICK");

            SX1262_OP.device_1.send_flag = true;
            SX1262_OP.device_1.connection_flag = SX1262_OP.state::CONNECTING;
            CycleTime_2 = millis() + 1000;

            // delay(1000);
            break;
        case Button_Triggered_OP.gesture::DOUBLE_CLICK:
            Serial.println("Key triggered: DOUBLE_CLICK");

            // delay(1000);
            break;
        case Button_Triggered_OP.gesture::LONG_PRESS:
            Serial.println("Key triggered: LONG_PRESS");

            // delay(1000);
            break;

        default:
            break;
        }
    }

    // if (millis() > CycleTime_6)
    // {
    //     Serial.print("Battery voltage measurement\n");

    //     Serial.print("ADC Value:");
    //     Serial.println(analogRead(BATTERY_ADC_DATA));

    //     Serial.printf("ADC Voltage: %.03f V\n", ((float)analogRead(BATTERY_ADC_DATA) * ((3000.0 / 4096.0))) / 1000.0);

    //     Serial.printf("Battery Voltage: %.03f V\n", (((float)analogRead(BATTERY_ADC_DATA) * ((3000.0 / 4096.0))) / 1000.0) * 2.0);
    //     Serial.println();

    //     CycleTime_6 = millis() + 5000;
    // }

    // delay(500);
}