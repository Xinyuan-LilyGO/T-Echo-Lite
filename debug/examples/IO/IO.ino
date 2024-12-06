/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2024-10-25 17:57:30
 * @LastEditTime: 2024-11-29 15:11:56
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

    // pinMode(GPS_UART_RX, OUTPUT);
    // pinMode(GPS_UART_TX, OUTPUT);
    // pinMode(GPS_1PPS, OUTPUT);
    // pinMode(GPS_WAKE_UP, OUTPUT);

    pinMode(SCREEN_RST, OUTPUT);
}

void loop()
{
    // digitalWrite(SCREEN_RST, i);

    // delay(1000);

    // i = !i;

    // Setup reset pin direction
    pinMode(SCREEN_RST, OUTPUT);
    // VDD (3.3V) goes high at start, lets just chill for a ms
    digitalWrite(SCREEN_RST, HIGH);
    delay(10);
    // bring reset low
    digitalWrite(SCREEN_RST, LOW);
    // wait 10ms
    delay(10);
    // bring out of reset
    digitalWrite(SCREEN_RST, HIGH);
    delay(10);

    delay(3000);
}
