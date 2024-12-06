
### Compilation Instructions for nRF52 Bootloader on Linux Environment

* * *
1. Move the directory `Adafruit_nRF52_Bootloader` to `/home/xxx`.
    
2. Download, move and install the tool.
    
    Download [gcc-arm-none-eabi-10.3-2021.10-x86_64-linux](https://developer.arm.com/downloads/-/gnu-rm/10-3-2021-10), move `gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar` to `/home/xxx/`. Then extract it using:
    
        tar -xvf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar
        
    
3. Update and install tools.
    
        sudo apt-get update
        sudo apt-get install make
        sudo apt-get install python3-intelhex
        
    
4. Compile the bootloader.
    
    Extract the Adafruit_nRF52_Bootloader.7z archive, Navigate to the `Adafruit_nRF52_Bootloader` directory and compile it with the following command:
    
        cd /home/xxx/Adafruit_nRF52_Bootloader
        sudo make CROSS_COMPILE=/home/xxx/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi- BOARD=t_echo_lite_nrf52840 all
        
    
5. (Optional)Change ownership of the directory.
    
        sudo chown -R $USER /home/xxx
        
    
6. (Optional)Compile with debug information.
    
    To compile with additional debug information, use the following command:
    
        sudo make CROSS_COMPILE=/home/xxx/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi- BOARD=t_echo_lite_nrf52840 DEBUG=1 all
