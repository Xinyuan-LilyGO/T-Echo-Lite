/*
 * @Description: Battery voltage testing program
 * @Author: LILYGO_L
 * @Date: 2024-11-07 12:04:52
 * @LastEditTime: 2025-06-05 14:51:04
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "pin_config.h"

bool Temp = false;

size_t CycleTime = 0;

volatile bool KET1_Triggered_Flag = false;

void External_Interrupt_Triggered()
{
    KET1_Triggered_Flag = true;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    pinMode(nRF52840_BOOT, INPUT_PULLUP);

    // // Breathing lamp
    // HwPWM0.addPin(LED_1);
    // HwPWM0.begin();
    // HwPWM0.setResolution(15);
    // HwPWM0.setClockDiv(PWM_PRESCALER_PRESCALER_DIV_1); // freq = 16Mhz
    // HwPWM0.writePin(LED_1, bit(15) - 1, false);        // Turn off the breathing light

    // Turn off LED
    pinMode(LED_1, OUTPUT);
    digitalWrite(LED_1, HIGH);

    // Measure battery
    pinMode(BATTERY_ADC_DATA, INPUT);
    pinMode(BATTERY_MEASUREMENT_CONTROL, OUTPUT);
    digitalWrite(BATTERY_MEASUREMENT_CONTROL, LOW); // Turn off battery voltage measurement

    attachInterrupt(nRF52840_BOOT, External_Interrupt_Triggered, FALLING);

    // Set the analog reference to 3.0V (default = 3.6V)
    analogReference(AR_INTERNAL_3_0);
    // Set the resolution to 12-bit (0..4095)
    analogReadResolution(12); // Can be 8, 10, 12 or 14
}

void loop()
{
    if (KET1_Triggered_Flag == true)
    {
        delay(300);

        KET1_Triggered_Flag = false;

        // Serial.println("KEY1_Triggered");

        Temp = !Temp;
    }

    if (Temp == false)
    {
        if (millis() > CycleTime)
        {
            digitalWrite(LED_1, HIGH);

            digitalWrite(BATTERY_MEASUREMENT_CONTROL, LOW); // Turn off battery voltage measurement
            Serial.print("Turn off battery voltage measurement\n");

            Serial.print("ADC Value:");
            Serial.println(analogRead(BATTERY_ADC_DATA));

            Serial.printf("ADC Voltage: %.03f V\n", ((float)analogRead(BATTERY_ADC_DATA) * ((3000.0 / 4096.0))) / 1000.0);

            Serial.printf("Battery Voltage: %.03f V\n", (((float)analogRead(BATTERY_ADC_DATA) * ((3000.0 / 4096.0))) / 1000.0) * 2.0);
            Serial.println();

            CycleTime = millis() + 3000;
        }
    }
    else
    {
        if (millis() > CycleTime)
        {
            digitalWrite(LED_1, LOW);

            digitalWrite(BATTERY_MEASUREMENT_CONTROL, HIGH); // Enable battery voltage measurement
            Serial.print("Turn on battery voltage measurement\n");

            Serial.print("ADC Value:");
            Serial.println(analogRead(BATTERY_ADC_DATA));

            Serial.printf("ADC Voltage: %.03f V\n", ((float)analogRead(BATTERY_ADC_DATA) * ((3000.0 / 4096.0))) / 1000.0);

            Serial.printf("Battery Voltage: %.03f V\n", (((float)analogRead(BATTERY_ADC_DATA) * ((3000.0 / 4096.0))) / 1000.0) * 2.0);
            Serial.println();

            CycleTime = millis() + 3000;
        }

        // const int maxValue = bit(15) - 1;

        // for (int i = maxValue; i > 0; i -= 1024)
        // {
        //     HwPWM0.writePin(LED_1, i, false);
        //     delay(5);
        // }
        // for (int i = 0; i <= maxValue; i += 1024)
        // {
        //     HwPWM0.writePin(LED_1, i, false);
        //     delay(2);
        // }
    }
}