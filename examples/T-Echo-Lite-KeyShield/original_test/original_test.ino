/*
 * @Description: xl9535
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2025-11-06 10:43:25
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include "t_echo_lite_keyshield_config.h"
#include "cpp_bus_driver_library.h"
#include "Adafruit_EPD.h"
#include "Display_Fonts.h"
#include "material_monochrome_176x192px.h"
#include "c2_b16_s44100_2.h"
#include "c2_b16_s44100_3.h"

#define SOFTWARE_NAME "Original_Test"
#define SOFTWARE_LASTEDITTIME "202511051807"
#define BOARD_VERSION "V1.0"

#define MCLK_MULTIPLE 32
#define SAMPLE_RATE 44100
#define MAX_IIS_DATA_TRANSMIT_SIZE 512

std::vector<std::string> Current_Text;

bool Screen_Refresh_Flag = false;

uint32_t Iis_Tx_Buffer[MAX_IIS_DATA_TRANSMIT_SIZE] = {0};
uint32_t Iis_Rx_Buffer[MAX_IIS_DATA_TRANSMIT_SIZE] = {0};

// 已经发送的数据大小
size_t Iis_Send_Data_Size = 0;
bool Iis_Transmit_Flag = false;

bool Iis_Data_Convert_Wait = false;

// volatile bool Interrupt_Flag = false;

size_t Cycle_Time = 0;

bool Fast_Refresh_Flag = true;
size_t Fast_Refresh_Count = 0;

SPIClass Custom_SPI_1(NRF_SPIM1, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC, SCREEN_RST,
                         SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &Custom_SPI_1, 8000000);

auto TCA8418_IIC_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iic_2>(TCA8418_SDA, TCA8418_SCL, &Wire);
auto Aw21009qnr_IIC_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iic_2>(AW21009QNR_SDA, AW21009QNR_SCL, &Wire);
auto ES8311_IIC_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iic_2>(ES8311_SDA, ES8311_SCL, &Wire);
auto AW86224_IIC_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iic_2>(AW86224_SDA, AW86224_SCL, &Wire);

auto ES8311_Iis_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iis>(ES8311_ADC_DATA, ES8311_DAC_DATA, ES8311_WS_LRCK, ES8311_BCLK, ES8311_MCLK);

auto TCA8418 = std::make_unique<Cpp_Bus_Driver::Tca8418>(TCA8418_IIC_Bus, TCA8418_IIC_ADDRESS, DEFAULT_CPP_BUS_DRIVER_VALUE);
auto Aw21009qnr = std::make_unique<Cpp_Bus_Driver::Aw21009xxx>(Aw21009qnr_IIC_Bus, AW21009QNR_IIC_ADDRESS, DEFAULT_CPP_BUS_DRIVER_VALUE);
auto AW86224 = std::make_unique<Cpp_Bus_Driver::Aw862xx>(AW86224_IIC_Bus, AW86224_IIC_ADDRESS, DEFAULT_CPP_BUS_DRIVER_VALUE);

auto ES8311 = std::make_unique<Cpp_Bus_Driver::Es8311>(ES8311_IIC_Bus, ES8311_Iis_Bus, ES8311_IIC_ADDRESS, DEFAULT_CPP_BUS_DRIVER_VALUE);

void Iis_Data_Convert(const void *input_data, void *out_buffer, size_t input_data_start_index, size_t byte)
{
    const uint8_t *input_ptr = (const uint8_t *)input_data + input_data_start_index;
    uint8_t *out_ptr = (uint8_t *)out_buffer;

    memcpy(out_ptr, input_ptr, byte);
}

void vibration_start(void)
{
    AW86224->run_ram_playback_waveform(2, 14, 255);
    delay(30);
}

void microphone_read()
{
    printf("microphone_read start\n");
    if (ES8311->start_transmit(nullptr, Iis_Rx_Buffer, MAX_IIS_DATA_TRANSMIT_SIZE) == false)
    {
        printf("microphone_read fail (ES8311->start_transmit fail)\n");
    }

    printf("music play finish%d\n");
    ES8311->stop_transmit();
}

void music_play(const uint16_t *data, size_t size)
{
    // 播放音乐测试
    printf("music_play start\n");

    Iis_Send_Data_Size = 0;
    Iis_Data_Convert(data, Iis_Tx_Buffer, 0, MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t));

    if (ES8311->start_transmit(Iis_Tx_Buffer, nullptr, MAX_IIS_DATA_TRANSMIT_SIZE) == true)
    {
        Iis_Send_Data_Size += MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t);
        Iis_Transmit_Flag = true;
    }
    else
    {
        Iis_Transmit_Flag = false;
        printf("music_play fail (ES8311->start_transmit fail)\n");
    }

    while (1)
    {
        if (Iis_Transmit_Flag == true)
        {
            if (Iis_Send_Data_Size >= size)
            {
                printf("music_play finish iis_send_data_size: %d\n", Iis_Send_Data_Size);
                ES8311->stop_transmit();

                Iis_Data_Convert_Wait = false;
                Iis_Transmit_Flag = false;
                break;
            }
            else
            {
                if (Iis_Data_Convert_Wait == false)
                {
                    size_t buffer_length = min(MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t), size - Iis_Send_Data_Size);

                    // printf("iis_send_data_size: %d\n", Iis_Send_Data_Size);
                    // printf("iis send data length: %d\n", buffer_length);

                    if (buffer_length != MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t))
                    {
                        memset(Iis_Tx_Buffer, 0, MAX_IIS_DATA_TRANSMIT_SIZE * sizeof(uint32_t));
                    }
                    Iis_Data_Convert(data, Iis_Tx_Buffer, Iis_Send_Data_Size, buffer_length);

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
}

void Es8311_Init(void)
{
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
    Serial.println("[T-Echo-Lite-KeyShield_" + (String)BOARD_VERSION "][" + (String)SOFTWARE_NAME +
                   "]_firmware_" + (String)SOFTWARE_LASTEDITTIME);

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);
    delay(100);
    digitalWrite(RT9080_EN, LOW);
    delay(100);
    digitalWrite(RT9080_EN, HIGH);
    delay(1000);

    pinMode(SCREEN_BS1, OUTPUT);
    digitalWrite(SCREEN_BS1, LOW);

    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, HIGH);

    pinMode(TCA8418_INT, INPUT_PULLUP);
    // attachInterrupt(TCA8418_INT, []() -> void
    //                 { Interrupt_Flag = true; }, FALLING);

    // Measure battery
    pinMode(BATTERY_ADC_DATA, INPUT);
    pinMode(BATTERY_MEASUREMENT_CONTROL, OUTPUT);
    digitalWrite(BATTERY_MEASUREMENT_CONTROL, HIGH);

    // Set the analog reference to 3.0V (default = 3.6V)
    analogReference(AR_INTERNAL_3_0);
    // Set the resolution to 12-bit (0..4095)
    analogReadResolution(12); // Can be 8, 10, 12 or 14

    TCA8418->begin();
    TCA8418->set_keypad_scan_window(0, 0, TCA8418_KEYPAD_SCAN_WIDTH, TCA8418_KEYPAD_SCAN_HEIGHT);
    TCA8418->set_irq_pin_mode(Cpp_Bus_Driver::Tca8418::Irq_Mask::KEY_EVENTS);
    TCA8418->clear_irq_flag(Cpp_Bus_Driver::Tca8418::Irq_Flag::KEY_EVENTS);

    Aw21009qnr->begin();

    AW86224->begin(500000);

    AW86224->init_ram_mode(Cpp_Bus_Driver::aw862xx_haptic_ram_12k_0809_170, sizeof(Cpp_Bus_Driver::aw862xx_haptic_ram_12k_0809_170));

    Es8311_Init();

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
    display.print("Keyboard: Tca8418");
    display.setCursor(25, 120);
    display.print("Dac: Es8311");
    display.setCursor(25, 130);
    display.print("Vibration: Aw86224");

    display.setCursor(25, 150);
    display.print("Software: " + (String)SOFTWARE_NAME);
    display.setCursor(25, 160);
    display.print("LastEditTime: " + (String)SOFTWARE_LASTEDITTIME);

    display.display(display.update_mode::FULL_REFRESH, true);

    music_play(c2_b16_s44100_2, sizeof(c2_b16_s44100_2));

    delay(3000);

    vibration_start();

    Aw21009qnr->set_brightness(Cpp_Bus_Driver::Aw21009xxx::Led_Channel::ALL, 4096);

    Screen_Refresh_Flag = true;
}

void loop()
{
    if (digitalRead(TCA8418_INT) == LOW)
    {
        // if (Interrupt_Flag == true)
        // {
        Cpp_Bus_Driver::Tca8418::Irq_Status is;

        if (TCA8418->parse_irq_status(TCA8418->get_irq_flag(), is) == false)
        {
            printf("parse_irq_status fail\n");
        }
        else
        {
            if (is.key_events_flag == true)
            {
                Cpp_Bus_Driver::Tca8418::Touch_Point tp;
                if (TCA8418->get_multiple_touch_point(tp) == true)
                {
                    printf("touch finger: %d\n", tp.finger_count);

                    for (uint8_t i = 0; i < tp.info.size(); i++)
                    {
                        switch (tp.info[i].event_type)
                        {
                        case Cpp_Bus_Driver::Tca8418::Event_Type::KEYPAD:
                        {
                            Cpp_Bus_Driver::Tca8418::Touch_Position tp_2;
                            if (TCA8418->parse_touch_num(tp.info[i].num, tp_2) == true)
                            {
                                printf("keypad event\n");
                                printf("   touch num:[%d] num: %d x: %d y: %d press_flag: %d\n", i + 1, tp.info[i].num, tp_2.x, tp_2.y, tp.info[i].press_flag);
                                if (tp.info[i].num <= (sizeof(Tca8418_Map) / sizeof(std::string)))
                                {
                                    printf("   touch string: %s\n", Tca8418_Map[tp.info[i].num - 1].c_str());

                                    if (tp.info[i].press_flag == true)
                                    {
                                        if (Current_Text.size() > 7)
                                        {
                                            Current_Text.erase(Current_Text.begin());
                                        }

                                        Current_Text.push_back(Tca8418_Map[tp.info[i].num - 1].c_str());

                                        if (Tca8418_Map[tp.info[i].num - 1] == "Home")
                                        {
                                            float voltage = (((float)analogRead(BATTERY_ADC_DATA) * (3000.0f / 4096.0f)) / 1000.0f) * 2.0f;
                                            char voltage_str[32];
                                            snprintf(voltage_str, sizeof(voltage_str), "Bat:%.3fv", voltage);

                                            if (Current_Text.size() > 7)
                                            {
                                                Current_Text.erase(Current_Text.begin());
                                            }
                                            Current_Text.push_back(voltage_str);

                                            microphone_read();

                                            int16_t buffer_microphone = Iis_Rx_Buffer[0];
                                            char microphone_str[32];
                                            snprintf(microphone_str, sizeof(microphone_str), "Mic:%d", buffer_microphone);

                                            if (Current_Text.size() > 7)
                                            {
                                                Current_Text.erase(Current_Text.begin());
                                            }
                                            Current_Text.push_back(microphone_str);
                                        }

                                        music_play(c2_b16_s44100_3, sizeof(c2_b16_s44100_3));
                                        vibration_start();

                                        Fast_Refresh_Count++;
                                        if (Fast_Refresh_Count < 5)
                                        {
                                            Fast_Refresh_Flag = true;
                                        }
                                        else
                                        {
                                            Fast_Refresh_Flag = false;
                                            Fast_Refresh_Count = 0;
                                        }

                                        Screen_Refresh_Flag = true;
                                        Cycle_Time = millis() + 300;
                                    }
                                }
                            }

                            break;
                        }
                        case Cpp_Bus_Driver::Tca8418::Event_Type::GPIO:
                            printf("gpio event\n");
                            printf("   touch num:[%d] num: %d press_flag: %d\n", i + 1, tp.info[i].num, tp.info[i].press_flag);
                            break;

                        default:
                            break;
                        }
                    }
                }

                TCA8418->clear_irq_flag(Cpp_Bus_Driver::Tca8418::Irq_Flag::KEY_EVENTS);
            }
        }

        //     Interrupt_Flag = false;
        // }
    }

    if (Screen_Refresh_Flag == true)
    {
        if (millis() > Cycle_Time)
        {
            if (Current_Text.size() == 0)
            {
                display.fillScreen(EPD_WHITE);
                display.clearBuffer();
                display.setTextColor(EPD_BLACK);
                display.setTextSize(1);
                display.setFont(&FreeSans9pt7b);
                display.setCursor(15, 70);
                display.print("Please enter the text");

                display.display(display.update_mode::FULL_REFRESH, true);
            }
            else
            {
                std::string show_text;
                for (uint8_t i = 0; i < Current_Text.size(); i++)
                {
                    show_text += "[" + Current_Text[i] + "]\n";
                }
                display.fillScreen(EPD_WHITE);
                display.clearBuffer();
                display.setTextColor(EPD_BLACK);
                display.setTextSize(1);
                display.setFont(&FreeSans9pt7b);
                display.setCursor(0, 13);
                display.print(show_text.c_str());

                if (Fast_Refresh_Flag == true)
                {
                    display.display(display.update_mode::PARTIAL_REFRESH, true, false);
                }
                else
                {
                    display.display(display.update_mode::FAST_REFRESH, true, false);
                }
                // display.display(display.update_mode::FAST_REFRESH, true, false);
            }

            Screen_Refresh_Flag = false;
        }
    }

    delay(10);
}
