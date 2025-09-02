/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-08-25 16:09:08
 * @LastEditTime: 2025-09-02 15:11:55
 * @License: GPL 3.0
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include "t_echo_lite_keyshield_config.h"
#include "cpp_bus_driver_library.h"
#include "c2_b16_s44100.h"

#define MCLK_MULTIPLE 32
#define SAMPLE_RATE 44100
#define MAX_IIS_DATA_TRANSMIT_SIZE 512

uint32_t Iis_Tx_Buffer[MAX_IIS_DATA_TRANSMIT_SIZE] = {0};

// 已经发送的数据大小
size_t Iis_Send_Data_Size = 0;
bool Iis_Transmit_Flag = false;

bool Iis_Data_Convert_Wait = false;

size_t Play_Count = 0;

auto IIC_Bus_0 = std::make_shared<Cpp_Bus_Driver::Hardware_Iic_2>(ES8311_SDA, ES8311_SCL, &Wire);

auto IIS_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iis>(ES8311_ADC_DATA, ES8311_DAC_DATA, ES8311_WS_LRCK, ES8311_BCLK, ES8311_MCLK);

auto ES8311 = std::make_unique<Cpp_Bus_Driver::Es8311>(IIC_Bus_0, IIS_Bus, ES8311_IIC_ADDRESS, DEFAULT_CPP_BUS_DRIVER_VALUE);

// void Iis_Data_Convert(const uint16_t *input_data, uint32_t *out_buffer, size_t input_data_start_index, size_t length)
// {
//     for (size_t i = 0; i < length; i++)
//     {
//         uint16_t sample_l = input_data[input_data_start_index + i * 2];
//         uint16_t sample_r = input_data[input_data_start_index + i * 2 + 1];

//         // 小端序：低位存储左声道，高位存储右声道
//         out_buffer[i] = (sample_r << 16) | sample_l;
//     }
// }

// void Iis_Data_Convert(const uint16_t *input_data, uint32_t *out_buffer, size_t input_data_start_index, size_t out_data_length)
// {
//     const uint16_t *input_ptr = input_data + input_data_start_index;
//     uint32_t *out_ptr = out_buffer;

//     for (size_t i = 0; i < out_data_length; ++i)
//     {
//         // 使用 memcpy 直接将两个 uint16_t 复制到 uint32_t 中，并进行字节序调整
//         memcpy(out_ptr, input_ptr, 2);                      // 复制 sample_l
//         memcpy(((uint8_t *)out_ptr) + 2, input_ptr + 1, 2); // 复制 sample_r 到高位

//         out_ptr++;
//         input_ptr += 2;
//     }
// }

// void Iis_Data_Convert(const uint16_t *input_data, uint32_t *out_buffer, size_t input_data_start_index, size_t out_data_length)
// {
//     const uint8_t *input_ptr = (const uint8_t *)(input_data + input_data_start_index);

//     for (size_t i = 0; i < out_data_length; i++)
//     {
//         // 明确复制字节，避免字节序混淆
//         memcpy(&out_buffer[i], input_ptr, 4);
//         input_ptr += 4;
//     }
// }

void Iis_Data_Convert(const void *input_data, void *out_buffer, size_t input_data_start_index, size_t byte)
{
    const uint8_t *input_ptr = (const uint8_t *)input_data + input_data_start_index;
    uint8_t *out_ptr = (uint8_t *)out_buffer;

    memcpy(out_ptr, input_ptr, byte);
}

void Iic_Scan(void)
{
    std::vector<uint8_t> address;
    if (IIC_Bus_0->scan_7bit_address(&address) == true)
    {
        for (size_t i = 0; i < address.size(); i++)
        {
            printf("discovered iic devices[%u]: %#x\n", i, address[i]);
        }
    }
    else
    {
        printf("No IIC device found\n");
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

    pinMode(nRF52840_BOOT, INPUT_PULLUP);

    ES8311->begin(nrf_i2s_ratio_t ::NRF_I2S_RATIO_32X, SAMPLE_RATE, nrf_i2s_swidth_t::NRF_I2S_SWIDTH_16BIT);

    while (1)
    {
        if (ES8311->begin(50000) == true)
        {
            printf("es8311 initialization success\n");
            break;
        }
        else
        {
            printf("es8311 initialization fail\n");
            delay(100);
        }
    }

    ES8311->set_master_clock_source(Cpp_Bus_Driver::Es8311::Clock_Source::ADC_DAC_MCLK);
    ES8311->set_clock(Cpp_Bus_Driver::Es8311::Clock_Source::ADC_DAC_MCLK, true);
    ES8311->set_clock(Cpp_Bus_Driver::Es8311::Clock_Source::ADC_DAC_BCLK, true);

    ES8311->set_clock_coeff(MCLK_MULTIPLE, SAMPLE_RATE);

    ES8311->set_serial_port_mode(Cpp_Bus_Driver::Es8311::Serial_Port_Mode::SLAVE);

    ES8311->set_sdp_data_bit_length(Cpp_Bus_Driver::Es8311::Sdp::ADC, Cpp_Bus_Driver::Es8311::Bits_Per_Sample::DATA_16BIT);
    ES8311->set_sdp_data_bit_length(Cpp_Bus_Driver::Es8311::Sdp::DAC, Cpp_Bus_Driver::Es8311::Bits_Per_Sample::DATA_16BIT);

    // Cpp_Bus_Driver::Es8311::Power_Status ps =
    //     {
    //         .contorl =
    //             {
    //                 .analog_circuits = true,               // 开启模拟电路
    //                 .analog_bias_circuits = true,          // 开启模拟偏置电路
    //                 .analog_adc_bias_circuits = true,      // 开启模拟ADC偏置电路
    //                 .analog_adc_reference_circuits = true, // 开启模拟ADC参考电路
    //                 .analog_dac_reference_circuit = true,  // 开启模拟DAC参考电路
    //                 .internal_reference_circuits = false,  // 关闭内部参考电路
    //             },
    //         .vmid = Cpp_Bus_Driver::Es8311::Vmid::START_UP_VMID_NORMAL_SPEED_CHARGE,
    //     };

    Cpp_Bus_Driver::Es8311::Power_Status ps;
    ps.contorl.analog_circuits = true;               // 开启模拟电路
    ps.contorl.analog_bias_circuits = true;          // 开启模拟偏置电路
    ps.contorl.analog_adc_bias_circuits = true;      // 开启模拟ADC偏置电路
    ps.contorl.analog_adc_reference_circuits = true; // 开启模拟ADC参考电路
    ps.contorl.analog_dac_reference_circuit = true;  // 开启模拟DAC参考电路
    ps.contorl.internal_reference_circuits = false;  // 关闭内部参考电路
    ps.vmid = Cpp_Bus_Driver::Es8311::Vmid::START_UP_VMID_NORMAL_SPEED_CHARGE;

    ES8311->set_power_status(ps);
    ES8311->set_pga_power(true);
    ES8311->set_adc_power(true);
    ES8311->set_dac_power(true);
    ES8311->set_output_to_hp_drive(true);
    ES8311->set_adc_offset_freeze(Cpp_Bus_Driver::Es8311::Adc_Offset_Freeze::DYNAMIC_HPF);
    ES8311->set_adc_hpf_stage2_coeff(10);
    ES8311->set_dac_equalizer(false);

    ES8311->set_mic(Cpp_Bus_Driver::Es8311::Mic_Type::ANALOG_MIC, Cpp_Bus_Driver::Es8311::Mic_Input::MIC1P_1N);
    ES8311->set_adc_auto_volume_control(false);
    ES8311->set_adc_gain(Cpp_Bus_Driver::Es8311::Adc_Gain::GAIN_18DB);
    ES8311->set_adc_pga_gain(Cpp_Bus_Driver::Es8311::Adc_Pga_Gain::GAIN_30DB);

    ES8311->set_adc_volume(191);
    ES8311->set_dac_volume(220);

    // 将ADC的数据自动输出到DAC上
    // ES8311->set_adc_data_to_dac(true);

    // 配置mclk_multiple为256
    // 配置sample_rate为44100
    // 配置data_bit_length为16
    // [2025-03-14 11:35:11.633] es8311 register[0]: 0X84
    // [2025-03-14 11:35:11.633] es8311 register[1]: 0X3F
    // [2025-03-14 11:35:11.633] es8311 register[2]: 0
    // [2025-03-14 11:35:11.633] es8311 register[3]: 0X10
    // [2025-03-14 11:35:11.633] es8311 register[4]: 0X10
    // [2025-03-14 11:35:11.633] es8311 register[5]: 0
    // [2025-03-14 11:35:11.633] es8311 register[6]: 0X3
    // [2025-03-14 11:35:11.633] es8311 register[7]: 0
    // [2025-03-14 11:35:11.633] es8311 register[8]: 0XFF
    // [2025-03-14 11:35:11.633] es8311 register[9]: 0XC
    // [2025-03-14 11:35:11.633] es8311 register[10]: 0XC
    // [2025-03-14 11:35:11.633] es8311 register[11]: 0
    // [2025-03-14 11:35:11.633] es8311 register[12]: 0X20
    // [2025-03-14 11:35:11.633] es8311 register[13]: 0X1
    // [2025-03-14 11:35:11.633] es8311 register[14]: 0XA
    // [2025-03-14 11:35:11.633] es8311 register[15]: 0
    // [2025-03-14 11:35:11.633] es8311 register[16]: 0X13
    // [2025-03-14 11:35:11.633] es8311 register[17]: 0X7C
    // [2025-03-14 11:35:11.633] es8311 register[18]: 0
    // [2025-03-14 11:35:11.633] es8311 register[19]: 0X10
    // [2025-03-14 11:35:11.633] es8311 register[20]: 0X1A
    // [2025-03-14 11:35:11.633] es8311 register[21]: 0
    // [2025-03-14 11:35:11.633] es8311 register[22]: 0X3
    // [2025-03-14 11:35:11.633] es8311 register[23]: 0XBF
    // [2025-03-14 11:35:11.633] es8311 register[24]: 0
    // [2025-03-14 11:35:11.633] es8311 register[25]: 0
    // [2025-03-14 11:35:11.633] es8311 register[26]: 0
    // [2025-03-14 11:35:11.633] es8311 register[27]: 0XC
    // [2025-03-14 11:35:11.633] es8311 register[28]: 0X6A
    // [2025-03-14 11:35:11.633] es8311 register[29]: 0
    // [2025-03-14 11:35:11.633] es8311 register[30]: 0
    // [2025-03-14 11:35:11.633] es8311 register[31]: 0
    // [2025-03-14 11:35:11.633] es8311 register[32]: 0
    // [2025-03-14 11:35:11.633] es8311 register[33]: 0
    // [2025-03-14 11:35:11.633] es8311 register[34]: 0
    // [2025-03-14 11:35:11.633] es8311 register[35]: 0
    // [2025-03-14 11:35:11.633] es8311 register[36]: 0
    // [2025-03-14 11:35:11.633] es8311 register[37]: 0
    // [2025-03-14 11:35:11.633] es8311 register[38]: 0
    // [2025-03-14 11:35:11.633] es8311 register[39]: 0
    // [2025-03-14 11:35:11.633] es8311 register[40]: 0
    // [2025-03-14 11:35:11.633] es8311 register[41]: 0
    // [2025-03-14 11:35:11.633] es8311 register[42]: 0
    // [2025-03-14 11:35:11.633] es8311 register[43]: 0
    // [2025-03-14 11:35:11.633] es8311 register[44]: 0
    // [2025-03-14 11:35:11.633] es8311 register[45]: 0
    // [2025-03-14 11:35:11.633] es8311 register[46]: 0
    // [2025-03-14 11:35:11.633] es8311 register[47]: 0
    // [2025-03-14 11:35:11.633] es8311 register[48]: 0
    // [2025-03-14 11:35:11.633] es8311 register[49]: 0
    // [2025-03-14 11:35:11.633] es8311 register[50]: 0XDC
    // [2025-03-14 11:35:11.633] es8311 register[51]: 0
    // [2025-03-14 11:35:11.633] es8311 register[52]: 0
    // [2025-03-14 11:35:11.633] es8311 register[53]: 0
    // [2025-03-14 11:35:11.633] es8311 register[54]: 0
    // [2025-03-14 11:35:11.633] es8311 register[55]: 0X8
    // [2025-03-14 11:35:11.633] es8311 register[56]: 0
    // [2025-03-14 11:35:11.633] es8311 register[57]: 0
    // [2025-03-14 11:35:11.633] es8311 register[58]: 0
    // [2025-03-14 11:35:11.633] es8311 register[59]: 0
    // [2025-03-14 11:35:11.633] es8311 register[60]: 0
    // [2025-03-14 11:35:11.633] es8311 register[61]: 0
    // [2025-03-14 11:35:11.633] es8311 register[62]: 0
    // [2025-03-14 11:35:11.633] es8311 register[63]: 0
    // [2025-03-14 11:35:11.633] es8311 register[64]: 0
    // [2025-03-14 11:35:11.633] es8311 register[65]: 0
    // [2025-03-14 11:35:11.633] es8311 register[66]: 0
    // [2025-03-14 11:35:11.633] es8311 register[67]: 0
    // [2025-03-14 11:35:11.633] es8311 register[68]: 0
    // [2025-03-14 11:35:11.633] es8311 register[69]: 0
    // [2025-03-14 11:35:11.633] es8311 register[70]: 0
    // [2025-03-14 11:35:11.633] es8311 register[71]: 0
    // [2025-03-14 11:35:11.633] es8311 register[72]: 0
    // [2025-03-14 11:35:11.633] es8311 register[73]: 0
    // [2025-03-14 11:35:11.633] es8311 register[74]: 0
    // [2025-03-14 11:35:11.633] es8311 register[75]: 0
    // [2025-03-14 11:35:11.633] es8311 register[76]: 0
    // [2025-03-14 11:35:11.633] es8311 register[77]: 0
    // [2025-03-14 11:35:11.633] es8311 register[78]: 0
    // [2025-03-14 11:35:11.633] es8311 register[79]: 0
    // [2025-03-14 11:35:11.633] es8311 register[80]: 0
    // [2025-03-14 11:35:11.633] es8311 register[81]: 0
    // [2025-03-14 11:35:11.633] es8311 register[82]: 0XFE
    // [2025-03-14 11:35:11.633] es8311 register[83]: 0
    // [2025-03-14 11:35:11.633] es8311 register[84]: 0
    // [2025-03-14 11:35:11.633] es8311 register[85]: 0
    // [2025-03-14 11:35:11.633] es8311 register[86]: 0
    // [2025-03-14 11:35:11.633] es8311 register[87]: 0
    // [2025-03-14 11:35:11.633] es8311 register[88]: 0
    // [2025-03-14 11:35:11.633] es8311 register[89]: 0
    // [2025-03-14 11:35:11.633] es8311 register[90]: 0
    // [2025-03-14 11:35:11.633] es8311 register[91]: 0
    // [2025-03-14 11:35:11.633] es8311 register[92]: 0
    // [2025-03-14 11:35:11.633] es8311 register[93]: 0
    // [2025-03-14 11:35:11.633] es8311 register[94]: 0
    // [2025-03-14 11:35:11.633] es8311 register[95]: 0
    // [2025-03-14 11:35:11.633] es8311 register[96]: 0
    // [2025-03-14 11:35:11.633] es8311 register[97]: 0
    // [2025-03-14 11:35:11.633] es8311 register[98]: 0
    // [2025-03-14 11:35:11.633] es8311 register[99]: 0
    // [2025-03-14 11:35:11.633] es8311 register[100]: 0
    // [2025-03-14 11:35:11.633] es8311 register[101]: 0
    // [2025-03-14 11:35:11.633] es8311 register[102]: 0
    // [2025-03-14 11:35:11.633] es8311 register[103]: 0
    // [2025-03-14 11:35:11.633] es8311 register[104]: 0
    // [2025-03-14 11:35:11.633] es8311 register[105]: 0
    // [2025-03-14 11:35:11.633] es8311 register[106]: 0
    // [2025-03-14 11:35:11.633] es8311 register[107]: 0
    // [2025-03-14 11:35:11.633] es8311 register[108]: 0
    // [2025-03-14 11:35:11.633] es8311 register[109]: 0
    // [2025-03-14 11:35:11.633] es8311 register[110]: 0
    // [2025-03-14 11:35:11.633] es8311 register[111]: 0
    // [2025-03-14 11:35:11.633] es8311 register[112]: 0
    // [2025-03-14 11:35:11.633] es8311 register[113]: 0
    // [2025-03-14 11:35:11.633] es8311 register[114]: 0
    // [2025-03-14 11:35:11.633] es8311 register[115]: 0
    // [2025-03-14 11:35:11.633] es8311 register[116]: 0
    // [2025-03-14 11:35:11.633] es8311 register[117]: 0
    // [2025-03-14 11:35:11.633] es8311 register[118]: 0
    // [2025-03-14 11:35:11.633] es8311 register[119]: 0
    // [2025-03-14 11:35:11.633] es8311 register[120]: 0
    // [2025-03-14 11:35:11.633] es8311 register[121]: 0
    // [2025-03-14 11:35:11.633] es8311 register[122]: 0
    // [2025-03-14 11:35:11.633] es8311 register[123]: 0
    // [2025-03-14 11:35:11.633] es8311 register[124]: 0
    // [2025-03-14 11:35:11.633] es8311 register[125]: 0
    // [2025-03-14 11:35:11.633] es8311 register[126]: 0
    // [2025-03-14 11:35:11.633] es8311 register[127]: 0
    // [2025-03-14 11:35:11.633] es8311 register[128]: 0
    // [2025-03-14 11:35:11.633] es8311 register[129]: 0
    // [2025-03-14 11:35:11.633] es8311 register[130]: 0
    // [2025-03-14 11:35:11.633] es8311 register[131]: 0
    // [2025-03-14 11:35:11.633] es8311 register[132]: 0
    // [2025-03-14 11:35:11.633] es8311 register[133]: 0
    // [2025-03-14 11:35:11.633] es8311 register[134]: 0
    // [2025-03-14 11:35:11.633] es8311 register[135]: 0
    // [2025-03-14 11:35:11.633] es8311 register[136]: 0
    // [2025-03-14 11:35:11.633] es8311 register[137]: 0
    // [2025-03-14 11:35:11.633] es8311 register[138]: 0
    // [2025-03-14 11:35:11.633] es8311 register[139]: 0
    // [2025-03-14 11:35:11.633] es8311 register[140]: 0
    // [2025-03-14 11:35:11.633] es8311 register[141]: 0
    // [2025-03-14 11:35:11.633] es8311 register[142]: 0
    // [2025-03-14 11:35:11.633] es8311 register[143]: 0
    // [2025-03-14 11:35:11.633] es8311 register[144]: 0
    // [2025-03-14 11:35:11.633] es8311 register[145]: 0
    // [2025-03-14 11:35:11.633] es8311 register[146]: 0
    // [2025-03-14 11:35:11.633] es8311 register[147]: 0
    // [2025-03-14 11:35:11.633] es8311 register[148]: 0
    // [2025-03-14 11:35:11.633] es8311 register[149]: 0
    // [2025-03-14 11:35:11.633] es8311 register[150]: 0
    // [2025-03-14 11:35:11.633] es8311 register[151]: 0
    // [2025-03-14 11:35:11.633] es8311 register[152]: 0
    // [2025-03-14 11:35:11.633] es8311 register[153]: 0
    // [2025-03-14 11:35:11.633] es8311 register[154]: 0
    // [2025-03-14 11:35:11.633] es8311 register[155]: 0
    // [2025-03-14 11:35:11.633] es8311 register[156]: 0
    // [2025-03-14 11:35:11.633] es8311 register[157]: 0
    // [2025-03-14 11:35:11.633] es8311 register[158]: 0
    // [2025-03-14 11:35:11.633] es8311 register[159]: 0
    // [2025-03-14 11:35:11.633] es8311 register[160]: 0
    // [2025-03-14 11:35:11.633] es8311 register[161]: 0
    // [2025-03-14 11:35:11.633] es8311 register[162]: 0
    // [2025-03-14 11:35:11.633] es8311 register[163]: 0
    // [2025-03-14 11:35:11.633] es8311 register[164]: 0
    // [2025-03-14 11:35:11.633] es8311 register[165]: 0
    // [2025-03-14 11:35:11.633] es8311 register[166]: 0
    // [2025-03-14 11:35:11.633] es8311 register[167]: 0
    // [2025-03-14 11:35:11.633] es8311 register[168]: 0
    // [2025-03-14 11:35:11.633] es8311 register[169]: 0
    // [2025-03-14 11:35:11.633] es8311 register[170]: 0
    // [2025-03-14 11:35:11.633] es8311 register[171]: 0
    // [2025-03-14 11:35:11.633] es8311 register[172]: 0
    // [2025-03-14 11:35:11.633] es8311 register[173]: 0
    // [2025-03-14 11:35:11.633] es8311 register[174]: 0
    // [2025-03-14 11:35:11.633] es8311 register[175]: 0
    // [2025-03-14 11:35:11.633] es8311 register[176]: 0
    // [2025-03-14 11:35:11.633] es8311 register[177]: 0
    // [2025-03-14 11:35:11.633] es8311 register[178]: 0
    // [2025-03-14 11:35:11.633] es8311 register[179]: 0
    // [2025-03-14 11:35:11.633] es8311 register[180]: 0
    // [2025-03-14 11:35:11.633] es8311 register[181]: 0
    // [2025-03-14 11:35:11.633] es8311 register[182]: 0
    // [2025-03-14 11:35:11.633] es8311 register[183]: 0
    // [2025-03-14 11:35:11.633] es8311 register[184]: 0
    // [2025-03-14 11:35:11.633] es8311 register[185]: 0
    // [2025-03-14 11:35:11.633] es8311 register[186]: 0
    // [2025-03-14 11:35:11.633] es8311 register[187]: 0
    // [2025-03-14 11:35:11.633] es8311 register[188]: 0
    // [2025-03-14 11:35:11.633] es8311 register[189]: 0
    // [2025-03-14 11:35:11.633] es8311 register[190]: 0
    // [2025-03-14 11:35:11.633] es8311 register[191]: 0
    // [2025-03-14 11:35:11.633] es8311 register[192]: 0
    // [2025-03-14 11:35:11.633] es8311 register[193]: 0
    // [2025-03-14 11:35:11.633] es8311 register[194]: 0
    // [2025-03-14 11:35:11.633] es8311 register[195]: 0
    // [2025-03-14 11:35:11.633] es8311 register[196]: 0
    // [2025-03-14 11:35:11.633] es8311 register[197]: 0
    // [2025-03-14 11:35:11.633] es8311 register[198]: 0
    // [2025-03-14 11:35:11.633] es8311 register[199]: 0
    // [2025-03-14 11:35:11.633] es8311 register[200]: 0
    // [2025-03-14 11:35:11.633] es8311 register[201]: 0
    // [2025-03-14 11:35:11.633] es8311 register[202]: 0
    // [2025-03-14 11:35:11.633] es8311 register[203]: 0
    // [2025-03-14 11:35:11.633] es8311 register[204]: 0
    // [2025-03-14 11:35:11.633] es8311 register[205]: 0
    // [2025-03-14 11:35:11.633] es8311 register[206]: 0
    // [2025-03-14 11:35:11.633] es8311 register[207]: 0
    // [2025-03-14 11:35:11.633] es8311 register[208]: 0
    // [2025-03-14 11:35:11.633] es8311 register[209]: 0
    // [2025-03-14 11:35:11.633] es8311 register[210]: 0
    // [2025-03-14 11:35:11.633] es8311 register[211]: 0
    // [2025-03-14 11:35:11.633] es8311 register[212]: 0
    // [2025-03-14 11:35:11.633] es8311 register[213]: 0
    // [2025-03-14 11:35:11.633] es8311 register[214]: 0
    // [2025-03-14 11:35:11.633] es8311 register[215]: 0
    // [2025-03-14 11:35:11.633] es8311 register[216]: 0
    // [2025-03-14 11:35:11.633] es8311 register[217]: 0
    // [2025-03-14 11:35:11.633] es8311 register[218]: 0
    // [2025-03-14 11:35:11.633] es8311 register[219]: 0
    // [2025-03-14 11:35:11.633] es8311 register[220]: 0
    // [2025-03-14 11:35:11.633] es8311 register[221]: 0
    // [2025-03-14 11:35:11.633] es8311 register[222]: 0
    // [2025-03-14 11:35:11.633] es8311 register[223]: 0
    // [2025-03-14 11:35:11.633] es8311 register[224]: 0
    // [2025-03-14 11:35:11.633] es8311 register[225]: 0
    // [2025-03-14 11:35:11.633] es8311 register[226]: 0
    // [2025-03-14 11:35:11.633] es8311 register[227]: 0
    // [2025-03-14 11:35:11.633] es8311 register[228]: 0
    // [2025-03-14 11:35:11.633] es8311 register[229]: 0
    // [2025-03-14 11:35:11.633] es8311 register[230]: 0
    // [2025-03-14 11:35:11.633] es8311 register[231]: 0
    // [2025-03-14 11:35:11.633] es8311 register[232]: 0
    // [2025-03-14 11:35:11.633] es8311 register[233]: 0
    // [2025-03-14 11:35:11.633] es8311 register[234]: 0
    // [2025-03-14 11:35:11.633] es8311 register[235]: 0
    // [2025-03-14 11:35:11.633] es8311 register[236]: 0
    // [2025-03-14 11:35:11.633] es8311 register[237]: 0
    // [2025-03-14 11:35:11.633] es8311 register[238]: 0
    // [2025-03-14 11:35:11.633] es8311 register[239]: 0
    // [2025-03-14 11:35:11.633] es8311 register[240]: 0
    // [2025-03-14 11:35:11.633] es8311 register[241]: 0
    // [2025-03-14 11:35:11.633] es8311 register[242]: 0
    // [2025-03-14 11:35:11.633] es8311 register[243]: 0
    // [2025-03-14 11:35:11.633] es8311 register[244]: 0
    // [2025-03-14 11:35:11.633] es8311 register[245]: 0
    // [2025-03-14 11:35:11.633] es8311 register[246]: 0
    // [2025-03-14 11:35:11.633] es8311 register[247]: 0
    // [2025-03-14 11:35:11.633] es8311 register[248]: 0
    // [2025-03-14 11:35:11.633] es8311 register[249]: 0
    // [2025-03-14 11:35:11.633] es8311 register[250]: 0
    // [2025-03-14 11:35:11.633] es8311 register[251]: 0
    // [2025-03-14 11:35:11.633] es8311 register[252]: 0X70
    // [2025-03-14 11:35:11.633] es8311 register[253]: 0X83
    // [2025-03-14 11:35:11.633] es8311 register[254]: 0X11
    // [2025-03-14 11:35:11.633] es8311 register[255]: 0X1

    // // 打印所有寄存器
    // uint8_t buffer = 0;
    // for (size_t i = 0; i < 256; i++)
    // {
    //     IIC_Bus_0->Bus_Iic_Guide::read(i, &buffer);
    //     printf("es8311 register[%d]: %#X\n", i, buffer);
    // }
}

void loop()
{
    // Iic_Scan();
    // delay(1000);

    // ADC和DAC相互回环测试
    // size_t data_lenght = 2048;
    // std::shared_ptr<uint16_t[]> data = std::make_shared<uint16_t[]>(data_lenght);
    // if (ES8311->read_data(data.get(), data_lenght * sizeof(uint16_t)) > 0)
    // {
    //     // for (uint8_t i = 0; i < 10; i++)
    //     // {
    //     //     printf("read_data: %d\n", data[i]);
    //     // }

    //     ES8311->write_data(data.get(), data_lenght * sizeof(uint16_t));
    // }

    if (digitalRead(nRF52840_BOOT) == LOW)
    {
        delay(300);
        // Iic_Scan();
        // ES8311->set_adc_data_to_dac(true);
        // uint8_t buffer = 0;
        // for (size_t i = 0; i < 256; i++)
        // {
        //     IIC_Bus_0->Bus_Iic_Guide::read((uint8_t)i, &buffer);
        //     printf("es8311 register[%d]: %#X\n", i, buffer);
        // }

        // 播放音乐测试
        printf("music play start\n");

        Iis_Send_Data_Size = 0;
        Iis_Data_Convert(c2_b16_s44100, Iis_Tx_Buffer, 0, MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t));

        if (ES8311->start_transmit(Iis_Tx_Buffer, nullptr, MAX_IIS_DATA_TRANSMIT_SIZE) == true)
        {
            Iis_Send_Data_Size += MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t);
            Iis_Transmit_Flag = true;
        }
        else
        {
            Iis_Transmit_Flag = false;
            printf("music play fail (ES8311->start_transmit fail)\n");
        }

        Play_Count++;
        printf("play_count: %d\n", Play_Count);
    }

    if (Iis_Transmit_Flag == true)
    {
        if (Iis_Send_Data_Size >= sizeof(c2_b16_s44100))
        {
            printf("music play finish iis_send_data_size: %d\n", Iis_Send_Data_Size);
            ES8311->stop_transmit();

            Iis_Data_Convert_Wait = false;
            Iis_Transmit_Flag = false;
        }
        else
        {
            if (Iis_Data_Convert_Wait == false)
            {
                size_t buffer_length = min(MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t), sizeof(c2_b16_s44100) - Iis_Send_Data_Size);

                // printf("iis_send_data_size: %d\n", Iis_Send_Data_Size);
                // printf("iis send data length: %d\n", buffer_length);

                if (buffer_length != MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t))
                {
                    memset(Iis_Tx_Buffer, 0, MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t));
                }
                Iis_Data_Convert(c2_b16_s44100, Iis_Tx_Buffer, Iis_Send_Data_Size, buffer_length);

                Iis_Send_Data_Size += buffer_length;

                Iis_Data_Convert_Wait = true;
            }
        }

        if (ES8311->get_write_event_flag() == true)
        {
            if (Iis_Data_Convert_Wait == true)
            {
                ES8311->set_next_write_data(Iis_Tx_Buffer);
                Iis_Data_Convert_Wait = false;
            }
        }
    }
}
