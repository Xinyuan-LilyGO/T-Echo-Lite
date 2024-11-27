/*
 * @Description: IIC Scan
 * @Author: LILYGO_L
 * @Date: 2024-03-26 15:51:59
 * @LastEditTime: 2024-10-29 17:35:17
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include "pin_config.h"

#define SDA ICM20948_SDA
#define SCL ICM20948_SCL

void scan_i2c_device(TwoWire &i2c)
{
    Serial.println("Scanning for I2C devices ...");
    Serial.print("      ");
    for (int i = 0; i < 0x10; i++)
    {
        Serial.printf("0x%02X|", i);
    }
    uint8_t error;
    for (int j = 0; j < 0x80; j += 0x10)
    {
        Serial.println();
        Serial.printf("0x%02X |", j);
        for (int i = 0; i < 0x10; i++)
        {
            Wire.beginTransmission(i | j);
            error = Wire.endTransmission();
            if (error == 0)
                Serial.printf("0x%02X|", i | j);
            else
                Serial.print(" -- |");
        }
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    Wire.setPins(SDA, SCL);
    Wire.begin();
    scan_i2c_device(Wire);
}

void loop()
{
    scan_i2c_device(Wire);
    delay(1000);
}