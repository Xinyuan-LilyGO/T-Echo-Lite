/*
 * @Description: Key trigger test
 * @Author: LILYGO_L
 * @Date: 2024-08-07 17:27:50
 * @LastEditTime: 2024-11-25 17:25:33
 * @License: GPL 3.0
 */
#include "Adafruit_EPD.h"
#include "pin_config.h"

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
};

SPIClass Custom_SPI(NRF_SPIM3, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC, SCREEN_RST,
                         SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &Custom_SPI, 32000000);

Button_Triggered_Operator Button_Triggered_OP;

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

void setup()
{
    Serial.begin(115200);
    // while (!Serial)
    // {
    //     delay(100);
    // }
    Serial.println("Ciallo");

    pinMode(nRF52840_BOOT, INPUT_PULLUP);

    display.begin();
    display.setRotation(1);

    display.fillScreen(EPD_WHITE);
    display.setCursor(10, 60);
    display.setTextColor(EPD_BLACK);
    display.setTextSize(2);

    display.print("Please press the button");

    display.display(display.update_mode::FULL_REFRESH, true);
}

void loop()
{
    if (Key_Scanning() == true)
    {
        display.fillScreen(EPD_WHITE);
        display.setCursor(10, 60);
        display.setTextColor(EPD_BLACK);
        display.setTextSize(2);

        switch (Button_Triggered_OP.current_state)
        {
        case Button_Triggered_OP.gesture::SINGLE_CLICK:
            Serial.println("Key triggered: SINGLE_CLICK");

            display.print("1.SINGLE_CLICK");

            // delay(1000);
            break;
        case Button_Triggered_OP.gesture::DOUBLE_CLICK:
            Serial.println("Key triggered: DOUBLE_CLICK");

            display.print("2.DOUBLE_CLICK");

            // delay(1000);
            break;
        case Button_Triggered_OP.gesture::LONG_PRESS:
            Serial.println("Key triggered: LONG_PRESS");

            display.print("3.LONG_PRESS");

            // delay(1000);
            break;

        default:
            break;
        }

        display.display(display.update_mode::FAST_REFRESH, true);
    }
}
