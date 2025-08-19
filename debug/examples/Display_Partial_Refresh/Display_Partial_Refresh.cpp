/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2024-08-09 17:20:53
 * @LastEditTime: 2024-10-16 15:44:46
 * @License: GPL 3.0
 */

#include "GxEPD2_BW.h"
#include "Fonts.h"
#include "t_echo_lite_config.h"
#include "Adafruit_EPD.h"

static size_t Count = 0;

SPIClass Custom_SPI(NRF_SPIM3, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
GxEPD2_BW<GxEPD2_122_GDEM0122T61, GxEPD2_122_GDEM0122T61::HEIGHT>
    display(GxEPD2_122_GDEM0122T61(SCREEN_CS, SCREEN_DC, SCREEN_RST, SCREEN_BUSY));

void GxEPD2_Quick_Refresh_Printf(int16_t x, int16_t y, const char *format, ...)
{
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);

        char buf[256];
        int len;
        va_list ap;
        va_start(ap, format);
        len = vsnprintf(buf, 256, format, ap);
        display.write(buf, len);
        va_end(ap);

    } while (display.nextPage());
}

void GxEPD2_Partial_Refresh_Printf(int16_t x, int16_t y, int16_t w, int16_t h, const char *format, ...)
{
    display.setPartialWindow(x, y, w, h);
    display.firstPage();
    do
    {
        // display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);

        char buf[256];
        int len;
        va_list ap;
        va_start(ap, format);
        len = vsnprintf(buf, 256, format, ap);
        display.write(buf, len);
        va_end(ap);

    } while (display.nextPage());
}

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

    display.init(0, true, 10, false, Custom_SPI, SPISettings(32000000, MSBFIRST, SPI_MODE0));
    display.setRotation(0);
    display.setTextSize(2);
    display.display(false);
}

void loop()
{
    GxEPD2_Quick_Refresh_Printf(10, 80, "Ciallo:%d", Count);
    Count++;
    delay(2000);
}
