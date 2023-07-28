# About

[English]() | [中文](README-CN.md)

This repository contains some driver code and hardware peripheral code.

**Soft Environment：** 

Windows 11, CLion2023, STM32CubeMX 6.8.1, openocd 0.12.0

**Hardware Environment:**

Weact Studio STM32H750VBT6, STM32F407ZGT6


# LVGLSimulator

Implemented LVGL Simulator in CLion, and here is the corresponding blog link:

[How to implement the LVGL simulator in CLion ?](LVGLSimulator/How to implement the LVGL simulator in CLion.md)

# BootLoader_Application

Here is specfic documentation for this project:
[How to implement STM32H750VBT6's Bootloader in CLion?](BootLoader_Application/How to implement Bootloader on STM32H750VBT6 in CLion.md)

It took a lot of time and experience to realize this Bootloader, but I still feel very happy that is was successfully completed in the end.

# ExFlashLVGL

This project implements downloading LVGL to external Flash.

# InternalFlash

Write a program to measure and verify the internal Flash size of STM32H750VBT6. My development board is 1024KB, if it exceeds this range, it will enter HardFault_Handler.

Here is a documentation in the project file:
[How to measure the internal flash size and verify it?](InternalFlash/How to measure the internal flash size and verify it.md)

# TODO-List

- [ ] Translate Blog into English
