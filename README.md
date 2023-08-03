# About

[English]() | [中文](README-CN.md)

This repository contains some driver code and hardware peripheral code.

**Soft Environment：** 

Windows 11, CLion2023, STM32CubeMX 6.8.1, openocd 0.12.0

**Hardware Environment:**

Weact Studio STM32H750VBT6, STM32F407ZGT6


# LVGLSimulator

Implemented LVGL Simulator in CLion, and here is the corresponding blog link:

[How to implement the LVGL simulator in CLion ?](LVGLSimulator/How%20to%20implement%20the%20LVGL%20simulator%20in%20CLion.md)

# BootLoader_Application

Here is specfic documentation for this project:
[How to implement STM32H750VBT6's Bootloader in CLion?](BootLoader_Application/How%20to%20implement%20Bootloader%20on%20STM32H750VBT6%20in%20CLion.md)

It took a lot of time and experience to realize this Bootloader, but I still feel very happy that is was successfully completed in the end.

# ExFlashLVGL

This project implements downloading LVGL to external Flash.

# InternalFlash

Write a program to measure and verify the internal Flash size of STM32H750VBT6. My development board is 1024KB, if it exceeds this range, it will enter HardFault_Handler.

Here is a documentation in the project file:
[How to measure the internal flash size and verify it?](InternalFlash/How%20to%20measure%20the%20internal%20flash%20size%20and%20verify%20it.md)

# STM32_DSP_Library

Port the latest arm-dsp library to STM32H750 using CLion. The newest library includes window functions that are not present in STM32CubeMX.

It's important to note that directly adding "CMSIS-DSP/Source/*.*" to file(GLOB_RECURSE SOURCES) in CMakeLists.txt can lead to compilation issues. This is because some files within the CMSIS-DSP\Source directory utilize #include "xxxx.c", causing multiple definition errors during compilation.

For instance, BasicMathFunctions.c and BasicMathFunctionsF16.c housed under the BasicMathFunctions directory include some source files, leading to these complications.

The solution I found was to delete the files that share the same name. This approach might seem a bit crude, but after numerous attempts, it proved to be the simplest method.

I've also reported this issue to Arm.

# TODO-List

- [ ] Translate Blog into English
