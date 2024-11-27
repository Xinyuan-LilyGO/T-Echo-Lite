/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2024-10-25 17:57:30
 * @LastEditTime: 2024-10-28 10:36:18
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "pin_config.h"

bool i = 0;

void setup()
{
    Serial.begin(115200);
    // while (!Serial)
    // {
    //     delay(100); // wait for native usb
    // }
    Serial.println("Ciallo");

    pinMode(GPS_UART_RX, OUTPUT);
    pinMode(GPS_UART_TX, OUTPUT);
    pinMode(GPS_1PPS, OUTPUT);
    pinMode(GPS_WAKE_UP, OUTPUT);
}

void loop()
{
    digitalWrite(GPS_UART_RX, i);
    digitalWrite(GPS_UART_TX, i);
    digitalWrite(GPS_1PPS, i);
    digitalWrite(GPS_WAKE_UP, i);

    delay(1000);

    i = !i;
}
