#include <SPI.h>
// EPD
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"
#include "Ap_29demo.h"
#include "pin_config.h"

void setup()
{
    // 3.3V Power ON
    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

#ifdef ESP8266
    pinMode(D0, INPUT);  // BUSY
    pinMode(D1, OUTPUT); // RES
    pinMode(D2, OUTPUT); // DC
    pinMode(D4, OUTPUT); // CS
#endif

#ifdef Arduino_UNO
    pinMode(SCREEN_BUSY, INPUT_PULLUP); // BUSY
    pinMode(SCREEN_RST, OUTPUT); // RES
    pinMode(SCREEN_DC, OUTPUT);  // DC
    pinMode(SCREEN_CS, OUTPUT);  // CS
#endif
    // SPI
    SPI.setPins(SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    SPI.begin();
}

// Tips//
/*
1.Flickering is normal when EPD is performing a full screen update to clear ghosting from the previous image so to ensure better clarity and legibility for the new image.
2.There will be no flicker when EPD performs a partial refresh.
3.Please make sue that EPD enters sleep mode when refresh is completed and always leave the sleep mode command. Otherwise, this may result in a reduced lifespan of EPD.
4.Please refrain from inserting EPD to the FPC socket or unplugging it when the MCU is being powered to prevent potential damage.)
5.Re-initialization is required for every full screen update.
6.When porting the program, set the BUSY pin to input mode and other pins to output mode.
*/
void loop()
{
    unsigned char i;
#if 1 // Full screen refresh, fast refresh, and partial refresh demostration.

    EPD_HW_Init();           // Full screen refresh initialization.
    EPD_WhiteScreen_White(); // Clear screen function.
    EPD_DeepSleep();         // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
    delay(2000);             // Delay for 2s.
    /************Full display(2s)*******************/
    EPD_HW_Init();                 // Full screen refresh initialization.
    EPD_WhiteScreen_ALL(gImage_1); // To Display one image using full screen refresh.
    EPD_DeepSleep();               // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
    delay(2000);                   // Delay for 2s.
    /************Fast refresh mode(2s)*******************/
    EPD_HW_Init_Fast();                 // Fast refresh initialization.
    EPD_WhiteScreen_ALL_Fast(gImage_2); // To display the second image using fast refresh.
    EPD_DeepSleep();                    // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
    delay(2000);                        // Delay for 2s.
#if 1                                   // Partial refresh demostration.
    // Partial refresh demo support displaying a clock at 5 locations with 00:00.  If you need to perform partial refresh more than 5 locations, please use the feature of using partial refresh at the full screen demo.
    // After 5 partial refreshes, implement a full screen refresh to clear the ghosting caused by partial refreshes.
    //////////////////////Partial refresh time demo/////////////////////////////////////
    EPD_HW_Init();                           // Electronic paper initialization.
    EPD_SetRAMValue_BaseMap(gImage_basemap); // Please do not delete the background color function, otherwise it will cause unstable display during partial refresh.
                                             // EPD_WhiteScreen_White(); // Clear screen function.
    for (i = 0; i < 10; i++)
    {
        EPD_Dis_Part_Time(64, 40 + 32 * 0, Num[i],          // x-A,y-A,DATA-A
                          64, 40 + 32 * 1, Num[0],          // x-B,y-B,DATA-B
                          64, 40 + 32 * 2, gImage_numdot,   // x-C,y-C,DATA-C
                          64, 40 + 32 * 3, Num[0],          // x-D,y-D,DATA-D
                          64, 40 + 32 * 4, Num[1], 32, 64); // x-E,y-E,DATA-E,Resolution 32*64
        delay(1000);
    };
    EPD_DeepSleep();         // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
    delay(5000);             // Delay for 2s.
    EPD_HW_Init();           // Full screen refresh initialization.
    EPD_WhiteScreen_White(); // Clear screen function.
    EPD_DeepSleep();         // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
    delay(2000);             // Delay for 2s.
#endif

#if 0 // Demo of using partial refresh to update the full screen, to enable this feature, please change 0 to 1.
  //After 5 partial refreshes, implement a full screen refresh to clear the ghosting caused by partial refreshes.
  //////////////////////Partial refresh time demo/////////////////////////////////////
      EPD_HW_Init(); //E-paper initialization 
      EPD_SetRAMValue_BaseMap(gImage_p1); //Please do not delete the background color function, otherwise it will cause an unstable display during partial refresh.
      EPD_Dis_PartAll(gImage_p1); //Image 1
      EPD_Dis_PartAll(gImage_p2); //Image 2
      EPD_Dis_PartAll(gImage_p3); //Image 3
      EPD_Dis_PartAll(gImage_p4); //Image 4
      EPD_Dis_PartAll(gImage_p5); //Image 5 
      EPD_DeepSleep();//Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
      delay(2000); //Delay for 2s. 
      EPD_HW_Init(); //Full screen refresh initialization.
      EPD_WhiteScreen_White(); //Clear screen function.
      EPD_DeepSleep(); //Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
      delay(2000); //Delay for 2s.
#endif

#if 0 // Demonstration of full screen refresh with 180-degree rotation, to enable this feature, please change 0 to 1.
      /************Full display(2s)*******************/
      EPD_HW_Init_180(); //Full screen refresh initialization.
      EPD_WhiteScreen_ALL(gImage_1); //To Display one image using full screen refresh.
      EPD_DeepSleep(); //Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
      delay(2000); //Delay for 2s.
#endif
#endif

#ifdef ESP8266
    while (1)
    {
        Sys_run(); // System run
        LED_run(); // Breathing lamp
    }
#endif
#ifdef Arduino_UNO
    // while (1)
    //     ; // The program stops here
#endif
}

//////////////////////////////////END//////////////////////////////////////////////////
