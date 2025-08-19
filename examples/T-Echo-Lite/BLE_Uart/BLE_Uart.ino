/*
 * @Description: A BLE serial communication example
            Upload the program, open the BLE Bluetooth debugging assistant, find the corresponding endpoint,
        and Bluetooth serial communication can be achieved.
 * @Author: LILYGO_L
 * @Date: 2024-08-14 14:20:44
 * @LastEditTime: 2025-06-05 14:51:39
 * @License: GPL 3.0
 */

#include <bluefruit.h>
#include "t_echo_lite_config.h"
#include <string.h>

struct BLE_Uart_Operator
{
    using state = enum {
        UNCONNECTED, // not connected
        CONNECTED,   // connected already
    };

    struct
    {
        bool state_flag = state::UNCONNECTED;
        char device_name[32] = {'\0'};
    } connection;
};

size_t CycleTime = 0;

/* UART Serivce: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
 * UART RXD    : 6E400002-B5A3-F393-E0A9-E50E24DCCA9E
 * UART TXD    : 6E400003-B5A3-F393-E0A9-E50E24DCCA9E
 */

// BLE Service
BLEDfu bledfu;   // OTA DFU service
BLEDis bledis;   // device information
BLEUart bleuart; // uart over ble

BLE_Uart_Operator BLE_Uart_OP;

void startAdv(void)
{
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();

    // Include bleuart 128-bit uuid
    Bluefruit.Advertising.addService(bleuart);

    // Secondary Scan Response packet (optional)
    // Since there is no room for 'Name' in Advertising packet
    Bluefruit.ScanResponse.addName();

    /* Start Advertising
     * - Enable auto advertising if disconnected
     * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     * - Timeout for fast mode is 30 seconds
     * - Start(timeout) with timeout = 0 will advertise forever (until connected)
     *
     * For recommended advertising interval
     * https://developer.apple.com/library/content/qa/qa1931/_index.html
     */
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
    Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
    // Get the reference to current connection
    BLEConnection *connection = Bluefruit.Connection(conn_handle);

    char central_name[32] = {0};
    connection->getPeerName(central_name, sizeof(central_name));

    Serial.print("Connected to ");
    Serial.println(central_name);

    strcpy(BLE_Uart_OP.connection.device_name, central_name);

    BLE_Uart_OP.connection.state_flag = BLE_Uart_OP.state::CONNECTED;
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
    (void)conn_handle;
    (void)reason;

    Serial.println();
    Serial.print("Disconnected, reason = 0x");
    Serial.println(reason, HEX);
    BLE_Uart_OP.connection.state_flag = BLE_Uart_OP.state::UNCONNECTED;
}

void setup()
{
    Serial.begin(115200);

#if CFG_DEBUG
    // Blocking wait for connection when debug mode is enabled via IDE
    while (!Serial)
        yield();
#endif

    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    Serial.println("BLE UART Example");
    Serial.println("---------------------------\n");

    // Setup the BLE LED to be enabled on CONNECT
    // Note: This is actually the default behavior, but provided
    // here in case you want to control this LED manually via PIN 19
    Bluefruit.autoConnLed(true);

    // Config the peripheral connection with maximum bandwidth
    // more SRAM required by SoftDevice
    // Note: All config***() function must be called before begin()
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

    Bluefruit.begin();
    Bluefruit.setTxPower(8); // Check bluefruit.h for supported values
    // Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

    // To be consistent OTA DFU should be added first if it exists
    bledfu.begin();

    // Configure and Start Device Information Service
    bledis.setManufacturer("LILYGO Industries");
    bledis.setModel("T-Echo-Lite");
    bledis.begin();

    // Configure and Start BLE Uart Service
    bleuart.begin();

    // Set up and start advertising
    startAdv();

    Serial.println("Please use the BLE debugging tool to connect to the development board.");
    Serial.println("Once connected, enter character(s) that you wish to send");
}

void loop()
{
    if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::CONNECTED)
    {
        if (millis() > CycleTime)
        {
            uint8_t send_buffer[100];
            size_t size = sprintf((char *)send_buffer,
                                  "BLE Test \n[T-Echo-Lite MAC ID]: %s\n[MCU Running time]: %u s\n",
                                  getMcuUniqueID(), millis() / 1000);
            bleuart.write(send_buffer, size);
            CycleTime = millis() + 1000;
        }

        // Forward data from HW Serial to BLEUART
        while (Serial.available())
        {
            // Delay to wait for enough input, since we have a limited transmission buffer
            delay(2);

            size_t temp_size = Serial.available();
            uint8_t temp_buf[temp_size];
            Serial.readBytes(temp_buf, temp_size);
            bleuart.write(temp_buf, temp_size);
        }

        // Forward from BLEUART to HW Serial
        while (bleuart.available())
        {
            size_t temp_size = bleuart.available();
            uint8_t temp_buf[temp_size];
            bleuart.read(temp_buf, temp_size);

            Serial.print("[");
            Serial.write(BLE_Uart_OP.connection.device_name,
                         strlen(BLE_Uart_OP.connection.device_name));
            Serial.print("]-> ");
            Serial.write(temp_buf, temp_size);
        }
    }
}
