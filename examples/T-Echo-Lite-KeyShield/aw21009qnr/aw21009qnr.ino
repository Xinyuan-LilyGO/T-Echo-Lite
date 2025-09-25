/*
 * @Description: xl9535
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2025-09-24 15:16:52
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include "t_echo_lite_keyshield_config.h"
#include "cpp_bus_driver_library.h"

auto Aw21009qnr_IIC_Bus = std::make_shared<Cpp_Bus_Driver::Hardware_Iic_2>(AW21009QNR_SDA, AW21009QNR_SCL, &Wire);

auto Aw21009qnr = std::make_unique<Cpp_Bus_Driver::Aw21009xxx>(Aw21009qnr_IIC_Bus, AW21009QNR_IIC_ADDRESS, DEFAULT_CPP_BUS_DRIVER_VALUE);

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
    delay(100);
    digitalWrite(RT9080_EN, LOW);
    delay(100);
    digitalWrite(RT9080_EN, HIGH);
    delay(1000);
    
    Aw21009qnr->begin();

    // Aw21009qnr->set_auto_power_save(true);
    // Aw21009qnr->set_chip_enable(true);
    // Aw21009qnr->set_global_current_limit(255);
    // Aw21009qnr->set_current_limit(Cpp_Bus_Driver::Aw21009xxx::Led_Channel::ALL, 255);

    Aw21009qnr->set_brightness(Cpp_Bus_Driver::Aw21009xxx::Led_Channel::ALL, 500);
}

void loop()
{

    delay(10);
}
