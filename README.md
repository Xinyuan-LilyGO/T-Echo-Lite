<!--
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2023-09-11 16:13:14
 * @LastEditTime: 2024-12-10 09:34:23
 * @License: GPL 3.0
-->
<h1 align = "center">T-Echo-Lite</h1>

## **English | [中文](./README_CN.md)**

## Version iteration:
| Version                              | Update date                       |
| :-------------------------------: | :-------------------------------: |
| T-Echo-Lite_V1.0            | 2024-12-06                         |

## PurchaseLink
| Product                     | SOC           |  FLASH  |  PSRAM   | Link                   |
| :------------------------: | :-----------: |:-------: | :---------: | :------------------: |
| T-Echo-Lite_V1.0   | nRF52840 |   1M   |256kB| NULL |

## Directory
- [Describe](#describe)
- [Preview](#preview)
- [Module](#module)
- [SoftwareDeployment](#SoftwareDeployment)
- [PinOverview](#pinoverview)
- [RelatedTests](#RelatedTests)
- [FAQ](#faq)
- [Project](#project)

## Describe

T-Echo-Lite is a lightweight version based on T-Echo, featuring a smaller volume and lower power consumption design compared to T-Echo. Its minimum deep sleep power consumption can reach 2μA to 10μA (due to differences in onboard components on different boards, power consumption performance may vary; the minimum power consumption mentioned here is based on the engineering board tested by the LILYGO laboratory). The board is equipped with a rich set of features, including an inertial sensor, LORA module, solar charging function (5V), external GPS, and more. Its excellent power consumption performance allows T-Echo-Lite to achieve superior battery life.

## Preview

### Actual Product Image

<!-- <p align="center" width="100%">
    <img src="image/14.jpg" alt="">
</p>

---

<p align="center" width="100%">
    <img src="image/13.jpg" alt="">
</p>

---

<p align="center" width="100%">
    <img src="image/15.jpg" alt="">
</p> -->

## Module
### 1. MCU
*   Chip: nRF52840
*   RAM: 256kB
*   FLASH: 1M
*   Related Documentation:
    > [nRF52840_Datasheet](https://docs.nordicsemi.com/bundle/ps_nrf52840/page/keyfeatures_html5.html)

### 2. Screen
*   Name: GDEM0122T61
*   Size: 1.22 inches
*   Resolution: 176x192px
*   Screen Type: E-PAPER
*   Driver Chip: SSD1681
*   Bus Communication Protocol: IIC
*   Additional Notes: Fast refresh is not supported (after consulting the screen manufacturer, they replied that it is not supported), it is recommended to use full refresh only
*   Dependent Libraries:
    > [Adafruit_EPD-4.5.5](./libraries/Adafruit_EPD-4.5.5/)  
    > [Adafruit_BusIO-1.16.1](./libraries/Adafruit_BusIO-1.16.1/)  
    > [Adafruit_SPIFlash-4.3.4](./libraries/Adafruit_SPIFlash-4.3.4/)  
    > [Adafruit-GFX-Library-1.11.10](./libraries/Adafruit-GFX-Library-1.11.10/)  
*   Related Documentation:
    > [GDEM0122T61](./information/GDEM0122T61.pdf)  
    > [SSD1681](./information/SSD1681.pdf)  

### 3. LORA
*   Chip Module: S62F
*   Chip: SX1262
*   Bus Communication Protocol: SPI
*   Dependent Libraries:
    > [RadioLib-6.6.0](./libraries/RadioLib-6.6.0/)  
    > [Adafruit_BusIO-1.16.1](./libraries/Adafruit_BusIO-1.16.1/)  
    > [Adafruit_SPIFlash-4.3.4](./libraries/Adafruit_SPIFlash-4.3.4/)  
*   Related Documentation:
    > [S62F](./information/S62F.pdf)  

### 4. GPS
*   Chip Module: L76K
*   Bus Communication Protocol: UART
*   Dependent Libraries:
    > [TinyGPSPlus-1.0.3a](./libraries/TinyGPSPlus-1.0.3a/)  
*   Related Documentation:
    > [L76KB-A58](./information/L76KB-A58.pdf)  

### 5. Inertial Measurement Unit
*   Chip: ICM20948
*   Bus Communication Protocol: IIC
*   Dependent Libraries:
    > [ICM20948_WE-1.1.11](./libraries/ICM20948_WE-1.1.11/)  
*   Related Documentation:
    > [ICM20948](./information/ICM20948.pdf)  

### 6. Flash
*   Chip: ZD25WQ32CEIGR
*   Bus Communication Protocol: SPI
*   Dependent Libraries:
    > [Adafruit_BusIO-1.16.1](./libraries/Adafruit_BusIO-1.16.1/)  
    > [Adafruit_SPIFlash-4.3.4](./libraries/Adafruit_SPIFlash-4.3.4/)  
*   Related Documentation:
    > [ZD25WQ32CEIGR](./information/ZD25WQ32CEIGR.pdf)

## SoftwareDeployment

### Examples Support

| Example | `[Arduino IDE (Adafruit_nRF52_V1.6.1)]` <br /> `[PlatformIO (nordicnrf52_V10.6.0)]` <br /> Support | Description | Picture |
| ------  | ------  | ------ | ------ | 
| [Battery_Measurement](./examples/Battery_Measurement) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [Original_Test](./examples/Original_Test) |<p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> | Product factory original testing |  |
| [BLE_Uart](./examples/BLE_Uart) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [Button_Triggered](./examples/Button_Triggered) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [Display](./examples/BLE_Uart) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [Display_BLE_Uart](./examples/Button_Triggered) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [Display_SX1262](./examples/BLE_Uart) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [Flash](./examples/Button_Triggered) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [Flash_Erase](./examples/BLE_Uart) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [GPS](./examples/Button_Triggered) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [GPS_Full](./examples/BLE_Uart) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [ICM20948](./examples/Button_Triggered) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [IIC_Scan_2](./examples/BLE_Uart) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [Sleep_Wake_Up](./examples/Button_Triggered) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [SX126x_PingPong](./examples/BLE_Uart) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |
| [SX126x_PingPong_2](./examples/Button_Triggered) | <p align="center"> <img src="https://img.shields.io/badge/-supported-green" alt="example" width="25%"> </p> |  |  |

| Bootloader | Description | Picture |
| ------  | ------  | ------ |
| [Bootloader_V1.0.0](./bootloader/t-echo_lite_nrf52840_bootloader-0.9.2-dirty_s140_6.1.1_v1.0.0.hex) <br /> [Bootloader_V1.0.0(uf2)](./bootloader/update-t-echo_lite_nrf52840_bootloader-0.9.2-dirty_nosd_v1.0.0.uf2) | Original |  |

| Firmware | Description | Picture |
| ------  | ------  | ------ |
| [Original_Test](./firmware/[T-Echo-Lite_V1.0][Original_Test]_firmware/[T-Echo-Lite_V1.0][Original_Test]_firmware_202412040900.hex.bin) <br /> [Original_Test(uf2)](./firmware/[T-Echo-Lite_V1.0][Original_Test]_firmware/[T-Echo-Lite_V1.0][Original_Test]_firmware_202412040900.uf2)| Product factory original testing |  |

### IDE and Flashing

#### PlatformIO
1. Install [VisualStudioCode](https://code.visualstudio.com/Download),choose installation based on your system type.

2. Open the "Extension" section of the Visual Studio Code software sidebar (Alternatively, use "<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>X</kbd>" to open the extension). Search for the "PlatformIO IDE" extension and download it.

3. During the installation of the extension, you can go to GitHub to download the program. You can download the main branch by clicking on the "<> Code" with green text, or you can download the program versions from the "Releases" section in the sidebar.

4. After the installation of the extension is completed, open the Explorer in the sidebar (Alternatively, use "<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>E</kbd>" go open it). Click on "Open Folder", locate the project code you just downloaded (the entire folder), and click "Add." At this point, the project files will be added to your workspace.

5. Open the "platformio.ini" file in the project folder (PlatformIO will automatically open the "platformio.ini" file corresponding to the added folder). Under the "[platformio]" section, uncomment and select the example program you want to burn (it should start with "default_envs = xxx") Then click "<kbd>[√](image/4.png)</kbd>" in the bottom left corner to compile. If the compilation is correct, connect the microcontroller to the computer and click "<kbd>[→](image/5.png)</kbd>" in the bottom left corner to download the program.

6. At this point, an error may occur, and you need to install [Python](https://www.python.org/downloads/). Open the folder "tool" -> "win10 vscode platformio start" sequentially, and execute the cmd command `python t-echo-lite_v1.0.0_setup.py` under the "win10 vscode platformio start" folder. This will complete the development board installation, and the compilation and flashing will no longer report errors.

#### Arduino

1. Install [Arduino](https://www.arduino.cc/en/software), and select the installation based on your system type.

2. Open the "example" directory of the project folder, select the example project folder, and open the file ending with ".ino" to open the Arduino IDE project workspace.

3. Open the "Tools" menu bar at the top right -> Select "Board" -> "Board Manager", find or search for "Adafruit_nRF52", and download the board file with the author named "Adafruit". Then return to the "Board" menu bar, select the board type under the "Adafruit_nRF52" board, and the selected board type is determined by the "board = xxx" header under the [env] directory in the "platformio.ini" file. If there is no corresponding board, you need to manually add the board under the "board" directory in the project folder. (If "Adafruit_nRF52" cannot be found, you need to open Preferences -> Add `https://www.adafruit.com/package_adafruit_index.json` to "Additional Board Manager URLs")
    
4. Open the menu bar "[File](image/6.png)" -> "[Preferences](image/6.png)", find the "[Project Folder Location](image/7.png)" section, and copy and paste all the library files along with the folders in the "libraries" folder under the project directory into the "libraries" folder in this directory.
    
5. Select the correct settings in the "Tools" menu, as shown in the table below.
    
| Setting                               | Value                                 |
| :-------------------------------: | :-------------------------------: |
| Board                                 | Nordic nRF52840 DK           |

6.  Select the correct port.

7.  Enable boot download mode: Press and release the RST chip reset button, wait for LED1 to light up (you must wait for LED1 to light up), then press and release the RST button again. Observe that LED1 gradually dims and then gradually lights up, indicating that the boot download mode has been entered.

8.  Click the top right "[√](image/8.png)" to compile. If there are no errors, connect the microcontroller to the computer and click the top right "[→](image/9.png)" to start the flashing process.

#### JLINK Flashing Firmware and Bootloader

1.  Install the software [nRF-Connect-for-Desktop](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-Desktop/Download#infotabs)

2.  Install the software [JLINK](https://www.segger.com/downloads/jlink/)

3.  Connect the JLINK pins correctly as shown in the figure below

<p align="center" width="100%">
    <img src="image/12.jpg" alt="">
</p>

4.  Open the software nRF-Connect-for-Desktop and install the tool [Programmer](./image/10.png) and open it

5.  Add files, select both the bootloader file and the firmware file at the same time, click [Erase&write](./image/11.png), and the flashing will be completed.

## PinOverview

| Flash pin          | nRF52840 pin      |
| :------------------: | :------------------:|
| CS      | IO 0.12                  |
| SCLK      | IO 0.4                  |
| (SPI)MOSI      | IO 0.6                  |
| (SPI)MISO      | IO 0.8                  |
| (QSPI)IO0      | IO 0.6                  |
| (QSPI)IO1      | IO 0.8                  |
| (QSPI)IO2      | IO 1.9                  |
| (QSPI)IO3      | IO 0.26                  |

| LED pin          | nRF52840 pin      |
| :------------------: | :------------------:|
| LED_1      | IO 1.7                  |
| LED_1      | IO 1.5                  |
| LED_1      | IO 1.14                  |

| Screen pin       | nRF52840 pin      |
| :------------------: | :------------------:|
| BS1                     | IO 1.12                  |
| BUSY                     | IO 0.3                  |
| RST                     | IO 0.28                  |
| DC                     | IO 0.21                  |
| CS                    | IO 0.22                  |
| SCLK                  | IO 0.19                  |
| MOSI                  | IO 0.20                  |

| LORA pin          | nRF52840 pin      |
| :------------------: | :------------------:|
| CS      | IO 0.11                  |
| RST      | IO 0.7                  |
| SCLK      | IO 0.13                  |
| MOSI      | IO 0.15                  |
| MISO      | IO 0.17                  |
| BUSY      | IO 0.14                  |
| INT      | IO 1.8                  |
| DIO1      | IO 1.8                  |
| DIO2      | IO 0.5                  |
| RF_VC1      | IO 0.27                  |
| RF_VC2      | IO 1.1                  |

| BOOT key pin          | nRF52840 pin      |
| :------------------: | :------------------:|
| BOOT      | IO 0.24                  |

| Two SH1.0 external sockets pin          | nRF52840 pin      |
| :------------------: | :------------------:|
| SH1_0_1_1      | IO 0.25                  |
| SH1_0_1_2      | IO 0.23                  |
| SH1_0_2_1      | IO 1.2                  |
| SH1_0_2_2      | IO 1.4                  |

| Battery pin          | nRF52840 pin      |
| :------------------: | :------------------:|
| BATTERY_MEASUREMENT_CONTROL      | IO 0.31                  |
| BATTERY_ADC_DATA      | IO 0.2                  |

| RT9080 power supply 3.3V pin          | nRF52840 pin      |
| :------------------: | :------------------:|
| RT9080_EN      | IO 0.30                  |

| GPS pin          | nRF52840 pin      |
| :------------------: | :------------------:|
| UART_RX      | IO 1.13                  |
| UART_TX      | IO 1.15                  |
| 1PPS      | IO 0.29                  |
| WAKE_UP      | IO 1.10                  |
| POWER_RT9080_EN      | IO 1.11                  |

| IMU pin          | nRF52840 pin      |
| :------------------: | :------------------:|
| SDA      | IO 1.4                  |
| SCL      | IO 1.2                  |
| INT      | IO 0.16                  |

### Power Dissipation
| Firmware | Program| Description | Picture |
| ------  | ------  | ------ | ------ | 
| [Sleep_Wake_Up](./firmware/[T-Echo-Lite_V1.0][Sleep_Wake_Up]_firmware/[T-Echo-Lite_V1.0][Sleep_Wake_Up]_firmware_202412040900.hex) <br /> [Sleep_Wake_Up(uf2)](./firmware/[T-Echo-Lite_V1.0][Sleep_Wake_Up]_firmware/[T-Echo-Lite_V1.0][Sleep_Wake_Up]_firmware_202412040900.uf2) | `Sleep_Wake_Up` | Minimum power consumption: 2.54uA <br /> More information can be found in the [Power Consumption Test Log](./relevant_test/PowerConsumptionTestLog_[T-Echo-Lite_V1.0]_20241210.pdf) | <p align="center"> <img src="image/13.png" alt="example" width="100%"> </p> |

## FAQ

* Q. After reading the above tutorials, I still don't know how to build a programming environment. What should I do?
* A. If you still don't understand how to build an environment after reading the above tutorials, you can refer to the [LilyGo-Document](https://github.com/Xinyuan-LilyGO/LilyGo-Document) document instructions to build it.

<br />

* Q. Why does Arduino IDE prompt me to update library files when I open it? Should I update them or not?
* A. Choose not to update library files. Different versions of library files may not be mutually compatible, so it is not recommended to update library files.

<br />

* Q. Why is there no debug information output from my board's USB?
* A. Please enable the "DTR" option in your serial assistant software.

<br />

* Q. Why does my board fail to flash when I use USB directly?
* A. Please press and release the RST chip reset button, wait for LED1 to light up (you must wait for LED1 to light up), then press and release the RST button again. Observe that LED1 gradually dims and then gradually lights up, indicating that the boot download mode has been entered. At this point, you can flash the board.

<br />

## Project
* [T-Echo-Lite_V1.0](./project/T-Echo-Lite_V1.0/T-Echo-Lite_V1.0.pdf)
* [T-Echo-Lite-Eapper_V1.0](./project/T-Echo-Lite_V1.0/T-Echo-Lite-Eapper_V1.0.pdf)
