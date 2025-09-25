/*
 * @Description: aw86224
 * @Author: LILYGO_L
 * @Date: 2024-12-25 10:33:25
 * @LastEditTime: 2025-09-24 15:22:27
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include "t_echo_lite_keyshield_config.h"
#include "cpp_bus_driver_library.h"

auto IIC_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iic_2>(AW86224_SDA, AW86224_SCL, &Wire);

auto AW86224 = std::make_unique<Cpp_Bus_Driver::Aw862xx>(IIC_Bus, AW86224_IIC_ADDRESS, DEFAULT_CPP_BUS_DRIVER_VALUE);

void Iic_Scan(void)
{
    std::vector<uint8_t> address;
    if (IIC_Bus->scan_7bit_address(&address) == true)
    {
        for (size_t i = 0; i < address.size(); i++)
        {
            printf("discovered iic devices[%u]: %#X\n", i, address[i]);
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
    delay(100);
    digitalWrite(RT9080_EN, LOW);
    delay(100);
    digitalWrite(RT9080_EN, HIGH);
    delay(1000);

    AW86224->begin(500000);
    // printf("AW86224 input voltage: %.06f V\n", AW86224->get_input_voltage());

    Iic_Scan();

    // 等待F0校准
    while (1)
    {
        uint32_t f0_value = AW86224->get_f0_detection();
        printf("AW86224 get f0 detection value: %ld\n", f0_value);

        if (AW86224->set_f0_calibrate(f0_value) == true)
        {
            break;
        }
    }

    // AW86224->set_waveform_data_sample_rate(Cpp_Bus_Driver::Aw862xx::Sample_Rate::RATE_12KHZ);

    // RAM播放
    AW86224->init_ram_mode(Cpp_Bus_Driver::aw862xx_haptic_ram_12k_0809_170, sizeof(Cpp_Bus_Driver::aw862xx_haptic_ram_12k_0809_170));
}

void loop()
{
    // Iic_Scan();
    // printf("AW86224 input voltage: %.06f V\n", AW86224->get_input_voltage());

    // // RTP播放
    //  Cpp_Bus_Driver::Aw862xx::System_Status ss;
    //  if (AW86224->get_system_status(ss) == true)
    //  {
    //      if (ss.rtp_fifo_full == false)
    //      {
    //          AW86224->run_rtp_playback_waveform(Cpp_Bus_Driver::haptic_waveform_ordinary, sizeof(Cpp_Bus_Driver::haptic_waveform_ordinary));
    //          printf("AW86224 rtp_playback_waveform \n");

    //          delay(100);
    //     }
    // }
    // delay(10);

    // RAM播放
    AW86224->run_ram_playback_waveform(1, 15, 255);
    delay(1000);
    AW86224->stop_ram_playback_waveform();
    delay(1000);

    AW86224->run_ram_playback_waveform(2, 14, 255);
    delay(30);
    AW86224->run_ram_playback_waveform(2, 14, 255);
    delay(30);

    delay(1000);

    AW86224->run_ram_playback_waveform(1, 14, 255);
    delay(2000);
}
