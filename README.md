# Funnyplaying Burnmaster Firmware

Based on Sanni's [cartreader](https://github.com/sanni/cartreader) firmware


## Building the firmware
**Requires [SEGGER Embedded Studio (6.22a recommended)](https://www.segger.com/products/development-tools/embedded-studio/)** 

To build the firmware open `GDCartReader.emProject` in Embedded Studio, then right click `Project 'GDCartReader'` and click `Build` the compiled firmware can be found at `ProjectFolder/Debug/Exe/GDCartReader.bin`, you will need to rename this file to `update.bin` which can now be put at the root of your SD Card to update your BurnMaster

**Alternatively** you can simply clone this repository and make your changes, Github will automatically build your firmware (See Actions Tab)

### Repository Notes:

This repository is setup to auto-build the firmware when a commit is made, builds can be found under the [Actions tab](https://github.com/HDR/Burnmaster-Firmware/actions), this makes it really easy to fork this repository and make your own changes without having to download Segger Embedded Studio


This source code is provided directly by funnyplaying, i have nothing to do with the development of the firmware.

**Note:** Be aware that the source code is for 1.10 and not 1.12 as i have not recieved the source code for the latest version from FunnyPlaying.
