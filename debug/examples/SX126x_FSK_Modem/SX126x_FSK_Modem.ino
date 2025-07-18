/*
   RadioLib SX126x FSK Modem Example

   This example shows how to use FSK modem in SX126x chips.

   NOTE: The sketch below is just a guide on how to use
         FSK modem, so this code should not be run directly!
         Instead, modify the other examples to use FSK
         modem and use the appropriate configuration
         methods.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---fsk-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include "RadioLib.h"
#include "pin_config.h"

SPIClass Custom_SPI(NRF_SPIM3, SX1262_MISO, SX1262_SCLK, SX1262_MOSI);

// SX1262 has the following connections:
// NSS pin:   10
// DIO1 pin:  2
// NRST pin:  3
// BUSY pin:  9
SX1262 radio = new Module(SX1262_CS, SX1262_DIO1, SX1262_RST, SX1262_BUSY, Custom_SPI);

// or using RadioShield
// https://github.com/jgromes/RadioShield
// SX1262 radio = RadioShield.ModuleA;

// or using CubeCell
// SX1262 radio = new Module(RADIOLIB_BUILTIN_MODULE);

uint8_t Send_Package[9] = {9, 8, 7, 6, 5, 4, 3, 2, 1};

volatile bool operationDone = false;
void setFlag(void)
{
    // we sent or received  packet, set the flag
    operationDone = true;
}

void Set_SX1262_RF_Transmitter_Switch(bool status)
{
    if (status == true)
    {
        digitalWrite(SX1262_RF_VC1, HIGH); // send
        digitalWrite(SX1262_RF_VC2, LOW);
    }
    else
    {
        digitalWrite(SX1262_RF_VC1, LOW); // receive
        digitalWrite(SX1262_RF_VC2, HIGH);
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(100); // wait for native usb
    }
    Serial.println("Ciallo");

    pinMode(SX1262_RF_VC1, OUTPUT);
    pinMode(SX1262_RF_VC2, OUTPUT);
    Set_SX1262_RF_Transmitter_Switch(false);

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    pinMode(nRF52840_BOOT, INPUT_PULLUP);

    Custom_SPI.begin();
    // initialize SX1262 FSK modem with default settings
    Serial.print(F("[SX1262] Initializing ... "));
    int state = radio.beginFSK();
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
            ;
    }

    // if needed, you can switch between LoRa and FSK modes
    //
    // radio.begin()       start LoRa mode (and disable FSK)
    // radio.beginFSK()    start FSK mode (and disable LoRa)

    // the following settings can also
    // be modified at run-time

    // state = radio.setFrequency(433.5);
    // state = radio.setBitRate(100.0);
    // state = radio.setFrequencyDeviation(10.0);
    // state = radio.setRxBandwidth(250.0);
    // state = radio.setOutputPower(10.0);
    // state = radio.setCurrentLimit(100.0);
    // state = radio.setDataShaping(RADIOLIB_SHAPING_1_0);

    // if (state != RADIOLIB_ERR_NONE)
    // {
    //     Serial.print(F("Unable to set configuration, code "));
    //     Serial.println(state);
    //     while (true)
    //         ;
    // }

    state = radio.setFrequency(868.1);
    state = radio.setPreambleLength(32);
    state = radio.setBitRate(300.0);
    state = radio.setFrequencyDeviation(30.0);
    state = radio.setRxBandwidth(467.0);
    state = radio.setOutputPower(22.0);
    state = radio.setCurrentLimit(140.0);
    state = radio.setDataShaping(RADIOLIB_SHAPING_1_0);
    uint8_t sync[] = {0xAB, 0xCD};
    state = radio.setSyncWord(sync, 2);
    // uint8_t syncWord[] = {0x01, 0x23, 0x45, 0x67,
    //                       0x89, 0xAB, 0xCD, 0xEF};
    // state = radio.setSyncWord(syncWord, 8);
    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.print(F("Unable to set configuration, code "));
        Serial.println(state);
        while (true)
            ;
    }

    // FSK modem on SX126x can handle the sync word setting in bits, not just
    // whole bytes. The value used is left-justified.
    // This makes same result as radio.setSyncWord(syncWord, 8):
    // state = radio.setSyncBits(syncWord, 64);
    // // This will use 0x012 as sync word (12 bits only):
    // state = radio.setSyncBits(syncWord, 12);

    // // FSK modem allows advanced CRC configuration
    // // Default is CCIT CRC16 (2 bytes, initial 0x1D0F, polynomial 0x1021, inverted)
    // // Set CRC to IBM CRC (2 bytes, initial 0xFFFF, polynomial 0x8005, non-inverted)
    state = radio.setCRC(2, 0x1D0F, 0x1021, true);
    // // set CRC length to 0 to disable CRC

    radio.setPacketReceivedAction(setFlag);
    radio.startReceive();

#warning "This sketch is just an API guide! Read the note at line 6."
}

void loop()
{
    // FSK modem can use the same transmit/receive methods
    // as the LoRa modem, even their interrupt-driven versions
    if (digitalRead(nRF52840_BOOT) == LOW)
    {
        // transmit FSK packet
        Set_SX1262_RF_Transmitter_Switch(true);
        int state = radio.transmit(Send_Package, 9);
        delay(10);
        Set_SX1262_RF_Transmitter_Switch(false);

        /*
          byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                            0x89, 0xAB, 0xCD, 0xEF};
          int state = radio.transmit(byteArr, 8);
        */
        if (state == RADIOLIB_ERR_NONE)
        {
            Serial.println(F("[SX1262] Packet transmitted successfully!"));
        }
        else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
        {
            Serial.println(F("[SX1262] Packet too long!"));
        }
        else if (state == RADIOLIB_ERR_TX_TIMEOUT)
        {
            Serial.println(F("[SX1262] Timed out while transmitting!"));
        }
        else
        {
            Serial.println(F("[SX1262] Failed to transmit packet, code "));
            Serial.println(state);
        }
        radio.startReceive();
        operationDone = false;
    }

    if (operationDone)
    {
        uint8_t receive_data[255] = {0};

        int state = radio.readData(receive_data, 9);

        if (state == RADIOLIB_ERR_NONE)
        {
            for (uint8_t i = 0; i < 9; i++)
            {
                printf("[SX1262] data[%d]: %d\n", i, receive_data[i]);
            }
        }
        else if (state == RADIOLIB_ERR_RX_TIMEOUT)
        {
            Serial.println(F("[SX1262] Timed out while waiting for packet!"));
        }
        else if (state == RADIOLIB_ERR_CRC_MISMATCH)
        {
            Serial.println(F("[SX1262] CRC error"));
        }
        else
        {
            Serial.println(F("[SX1262] Failed to receive packet, code "));
            Serial.println(state);
        }

        // reset flag
        operationDone = false;
    }
}
