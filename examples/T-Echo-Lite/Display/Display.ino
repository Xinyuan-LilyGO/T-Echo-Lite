/*
 * @Description: Ink screen test
 * @Author: LILYGO_L
 * @Date: 2024-08-07 17:27:50
 * @LastEditTime: 2025-11-14 10:33:50
 * @License: GPL 3.0
 */
#include "Adafruit_EPD.h"
#include "t_echo_lite_config.h"
#include "Display_Fonts.h"

static size_t Count = 0;

bool Screen_Partial_Refresh_Init_Lock = false;

SPIClass Custom_SPI(NRF_SPIM3, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC, SCREEN_RST,
                         SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &Custom_SPI, 8000000);

void setup()
{
    Serial.begin(115200);
    // while (!Serial)
    // {
    //     delay(100);
    // }
    Serial.println("Ciallo");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    pinMode(SCREEN_BS1, OUTPUT);
    digitalWrite(SCREEN_BS1, LOW);

    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, HIGH);

    display.begin();
    display.setRotation(1);
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(3);
    display.fillScreen(EPD_WHITE);
    display.setTextColor(EPD_BLACK);
    display.display(display.Update_Mode::FULL_REFRESH, true);
}

void loop()
{
    display.fillScreen(EPD_WHITE);
    display.setCursor(10, 60);
    display.setTextColor(EPD_BLACK);
    display.printf("%d", Count);

    if (Count % 10 == 0 && Count != 0)
    {
        // display.display(display.Update_Mode::FULL_REFRESH, true);
        display.display(display.Update_Mode::FAST_REFRESH, true);
        Screen_Partial_Refresh_Init_Lock = false;
    }
    else
    {
        if (Screen_Partial_Refresh_Init_Lock == false)
        {
            display.setRAMValueBaseMap(display.Update_Mode::FAST_REFRESH);
            Screen_Partial_Refresh_Init_Lock = true;
        }
        display.display(display.Update_Mode::PARTIAL_REFRESH, true);
    }
    delay(1000);

    Count++;
    if ((Count % 2) == 0)
    {
        digitalWrite(LED_1, HIGH);
        digitalWrite(LED_2, HIGH);
        digitalWrite(LED_3, HIGH);
        Serial.println("LED ON");
    }
    else
    {
        digitalWrite(LED_1, LOW);
        digitalWrite(LED_2, LOW);
        digitalWrite(LED_3, LOW);
        Serial.println("LED OFF");
    }
}
