# 说明

需要使用DSP库，但是STM32CubeMX的最新版本DSP库还在1.3，而gihub上的最新版本已经到了1.14了。因此移植一下，作为模板。

# 步骤

1. 通过STM32CubeMX配置好自己的工程，比如我的工程命名为STM32_DSP_Library

2. 下载[CMSIS-DSP](https://github.com/ARM-software/CMSIS-DSP)到自己的项目中

3. 修改`STM32_DSP_Library/CMakeLists.txt`

    - 定义变量`CMSISCORE`，原因是`CMSIS-DSP/Source`的`CMakeLists.txt`使用到了`CMSISCORE`这个变量。该变量指示的位置是该项目的`Drivers/CMSIS/`。因为`CMSIS-DSP`要使用STM32CubeMX生成的`Drivers/CMSIS/Include`中的一些头文件。

      ```cmake
      set(CMSISCORE "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/")
      add_subdirectory(CMSIS-DSP/Source bin_dsp)
      ```

    - 在`add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})`之后添加`target_link_libraries(${PROJECT_NAME}.elf PUBLIC CMSISDSP)`。目的是把CMSISDSP连接到`${PROJECT_NAME}.elf`，这样一来，代码中就可以调用CMSISDSP的相关API了。

      ```cmake
      add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})
      target_link_libraries(${PROJECT_NAME}.elf PUBLIC CMSISDSP)
      ```

4. 在main.c中测试相应的函数并编译即可。

# 参考

[ARM-software/CMSIS-DSP: CMSIS-DSP embedded compute library for Cortex-M and Cortex-A (github.com)](https://github.com/ARM-software/CMSIS-DSP/issues/114)