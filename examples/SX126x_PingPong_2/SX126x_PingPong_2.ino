/*
 * @Description: RadioLib SX126x Ping-Pong 2 Example
 * @Author: LILYGO_L
 * @Date: 2024-11-07 12:04:52
 * @LastEditTime: 2025-06-05 15:06:10
 * @License: GPL 3.0
 */
#include "RadioLib.h"
#include "pin_config.h"

// uncomment the following only on one
// of the nodes to initiate the pings
// #define INITIATING_NODE

static const uint32_t Local_MAC[2] =
    {
        NRF_FICR->DEVICEID[0],
        NRF_FICR->DEVICEID[1],
};

size_t CycleTime = 0;

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

uint8_t Receive_Package[16];
uint32_t Receive_Data = 0;

uint8_t Send_Package[16] = {'M', 'A', 'C', ':',
                            (uint8_t)(Local_MAC[1] >> 24), (uint8_t)(Local_MAC[1] >> 16),
                            (uint8_t)(Local_MAC[1] >> 8), (uint8_t)(Local_MAC[1]),
                            (uint8_t)(Local_MAC[0] >> 24), (uint8_t)(Local_MAC[0] >> 16),
                            (uint8_t)(Local_MAC[0] >> 8), (uint8_t)Local_MAC[0],
                            0, 0, 0, 0};

uint32_t Send_Data = 0;
bool Send_Flag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;

void setFlag(void)
{
    // we sent or received a packet, set the flag
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

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    pinMode(SX1262_RF_VC1, OUTPUT);
    pinMode(SX1262_RF_VC2, OUTPUT);
    Set_SX1262_RF_Transmitter_Switch(false);

    pinMode(nRF52840_BOOT, INPUT_PULLUP);

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    // initialize SX1262 with default settings
    Serial.println("[SX1262] Initializing ... ");
    Custom_SPI.begin();
    while (radio.begin() != RADIOLIB_ERR_NONE)
    {
        Serial.println("SX1262 initialization failed");
        delay(1000);
    }
    Serial.println("SX1262 initialization successful");

    radio.setFrequency(914.9);
    // radio.setFrequency(868.6);
    radio.setBandwidth(500.0);
    radio.setSpreadingFactor(12);
    radio.setCodingRate(8);
    radio.setSyncWord(0xAB);
    radio.setOutputPower(22);
    radio.setCurrentLimit(140);
    radio.setPreambleLength(16);
    radio.setCRC(false);

    // set the function that will be called
    // when new packet is received
    radio.setDio1Action(setFlag);

    Send_Flag = true;
    CycleTime = millis() + 1000;

    // // start listening for LoRa packets on this node
    // Serial.println("[SX1262] Starting to listen ... ");
    // state = radio.startReceive();
    // if (state == RADIOLIB_ERR_NONE)
    // {
    //     Serial.println("success!");
    // }
    // else
    // {
    //     Serial.print("failed, code ");
    //     Serial.println(state);
    //     while (true)
    //         ;
    // }
}

void loop()
{
    if (digitalRead(nRF52840_BOOT) == LOW)
    {
        Send_Flag = true;
        CycleTime = millis() + 1000;
    }

    if (Send_Flag == true)
    {
        if (millis() > CycleTime)
        {
            Send_Flag = false;
            // send another one
            Serial.println("[SX1262] Sending another packet ... ");

            Send_Package[12] = (uint8_t)(Send_Data >> 24);
            Send_Package[13] = (uint8_t)(Send_Data >> 16);
            Send_Package[14] = (uint8_t)(Send_Data >> 8);
            Send_Package[15] = (uint8_t)Send_Data;

            Set_SX1262_RF_Transmitter_Switch(true);
            radio.transmit(Send_Package, 16);
            Set_SX1262_RF_Transmitter_Switch(false);
            radio.startReceive();
        }
    }

    // check if the previous operation finished
    if (operationDone)
    {
        // reset flag
        operationDone = false;

        if (radio.readData(Receive_Package, 16) == RADIOLIB_ERR_NONE)
        {
            if ((Receive_Package[0] == 'M') &&
                (Receive_Package[1] == 'A') &&
                (Receive_Package[2] == 'C') &&
                (Receive_Package[3] == ':'))
            {
                uint32_t temp_mac[2];
                temp_mac[0] =
                    ((uint32_t)Receive_Package[8] << 24) |
                    ((uint32_t)Receive_Package[9] << 16) |
                    ((uint32_t)Receive_Package[10] << 8) |
                    (uint32_t)Receive_Package[11];
                temp_mac[1] =
                    ((uint32_t)Receive_Package[4] << 24) |
                    ((uint32_t)Receive_Package[5] << 16) |
                    ((uint32_t)Receive_Package[6] << 8) |
                    (uint32_t)Receive_Package[7];

                if ((temp_mac[0] != Local_MAC[0]) && (temp_mac[1] != Local_MAC[1]))
                {
                    Receive_Data =
                        ((uint32_t)Receive_Package[12] << 24) |
                        ((uint32_t)Receive_Package[13] << 16) |
                        ((uint32_t)Receive_Package[14] << 8) |
                        (uint32_t)Receive_Package[15];

                    // packet was successfully received
                    Serial.println("[SX1262] Received packet!");

                    // print data of the packet
                    for (int i = 0; i < 16; i++)
                    {
                        Serial.printf("[SX1262] Data[%d]: %#X\n", i, Receive_Package[i]);
                    }

                    // Serial.print("Chip Mac ID[0]: ");
                    // Serial.print((uint32_t)NRF_FICR->DEVICEID[0]);
                    // Serial.println();
                    // Serial.print("Chip Mac ID[1]: ");
                    // Serial.print((uint32_t)NRF_FICR->DEVICEID[1]);
                    // Serial.println();

                    // print data of the packet
                    Serial.print("[SX1262] Data:\t\t");
                    Serial.println(Receive_Data);

                    // print RSSI (Received Signal Strength Indicator)
                    Serial.print("[SX1262] RSSI:\t\t");
                    Serial.print(radio.getRSSI());
                    Serial.println(" dBm");

                    // print SNR (Signal-to-Noise Ratio)
                    Serial.print("[SX1262] SNR:\t\t");
                    Serial.print(radio.getSNR());
                    Serial.println(" dB");

                    Send_Data = Receive_Data + 1;

                    Send_Flag = true;
                    CycleTime = millis() + 1000;
                }
            }
        }
    }
}
