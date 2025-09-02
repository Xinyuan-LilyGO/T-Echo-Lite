/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-08-25 16:09:08
 * @LastEditTime: 2025-09-02 17:19:55
 * @License: GPL 3.0
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include "t_echo_lite_keyshield_config.h"
#include "cpp_bus_driver_library.h"
#include "c2_b16_s44100.h"
#include "PDM.h"

#define SAMPLE_RATE 16000
#define MAX_IIS_DATA_TRANSMIT_SIZE 128
#define MAX_IIS_DATA_TRANSMIT_MULTIPLE 5

#define MAX_PDM_DATA_TRANSMIT_SIZE 128

uint32_t Iis_Tx_Buffer[MAX_IIS_DATA_TRANSMIT_SIZE] = {0};

bool Iis_Data_Convert_Wait = false;

std::vector<uint32_t> Pdm_Rx_Stream;

auto IIS_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iis>(-1, SPEAKER_DATA, SPEAKER_WS_LRCK, SPEAKER_BCLK, -1);

void Iis_Data_Convert(const void *input_data, void *out_buffer, size_t input_data_start_index, size_t byte)
{
    const uint8_t *input_ptr = (const uint8_t *)input_data + input_data_start_index;
    uint8_t *out_ptr = (uint8_t *)out_buffer;

    memcpy(out_ptr, input_ptr, byte);
}

// Pdm_Callback中断回调里面不能放printf
void Pdm_Callback()
{
    if (PDM.available() >= MAX_PDM_DATA_TRANSMIT_SIZE)
    {
        uint32_t pdm_rx_buffer[MAX_PDM_DATA_TRANSMIT_SIZE];
        int32_t buffer_length = PDM.read(pdm_rx_buffer, MAX_PDM_DATA_TRANSMIT_SIZE * sizeof(uint32_t));

        if (Pdm_Rx_Stream.size() < (MAX_IIS_DATA_TRANSMIT_SIZE * MAX_IIS_DATA_TRANSMIT_MULTIPLE))
        {
            Pdm_Rx_Stream.insert(Pdm_Rx_Stream.end(), pdm_rx_buffer, pdm_rx_buffer + (buffer_length / sizeof(uint32_t)));
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
    printf("Ciallo\n");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    // 必须加上电延时
    delay(500);

    PDM.setPins(MICROPHONE_DATA, MICROPHONE_SCLK, -1);
    PDM.onReceive(Pdm_Callback);

    if (PDM.begin(1, SAMPLE_RATE) == false)
    {
        printf("failed to start pdm\n");
        delay(100);
    }

    IIS_Bus->begin(nrf_i2s_ratio_t ::NRF_I2S_RATIO_128X, SAMPLE_RATE, nrf_i2s_swidth_t::NRF_I2S_SWIDTH_16BIT, nrf_i2s_channels_t::NRF_I2S_CHANNELS_LEFT);

    while (1)
    {
        if (Pdm_Rx_Stream.size() >= MAX_IIS_DATA_TRANSMIT_SIZE)
        {
            // 语音扬声器测试
            printf("voice speaker start\n");

            Iis_Data_Convert(Pdm_Rx_Stream.data(), Iis_Tx_Buffer, 0, MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t));

            if (IIS_Bus->start_transmit(Iis_Tx_Buffer, nullptr, MAX_IIS_DATA_TRANSMIT_SIZE) == false)
            {
                printf("music play fail (IIS_Bus->start_transmit fail)\n");
            }

            Pdm_Rx_Stream.erase(Pdm_Rx_Stream.begin(), Pdm_Rx_Stream.begin() + MAX_IIS_DATA_TRANSMIT_SIZE);

            break;
        }
    }
}

void loop()
{
    if (IIS_Bus->get_write_event_flag() == true)
    {
        if (Iis_Data_Convert_Wait == true)
        {
            IIS_Bus->set_next_write_data(Iis_Tx_Buffer);
            Iis_Data_Convert_Wait = false;
        }
    }

    if (Pdm_Rx_Stream.size() >= MAX_IIS_DATA_TRANSMIT_SIZE)
    {
        if (Iis_Data_Convert_Wait == false)
        {
            printf("Pdm_Rx_Stream size: %d\n", Pdm_Rx_Stream.size());

            // 输出左声道数据
            printf("left: %d\n", static_cast<int16_t>(Pdm_Rx_Stream[0] >> 16));

            // 输出右声道数据
            printf("right: %d\n", static_cast<int16_t>(Pdm_Rx_Stream[0]));

            Iis_Data_Convert(Pdm_Rx_Stream.data(), Iis_Tx_Buffer, 0, MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t));

            // 只移除最开始的MAX_IIS_DATA_TRANSMIT_SIZE个数据
            Pdm_Rx_Stream.erase(Pdm_Rx_Stream.begin(), Pdm_Rx_Stream.begin() + MAX_IIS_DATA_TRANSMIT_SIZE);
            Iis_Data_Convert_Wait = true;
        }
    }
}
