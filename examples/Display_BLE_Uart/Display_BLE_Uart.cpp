/*
 * @Description: Ink screen and Bluetooth BLE test
 * @Author: LILYGO_L
 * @Date: 2024-08-16 15:31:24
 * @LastEditTime: 2024-11-27 09:17:19
 * @License: GPL 3.0
 */

#include "Adafruit_EPD.h"
#include "pin_config.h"
#include "Display_Fonts.h"
#include <bluefruit.h>

struct BLE_Uart_Operator
{
    using state = enum {
        UNCONNECTED, // 未连接
        CONNECTED,   // 已连接
    };

    struct
    {
        bool state_flag = state::UNCONNECTED;
        bool trigger_flag = false;
        char device_name[32] = {'\0'};
    } connection;

    struct
    {
        uint8_t receive_data[100] = {'\0'};
    } transmission;

    bool initialization_flag = false;
    bool transmission_fast_refresh_flag = false;
};

static size_t CycleTime = 0;

/* UART Serivce: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
 * UART RXD    : 6E400002-B5A3-F393-E0A9-E50E24DCCA9E
 * UART TXD    : 6E400003-B5A3-F393-E0A9-E50E24DCCA9E
 */

// BLE Service
BLEDfu bledfu;   // OTA DFU service
BLEDis bledis;   // device information
BLEUart bleuart; // uart over ble

SPIClass Custom_SPI(NRF_SPIM0, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
Adafruit_SSD1681 display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC, SCREEN_RST,
                         SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &Custom_SPI, 8000000);

BLE_Uart_Operator BLE_Uart_OP;

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
    BLE_Uart_OP.connection.trigger_flag = true;
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
    BLE_Uart_OP.connection.trigger_flag = true;
    BLE_Uart_OP.transmission_fast_refresh_flag = true;
}

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

bool BLE_Uart_Initialization(void)
{
    // Setup the BLE LED to be enabled on CONNECT
    // Note: This is actually the default behavior, but provided
    // here in case you want to control this LED manually via PIN 19
    Bluefruit.autoConnLed(true);

    // Config the peripheral connection with maximum bandwidth
    // more SRAM required by SoftDevice
    // Note: All config***() function must be called before begin()
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

    if (Bluefruit.begin() == false)
    {
        Serial.println("BLE initialization failed");
        return false;
    }
    Serial.println("BLE initialization successful");

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

    return true;
}

void GFX_Print_TEST(String s)
{
    display.fillScreen(EPD_WHITE);
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(2);

    display.setCursor(SCREEN_WIDTH / 4 + 5, SCREEN_HEIGHT / 4 - 15);
    display.printf("TEST");

    display.setFont(&FreeMono9pt7b);
    display.setCursor(20, SCREEN_HEIGHT / 4 + 10);
    display.setTextSize(1);
    display.print(s);

    display.setFont(&Org_01);
    display.setCursor(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 + 40);
    display.setTextSize(4);
    display.printf("3");
    display.display(display.update_mode::FULL_REFRESH, true);
    delay(200);
    display.fillRect(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 + 20, 30, 40, EPD_WHITE);
    display.setCursor(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 + 40);
    display.printf("2");
    display.display(display.update_mode::PARTIAL_REFRESH, true);
    display.display(display.update_mode::PARTIAL_REFRESH, true);
    delay(200);
    display.fillRect(SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 + 20, 30, 30, EPD_WHITE);
    display.setCursor(SCREEN_WIDTH / 2 + 3, SCREEN_HEIGHT / 2 + 40);
    display.printf("1");
    display.display(display.update_mode::PARTIAL_REFRESH, true);
    display.display(display.update_mode::PARTIAL_REFRESH, true);
    delay(200);
}

void GFX_Print_BLE_Uart_Info(void)
{
    display.fillScreen(EPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(1);
    display.setCursor(25, 15);
    display.printf("BLE Uart Info");

    display.setFont(&Org_01);
    display.setCursor(10, 105);
    display.printf("<-----------Receive Info-----------> ");
}

void GFX_Print_BLE_Uart_Init_Successful_Refresh_Info(void)
{
    display.setFont(&Org_01);
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 40, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 28);
    display.printf("[Status]: Init successful");

    display.setCursor(10, 35);
    display.printf("[BLE Name]: T-Echo-Lite-nRF52840");
    display.setCursor(10, 49);
    display.printf("[Uart Serivce]: 6E400001......");
    display.setCursor(10, 56);
    display.printf("[Uart RX]: 6E400002......");
    display.setCursor(10, 63);
    display.printf("[Uart TX]: 6E400003......");

    display.setFont(&Org_01);
    display.setTextSize(1);
    display.setCursor(10, 77);
    display.printf("[Local MAC]: %s", getMcuUniqueID());
}

void GFX_Print_BLE_Uart_Init_Failed_Refresh_Info(void)
{
    display.setFont(&Org_01);
    display.fillRect(0, 28 - 5, SCREEN_WIDTH, 40, EPD_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 28);
    display.printf("[Status]: Init failed");
}

void GFX_Print_BLE_Uart_Connect_Info(void)
{
    display.fillRect(0, 91 - 5, SCREEN_HEIGHT, 14, EPD_WHITE);
    display.setCursor(10, 91);

    if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::CONNECTED)
    {
        display.printf("[Connect]: Connected");
    }
    else if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::UNCONNECTED)
    {
        display.printf("[Connect]: Unconnected");
    }
}

void GFX_Print_BLE_Uart_Transmission_Refresh_Info(void)
{
    display.setFont(&Org_01);
    display.fillRect(0, 112 - 5, SCREEN_HEIGHT, 42, EPD_WHITE);

    display.setCursor(10, 112);
    if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::CONNECTED)
    {
        display.printf("[Receive Data]: ");

        display.setCursor(20, 119);
        display.print("[");
        display.write(BLE_Uart_OP.connection.device_name,
                      strlen(BLE_Uart_OP.connection.device_name));
        display.print("]-> ");
        display.write(BLE_Uart_OP.transmission.receive_data,
                      strlen((const char *)BLE_Uart_OP.transmission.receive_data));
    }
    else if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::UNCONNECTED)
    {
        display.printf("[Receive Data]: null");
    }
}

void setup()
{
    Serial.begin(115200);
    // while (!Serial)
    // {
    //     delay(100);
    // }
    Serial.println("Ciallo");

    display.begin();
    display.setRotation(1);
    display.setTextColor(EPD_BLACK);

    GFX_Print_TEST("BLE Uart test");

    GFX_Print_BLE_Uart_Info();
    if (BLE_Uart_Initialization() == true)
    {
        GFX_Print_BLE_Uart_Init_Successful_Refresh_Info();
        BLE_Uart_OP.initialization_flag = true;
    }
    else
    {
        GFX_Print_BLE_Uart_Init_Failed_Refresh_Info();
        BLE_Uart_OP.initialization_flag = false;
    }
    GFX_Print_BLE_Uart_Connect_Info();
    GFX_Print_BLE_Uart_Transmission_Refresh_Info();
    display.display(display.update_mode::FULL_REFRESH, true);
}

void loop()
{
    if (BLE_Uart_OP.connection.trigger_flag == true)
    {
        GFX_Print_BLE_Uart_Connect_Info();
        display.display(display.update_mode::FAST_REFRESH, true);

        BLE_Uart_OP.connection.trigger_flag = false;
    }

    if (BLE_Uart_OP.transmission_fast_refresh_flag == true)
    {
        GFX_Print_BLE_Uart_Transmission_Refresh_Info();
        display.display(display.update_mode::FAST_REFRESH, true);
        BLE_Uart_OP.transmission_fast_refresh_flag = false;
    }

    if (BLE_Uart_OP.connection.state_flag == BLE_Uart_OP.state::CONNECTED)
    {
        if (millis() > CycleTime)
        {
            uint8_t send_buffer[100];
            size_t size = sprintf((char *)send_buffer,
                                  "BLE Test \n[T-Echo-Lite MAC ID]: %s\n[MCU Running time]: %u s\n",
                                  getMcuUniqueID(), (uint32_t)(millis() / 1000));
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
            memset(BLE_Uart_OP.transmission.receive_data, '\0', 100);
            // size_t temp_size = bleuart.available();
            // uint8_t temp_buf[temp_size];
            bleuart.read(BLE_Uart_OP.transmission.receive_data, 100);

            Serial.print("[");
            Serial.write(BLE_Uart_OP.connection.device_name,
                         strlen(BLE_Uart_OP.connection.device_name));
            Serial.print("]-> ");
            Serial.write(BLE_Uart_OP.transmission.receive_data,
                         strlen((const char *)BLE_Uart_OP.transmission.receive_data));
            BLE_Uart_OP.transmission_fast_refresh_flag = true;
        }
    }
}
