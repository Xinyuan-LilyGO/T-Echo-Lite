/*
 * @Description: Ink screen test
 * @Author: LILYGO_L
 * @Date: 2024-08-07 17:27:50
 * @LastEditTime: 2024-11-25 17:27:56
 * @License: GPL 3.0
 */
#include "Adafruit_EPD.h"
#include "pin_config.h"

static size_t Count = 0;

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

    display.begin();
    display.setRotation(1);
    display.fillScreen(EPD_WHITE);
    display.setTextColor(EPD_BLACK);
    display.display(display.update_mode::FULL_REFRESH, true);
}

void loop()
{
    display.fillScreen(EPD_WHITE);
    display.setCursor(10, 60);
    display.setTextColor(EPD_BLACK);
    display.setTextSize(5);
    display.printf("%d", Count);

    if (Count % 3 == 0 && Count != 0)
    {
        display.display(display.update_mode::FULL_REFRESH, true);
    }
    else
    {
        display.display(display.update_mode::FAST_REFRESH, true);
        // display.display(display.update_mode::PARTIAL_REFRESH, true);
        delay(5000);
    }
    // delay(1000);
    Count++;
}