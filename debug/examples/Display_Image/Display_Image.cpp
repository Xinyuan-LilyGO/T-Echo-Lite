/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2024-08-07 17:27:50
 * @LastEditTime: 2024-10-16 14:32:03
 * @License: GPL 3.0
 */
#include "Adafruit_EPD.h"
#include "t_echo_lite_config.h"
#include "material_monochrome_176x192px.h"
#include "Display_Fonts.h"

SPIClass Custom_SPI(NRF_SPIM0, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
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
    Serial.println("Adafruit EPD test");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    display.begin();
    display.setRotation(1);

    // display.fillScreen(EPD_WHITE);
    // display.clearBuffer();
    // display.display(display.update_mode::FULL_REFRESH, true);
}

void loop()
{
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
    display.print("LoRa: SX1262");
    display.setCursor(25, 120);
    display.print("Flash: ZD25WQ32C(4MB)");

    display.display(display.update_mode::FULL_REFRESH, true);

    delay(5000);
}
