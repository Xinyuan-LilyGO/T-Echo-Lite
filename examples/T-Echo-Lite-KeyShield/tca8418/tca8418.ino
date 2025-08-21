/*
 * @Description: xl9535
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2025-08-20 16:03:50
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include "t_echo_lite_keyshield_config.h"
#include "cpp_bus_driver_library.h"

auto TCA8418_IIC_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iic_2>(TCA8418_SDA, TCA8418_SCL, &Wire);

auto TCA8418 = std::make_unique<Cpp_Bus_Driver::Tca8418>(TCA8418_IIC_Bus, TCA8418_IIC_ADDRESS, DEFAULT_CPP_BUS_DRIVER_VALUE);

volatile bool Interrupt_Flag = false;

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

    pinMode(TCA8418_INT, INPUT_PULLUP);
    attachInterrupt(TCA8418_INT, []() -> void
                    { Interrupt_Flag = true; }, FALLING);

    TCA8418->begin();
    TCA8418->set_keypad_scan_window(0, 0, TCA8418_KEYPAD_SCAN_WIDTH, TCA8418_KEYPAD_SCAN_HEIGHT);
    TCA8418->set_irq_pin_mode(Cpp_Bus_Driver::Tca8418::Irq_Mask::KEY_EVENTS);
    TCA8418->clear_irq_flag(Cpp_Bus_Driver::Tca8418::Irq_Flag::KEY_EVENTS);
}

void loop()
{
    if (Interrupt_Flag == true)
    {
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

        Interrupt_Flag = false;
    }

    delay(10);
}
