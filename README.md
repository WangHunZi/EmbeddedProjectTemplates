# 说明

本项目主要的包含一些驱动程序、外设的使用。

**软件环境：** 
- CLion

    ```
    CLion 2023.1
    Build #CL-231.8109.174, built on March 28, 2023
    ```

- STM32CubeMX

    ```
    Version 6.8.0
    ```

**硬件环境：** STM32H750VBT6、STM32F407ZGT6【最小系统板和正点原子开发板】。

**初期目标：** 由于驱动程序是相对固定的，和芯片的型号没有太多的差别，为了减少这样的差别的存在，驱动程序的通信方式暂时以软件模拟的形式展现。完成在个人项目和实际比赛中要用到的一些常见的驱动程序。


# LVGLSimulator

在CLion中实现LVGL模拟器环境。对应的文章：[在CLion中搭建LVGL模拟器](https://blog.csdn.net/qq_44656481/article/details/125208978?spm=1001.2014.3001.5501)

# BootLoader_Application

**软件环境：**

1. CLion
2. STM32CubeMX

**硬件环境：**

1. STM32H750VBT6
2. ST-Link

本工程在STM32H750VBT6上实现BootLoader，并跑通LVGL。
