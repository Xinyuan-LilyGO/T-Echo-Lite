// include the library
#include "RadioLib.h"
#include "pin_config.h"

// uncomment the following only on one
// of the nodes to initiate the pings
#define INITIATING_NODE

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

// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;

// this function is called when a complete packet
// is transmitted or received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void)
{
    // we sent or received a packet, set the flag
    operationDone = true;
}

void Set_SX1262_RF_Transmitter_Switch(bool status)
{
    if (status == true)
    {
        digitalWrite(SX1262_RF_VC1, HIGH); // 发送
        digitalWrite(SX1262_RF_VC2, LOW);
    }
    else
    {
        digitalWrite(SX1262_RF_VC1, LOW); // 接收
        digitalWrite(SX1262_RF_VC2, HIGH);
    }
}

void setup()
{
    Serial.begin(115200);
    // while (!Serial)
    // {
    //     delay(100); // wait for native usb
    // }
    Serial.println("Ciallo");

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    pinMode(SX1262_RF_VC1, OUTPUT);
    pinMode(SX1262_RF_VC2, OUTPUT);
    Set_SX1262_RF_Transmitter_Switch(false);

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

    radio.setFrequency(868.0);
    radio.setBandwidth(125.0);
    radio.setSpreadingFactor(9);
    radio.setCodingRate(6);
    radio.setSyncWord(0xAB);
    radio.setOutputPower(22);
    radio.setCurrentLimit(140);
    radio.setPreambleLength(16);
    radio.setCRC(false);

    // set the function that will be called
    // when new packet is received
    radio.setDio1Action(setFlag);

    Set_SX1262_RF_Transmitter_Switch(true);
    radio.transmitDirect();
}

void loop()
{
}
