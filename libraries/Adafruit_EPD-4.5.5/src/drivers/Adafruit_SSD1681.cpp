#include "Adafruit_SSD1681.h"
#include "Adafruit_EPD.h"

#define EPD_RAM_BW 0x10
#define EPD_RAM_RED 0x13

#define BUSY_WAIT 2000

// clang-format off

// const uint8_t ssd1681_default_init_code[] {
//   SSD1681_SW_RESET, 0, // soft reset
//     0xFF, 20,          // busy wait
//     SSD1681_DATA_MODE, 1, 0x03, // Ram data entry mode
//     SSD1681_WRITE_BORDER, 1, 0x05, // border color
//     SSD1681_TEMP_CONTROL, 1, 0x80, // Temp control
//     SSD1681_SET_RAMXCOUNT, 1, 0,
//     SSD1681_SET_RAMYCOUNT, 2, 0, 0,
//     0xFE};

const uint8_t ssd1681_default_init_code_full[] {
    SSD1681_SW_RESET, 0, // soft reset
    0xFF, 20,          // busy wait

    SSD1681_DRIVER_CONTROL, 3, //Driver output control 
    ((176-1)%256),((176-1)/256),0x00, 

    SSD1681_DATA_MODE, 1, 0x03, // Ram data entry mode

    SSD1681_SET_RAMXPOS, 2, //set Ram-X address start/end position   
    0x00, (192/8-1), 

    SSD1681_SET_RAMYPOS, 4, //set Ram-Y address start/end position          
    ((176-1)%256),((176-1)/256),0x00,0x00,

    SSD1681_WRITE_BORDER, 1, 0x05, //BorderWavefrom

    SSD1681_TEMP_CONTROL, 1, 0x80, //Read built-in temperature sensor

    SSD1681_SET_RAMXCOUNT, 1, 0x00, // set RAM x address count to 0;
    
    SSD1681_SET_RAMYCOUNT, 2, // set RAM y address count to 0X199;    
    ((176-1)%256),((176-1)/256),

    0xFF, 20,          // busy wait

    0xFE};

const uint8_t ssd1681_default_init_code_fast[] {
    SSD1681_SW_RESET, 0, // soft reset
    0xFF, 20,          // busy wait

    SSD1681_DATA_MODE, 1, 0x03, // Ram data entry mode 

    SSD1681_TEMP_CONTROL, 1, 0x80, //Read built-in temperature sensor

    SSD1681_DISP_CTRL2, 1, 0xB1, // Load temperature value
    SSD1681_MASTER_ACTIVATE, 0,  
    0xFF, 20,          // busy wait

    SSD1681_TEMP_WRITE, 2, 
    0x64, 0x00,// Write to temperature register

    SSD1681_DISP_CTRL2, 1, 0x91, // Load temperature value
    SSD1681_MASTER_ACTIVATE, 0,  

    0xFF, 20,          // busy wait

    0xFE};

// clang-format on

/**************************************************************************/
/*!
    @brief constructor if using external SRAM chip and software SPI
    @param width the width of the display in pixels
    @param height the height of the display in pixels
    @param SID the SID pin to use
    @param SCLK the SCLK pin to use
    @param DC the data/command pin to use
    @param RST the reset pin to use
    @param CS the chip select pin to use
    @param SRCS the SRAM chip select pin to use
    @param MISO the MISO pin to use
    @param BUSY the busy pin to use
*/
/**************************************************************************/
Adafruit_SSD1681::Adafruit_SSD1681(int width, int height, int16_t SID,
                                   int16_t SCLK, int16_t DC, int16_t RST,
                                   int16_t CS, int16_t SRCS, int16_t MISO,
                                   int16_t BUSY)
    : Adafruit_EPD(width, height, SID, SCLK, DC, RST, CS, SRCS, MISO, BUSY)
{
    if ((height % 8) != 0)
    {
        height += 8 - (height % 8);
    }

    buffer1_size = ((uint32_t)width * (uint32_t)height) / 8;
    buffer2_size = buffer1_size;

    if (SRCS >= 0)
    {
        use_sram = true;
        buffer1_addr = 0;
        buffer2_addr = buffer1_size;
        buffer1 = buffer2 = NULL;
    }
    else
    {
        buffer1 = (uint8_t *)malloc(buffer1_size);
        buffer2 = (uint8_t *)malloc(buffer2_size);
    }

    singleByteTxns = true;
}

// constructor for hardware SPI - we indicate DataCommand, ChipSelect, Reset

/**************************************************************************/
/*!
    @brief constructor if using on-chip RAM and hardware SPI
    @param width the width of the display in pixels
    @param height the height of the display in pixels
    @param DC the data/command pin to use
    @param RST the reset pin to use
    @param CS the chip select pin to use
    @param SRCS the SRAM chip select pin to use
    @param BUSY the busy pin to use
*/
/**************************************************************************/
Adafruit_SSD1681::Adafruit_SSD1681(int width, int height, int16_t DC,
                                   int16_t RST, int16_t CS, int16_t SRCS,
                                   int16_t BUSY, SPIClass *spi, int32_t speed)
    : Adafruit_EPD(width, height, DC, RST, CS, SRCS, BUSY, spi, speed)
{
    if ((height % 8) != 0)
    {
        height += 8 - (height % 8);
    }

    buffer1_size = ((uint32_t)width * (uint32_t)height) / 8;
    buffer2_size = buffer1_size;

    if (SRCS >= 0)
    {
        use_sram = true;
        buffer1_addr = 0;
        buffer2_addr = buffer1_size;
        buffer1 = buffer2 = NULL;
    }
    else
    {
        buffer1 = (uint8_t *)malloc(buffer1_size);
        buffer2 = (uint8_t *)malloc(buffer2_size);
    }

    singleByteTxns = true;
}

/**************************************************************************/
/*!
    @brief wait for busy signal to end
*/
/**************************************************************************/
void Adafruit_SSD1681::busy_wait(void)
{
    if (_busy_pin >= 0)
    {
        uint32_t temp = 0;
        while (digitalRead(_busy_pin))
        { // wait for busy low
            delay(10);
            temp++;
            if (temp > BUSY_WAIT / 10)
            {
                break;
            }
        }
    }
    else
    {
        delay(BUSY_WAIT);
    }
}

/**************************************************************************/
/*!
    @brief begin communication with and set up the display.
    @param reset if true the reset pin will be toggled.
*/
/**************************************************************************/
void Adafruit_SSD1681::begin(bool reset)
{
    Adafruit_EPD::begin(reset);
    setBlackBuffer(0, true);  // black defaults to inverted
    setColorBuffer(1, false); // red defaults to un inverted
    powerDown();
}

void Adafruit_SSD1681::update(Update_Mode mode, bool busy_enable)
{
    uint8_t buffer;

    switch (mode)
    {
    case Update_Mode::FULL_REFRESH:
        buffer = 0xF7;
        break;
    case Update_Mode::PARTIAL_REFRESH:
        buffer = 0xFF;
        break;
    case Update_Mode::FAST_REFRESH:
        buffer = 0xC7;
        break;

    default:
        break;
    }

    EPD_command(SSD1681_DISP_CTRL2, &buffer, 1);

    EPD_command(SSD1681_MASTER_ACTIVATE);

    if (busy_enable == true)
    {
        busy_wait();

        if (_busy_pin <= -1)
        {
            delay(1000);
        }
    }
}

void Adafruit_SSD1681::setRAMValueBaseMap(Update_Mode mode, bool busy_enable)
{
    powerUp(mode);

    writeRAMCommand(0);
    dcHigh();
    for (uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        SPItransfer(0xFF);
    }
    csHigh();

    writeRAMCommand(1);
    dcHigh();
    for (uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        SPItransfer(0xFF);
    }
    csHigh();

    // 执行全屏刷新
    update(mode, busy_enable);

    if (mode == Update_Mode::FULL_REFRESH)
    {
        delay(1000);
    }
}

void Adafruit_SSD1681::displayPartial(uint16_t x, uint16_t y,
                                      uint16_t w, uint16_t h, const uint8_t *datas, bool busy_enable)
{
    uint32_t x_end, y_end;

    // 根据旋转调整坐标
    uint16_t x1 = x;
    uint16_t y1 = y;
    uint16_t x2 = x + w;
    uint16_t y2 = y + h;

    // 旋转处理
    switch (getRotation())
    {
    case 0:
        EPD_swap(x1, y1);
        EPD_swap(x2, y2);
        y1 = WIDTH - y1;
        y2 = WIDTH - y2;
        break;
    case 1:
        EPD_swap(x1, y1);
        EPD_swap(x2, y2);
        y1 = WIDTH - y1;
        y2 = WIDTH - y2;
        break;
    case 2:
        EPD_swap(x1, y1);
        EPD_swap(x2, y2);
        x1 = HEIGHT - x1;
        x2 = HEIGHT - x2;
        break;
    case 3:
        EPD_swap(x1, y1);
        EPD_swap(x2, y2);
        x1 = HEIGHT - x1;
        x2 = HEIGHT - x2;
        break;

    default:
        break;
    }
    if (x1 > x2)
        EPD_swap(x1, x2);
    if (y1 > y2)
        EPD_swap(y1, y2);

    // 重新计算宽度和高度
    w = x2 - x1;
    h = y2 - y1;

    // 坐标计算
    x1 = x1 / 8;
    x_end = x1 + w / 8 - 1;
    y1 = y1 - 1;
    y_end = y1 + h - 1;

    // 硬件初始化
    hardwareReset();
    // busy_wait();

    // 设置命令序列
    uint8_t buf[4];
    buf[0] = 0x80;
    EPD_command(SSD1681_WRITE_BORDER, buf, 1);
    buf[0] = x1;
    buf[1] = x_end;
    EPD_command(SSD1681_SET_RAMXPOS, buf, 2);
    buf[0] = y1 % 256;
    buf[1] = y1 / 256;
    buf[2] = y_end % 256;
    buf[3] = y_end / 256;
    EPD_command(SSD1681_SET_RAMYPOS, buf, 4);
    buf[0] = x1;
    EPD_command(SSD1681_SET_RAMXCOUNT, buf, 1);
    buf[0] = y1 % 256;
    buf[1] = y1 / 256;
    EPD_command(SSD1681_SET_RAMYCOUNT, buf, 2);

    // 写入数据
    writeRAMCommand(0);
    dcHigh();

    // 直接传输数据（旋转由坐标处理完成）
    for (uint32_t i = 0; i < h * w / 8; i++)
    {
        SPItransfer(pgm_read_byte(&datas[i]));
    }
    csHigh();

    // 执行刷新
    update(Update_Mode::PARTIAL_REFRESH, busy_enable);
}

void Adafruit_SSD1681::powerUp(Update_Mode mode)
{
    hardwareReset();
    // busy_wait();
    switch (mode)
    {
    case Update_Mode::FULL_REFRESH:
        EPD_commandList(ssd1681_default_init_code_full);
        break;
    case Update_Mode::FAST_REFRESH:
        EPD_commandList(ssd1681_default_init_code_fast);
        break;

    default:
        EPD_commandList(ssd1681_default_init_code_full);
        break;
    }

    // uint8_t buf[5];

    // // Set display size and driver output control
    // buf[0] = (WIDTH - 1);
    // buf[1] = (WIDTH - 1) >> 8;
    // buf[2] = 0x00;
    // EPD_command(SSD1681_DRIVER_CONTROL, buf, 3);

    setRAMWindow(0, 0, (HEIGHT / 8) - 1, WIDTH - 1);
}

/**************************************************************************/
/*!
    @brief wind down the display
*/
/**************************************************************************/
void Adafruit_SSD1681::powerDown()
{
    uint8_t buf[1];
    // Only deep sleep if we can get out of it
    if (_reset_pin >= 0)
    {
        // deep sleep
        buf[0] = 0x01;
        EPD_command(SSD1681_DEEP_SLEEP, buf, 1);
        delay(100);
    }
    else
    {
        EPD_command(SSD1681_SW_RESET);
        busy_wait();
    }
}

/**************************************************************************/
/*!
    @brief Send the specific command to start writing to EPD display RAM
    @param index The index for which buffer to write (0 or 1 or tri-color
   displays) Ignored for monochrome displays.
    @returns The byte that is read from SPI at the same time as sending the
   command
*/
/**************************************************************************/
uint8_t Adafruit_SSD1681::writeRAMCommand(uint8_t index)
{
    if (index == 0)
    {
        return EPD_command(SSD1681_WRITE_RAM1, true);
    }
    if (index == 1)
    {
        return EPD_command(SSD1681_WRITE_RAM2, true);
    }
    return 0;
}

/**************************************************************************/
/*!
    @brief Some displays require setting the RAM address pointer
    @param x X address counter value
    @param y Y address counter value
*/
/**************************************************************************/
void Adafruit_SSD1681::setRAMAddress(uint16_t x, uint16_t y)
{
    uint8_t buf[2];

    // set RAM x address count
    buf[0] = x;
    EPD_command(SSD1681_SET_RAMXCOUNT, buf, 1);

    // set RAM y address count
    buf[0] = y;
    buf[1] = y >> 8;
    EPD_command(SSD1681_SET_RAMYCOUNT, buf, 2);
}

/**************************************************************************/
/*!
    @brief Some displays require setting the RAM address pointer
    @param x X address counter value
    @param y Y address counter value
*/
/**************************************************************************/
void Adafruit_SSD1681::setRAMWindow(uint16_t x1, uint16_t y1, uint16_t x2,
                                    uint16_t y2)
{
    uint8_t buf[5];

    // Set ram X start/end postion
    buf[0] = x1;
    buf[1] = x2;
    EPD_command(SSD1681_SET_RAMXPOS, buf, 2);

    // Set ram Y start/end postion
    buf[0] = y1;
    buf[1] = y1 >> 8;
    buf[2] = y2;
    buf[3] = y2 >> 8;
    EPD_command(SSD1681_SET_RAMYPOS, buf, 4);
}
