/*
 * @Description: GPS test
 * @Author: LILYGO_L
 * @Date: 2024-10-25 17:57:30
 * @LastEditTime: 2025-03-13 17:57:07
 * @License: GPL 3.0
 */
#include "TinyGPSPlus.h"
#include "pin_config.h"
#include <Adafruit_TinyUSB.h>

// The TinyGPSPlus object
TinyGPSPlus gps;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(100); // wait for native usb
    }
    Serial.println("Ciallo");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    Serial2.setPins(GPS_UART_RX, GPS_UART_TX);
    Serial2.begin(9600);

    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);
    pinMode(GPS_RT9080_EN, OUTPUT);
    digitalWrite(GPS_RT9080_EN, HIGH);

    pinMode(GPS_1PPS, INPUT);
    pinMode(GPS_WAKE_UP, OUTPUT);
    digitalWrite(GPS_WAKE_UP, HIGH);

    Serial.println(F("DeviceExample.ino"));
    Serial.println(F("A simple demonstration of TinyGPSPlus with an attached GPS module"));
    Serial.print(F("Testing TinyGPSPlus library v. "));
    Serial.println(TinyGPSPlus::libraryVersion());
    Serial.println(F("by Mikal Hart"));
    Serial.println();
}

void loop()
{
    // This sketch displays information every time a new sentence is correctly encoded.
    while (Serial2.available() > 0)
        if (gps.encode(Serial2.read()))
            displayInfo();

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
        Serial.println(F("No GPS detected: check wiring."));
        // while (true)
        //     ;

        delay(1000);
    }
}

void displayInfo()
{
    Serial.print(F("Location: "));
    if (gps.location.isValid())
    {
        Serial.print(gps.location.lat(), 6);
        Serial.print(F(","));
        Serial.print(gps.location.lng(), 6);
    }
    else
    {
        Serial.print(F("INVALID"));
    }

    Serial.print(F("  Date/Time: "));
    if (gps.date.isValid())
    {
        Serial.print(gps.date.month());
        Serial.print(F("/"));
        Serial.print(gps.date.day());
        Serial.print(F("/"));
        Serial.print(gps.date.year());
    }
    else
    {
        Serial.print(F("INVALID"));
    }

    Serial.print(F(" "));
    if (gps.time.isValid())
    {
        if (gps.time.hour() < 10)
            Serial.print(F("0"));
        Serial.print(gps.time.hour());
        Serial.print(F(":"));
        if (gps.time.minute() < 10)
            Serial.print(F("0"));
        Serial.print(gps.time.minute());
        Serial.print(F(":"));
        if (gps.time.second() < 10)
            Serial.print(F("0"));
        Serial.print(gps.time.second());
        Serial.print(F("."));
        if (gps.time.centisecond() < 10)
            Serial.print(F("0"));
        Serial.print(gps.time.centisecond());
    }
    else
    {
        Serial.print(F("INVALID"));
    }

    Serial.println();
}
