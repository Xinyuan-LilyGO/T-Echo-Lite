/*
 * @Description: None
 * @Author: None
 * @Date: 2023-09-12 17:09:04
 * @LastEditTime: 2024-11-25 17:25:48
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

void Chip_Scan(void)
{
    Serial.printf("\n------------------------------------------------\n");

    Serial.print("Chip Mac ID[0]: ");
    Serial.print((uint32_t)NRF_FICR->DEVICEID[0]);
    Serial.println();
    Serial.print("Chip Mac ID[1]: ");
    Serial.print((uint32_t)NRF_FICR->DEVICEID[1]);
    Serial.println();

    // uint64_t temp = (uint64_t)(((uint64_t)NRF_FICR->DEVICEID[1]) << 32 | (uint32_t)NRF_FICR->DEVICEID[0]);

    Serial.printf("------------------------------------------------\n");
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");
}

void loop()
{
    Chip_Scan();
    delay(1000);
}