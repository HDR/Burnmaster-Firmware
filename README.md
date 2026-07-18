# Funnyplaying Burnmaster Firmware

Based on Sanni's [cartreader](https://github.com/sanni/cartreader) firmware


## Building the firmware
### Requirements:
**[Keil MDK-ARM](https://www.keil.com/demo/eval/arm.htm)** (uVision 5)

**[GD32F10x_DFP](https://gd32mcu.com/data/documents/pack/GigaDevice.GD32F10x_DFP.2.0.3.pack)**

**[ARM Compiler 5.0.6 Update 7](https://developer.arm.com/downloads/view/ACOMP5?revision=r5p6-07rel1)** (Requires an account)

To build the firmware open `GDCartReader.uvprojx` in uVision, then right click `Project` and click `Build Target` (after setting up uVision, which won't be covered here)

---

This source code is provided directly by funnyplaying, i have nothing to do with the development of the firmware.

**Note:** This repo will only host the firmware source provided by funnyplaying, PR's will not be merged, and will be left open (or closed in the case of ai generated PR's)

**Note 2:** with 1.13 Funnyplaying switched from segger embedded studio to uVision, which requires you to register an account to get access to ARM Compiler 5.0.6