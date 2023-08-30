# Funnyplaying Burnmaster Firmware

Based on Sanni's [cartreader](https://github.com/sanni/cartreader) firmware


## Building the firmware
**Requires [SEGGER Embedded Studio](https://www.segger.com/products/development-tools/embedded-studio/)**

To build the firmware open `GDCartReader.emProject` in Embedded Studio, then right click `Project 'GDCartReader'` and click `Build` the compiled firmware can be found at either `ProjectFolder/Release/Exe/GDCartReader.bin` or `ProjectFolder/Debug/Exe/GDCartReader.bin` depending on your build configuration, rename the file to `update.bin`and put it on your SD Card to flash it to the burnmaster

### Repository Notes:

This repository is setup to auto-build the firmware when a commit is made, builds can be found under the [Actions tab](https://github.com/HDR/Burnmaster-Firmware/actions), this makes it really easy to fork this repository and make your own changes without having to download Segger Embedded Studio
