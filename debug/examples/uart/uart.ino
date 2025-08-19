/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2024-10-25 17:57:30
 * @LastEditTime: 2025-06-05 17:50:26
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "t_echo_lite_config.h"

size_t CycleTime = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    Serial2.setPins(_PINNUM(0, 8), _PINNUM(0, 6));
    Serial2.begin(115200);`
}

void loop()
{
    while (Serial2.available() > 0)
    {
        Serial2.printf("%c",Serial2.read());
    }

    if (millis() > CycleTime)
    {
        Serial.println("Ciallo");
        Serial2.println("Ciallo");
        CycleTime = millis() + 1000;
    }

    // delay(3000);
}
