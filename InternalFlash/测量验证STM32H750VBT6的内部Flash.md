# 说明

在`Bootloader_Application`工程告一段落后，想到还应该验证一下内部Flash。这是本工程的由来。

本文主要包含实现这个工程时遇到的一些问题以及个人的理解。

# 文件修改

## STM32CubeMX配置

硬件方面的配置是最简的，除了基本的时钟以外，配置了一个LED灯。主频是480M，没有使能MPU。

# cfg文件

cfg文件未作改动，只是修改了本地的`stm32h7x.cfg`文件。它的路径是`openocd\share\openocd\scripts\target\stm32h7x.cfg`，将文件中的`flash bank $_CHIPNAME.bank1.cpu0 stm32h7x 0x08000000 0 0 0 $_CHIPNAME.cpu0`修改为`flash bank $_CHIPNAME.bank1.cpu0 stm32h7x 0x08000000 0x200000 0 0 $_CHIPNAME.cpu0`，代表着它拥有2MB的内存。

## 链接文件——STM32H750VBTX_FLASH.ld

修改了FLASH的大小：

```c
MEMORY
{
      FLASH (rx)     : ORIGIN = 0x08000000, LENGTH = 2048K
      ……
}
```

此时编译没有任何问题，然后容量的会发生变化：

![image-20230710224623992](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230710224623992-1689000393598-1.png)

其实还修改了`STM32H750VBTX_FLASH.ld`，修改的位置放到了后面进行叙述。

# 代码编写

主要分为两个部分，一部分是写，另外一部分是读验证。

## 数据写入

**考虑到换算的直观**，所以声明的是`uint8_t`类型的数组。

也就是数组中每个元素占用了8 bit（1B），由1KB = 1024 * 8bit，可得知1024个数组元素才构成1KB。如果有1MB的内存要验证，则需要1024 * 1024个数组元素。

我定义了一个`KB`的宏，值为`1024`，这意味着，我将以`KB`为单位进行验证。

### 数组的声明问题

这里面也有一些问题。声明的数组被`copy`一份到`RAM_D1`中去了，然后由于`array`太大，`RAM_D1`直接爆掉了。那我是怎么定义这个数组的呢：

```c
uint8_t array[DATA_SIZE] = { 0 }; // DATA_SIZE = 1016 * 1024
```

然后编译就出现了：

![image-20230710230057851](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230710230057851.png)

值得一提的是，这样的定义也是一样的结果：

```c
uint8_t array[DATA_SIZE] = { }; // DATA_SIZE = 1016 * 1024
```

一致的原因是C语言标准中存在这一条，参见[Array initialization - cppreference.com](https://en.cppreference.com/w/c/language/array_initialization)

![image-20230710231951969](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230710231951969.png)

解决这个问题有一个显而易见的思路，既然能够声明到RAM中，那也可以声明到内部的FLASH中。RAM没电就会掉数据嘛，那图片这样的数组怎么放的呢？图片肯定是放在内部FLASH中的，因此也可以断定，肯定是有这个解决办法的。还有一个办法是，一个从根本原因思考得来的，为什么数组的声明会到RAM中？第二个办法后面会有记录。

#### 解决办法一：变换声明方式

尝试变换声明方式：

```c
uint8_t array[DATA_SIZE] = { 1 }; // DATA_SIZE = 1016 * 1024
```

然后编译出现：

![image-20230710233221460](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230710233221460.png)

这样的声明是将数组的第一个元素声明为`1`，而数组的其他元素还是`0`。参见[Array initialization - cppreference.com](https://en.cppreference.com/w/c/language/array_initialization)

![image-20230710233622468](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230710233622468.png)

好像确实有点效果，虽然`RAM_D1`还是爆了，但是有点不同的是`FLASH`也确实占用了很多东西。为什么？我觉得还是和图片数组一个道理，如果你是个全为0的数组，那么说明这个数组很可能是在运行时决定具体内容的，由此断电可以不用保存，所以全部放在了RAM中了，当我们给数组中的某一位赋了具体的数值，那就得保存到FLASH中，可为什么RAM中也还是要占用空间呢？暂且留住这个疑问。

然后我尝试将数组定义为：

```c
const uint8_t array[DATA_SIZE] = { 1 }; // DATA_SIZE = 1016 * 1024
```

编译的结果确实是我想要的：

![image-20230710234546674](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230710234546674.png)

但问题是`const`声明的数组是没办法修改的呀。

`static`呢？结果和`uint8_t array[DATA_SIZE] = { 1 }; // DATA_SIZE = 1016 * 1024`的结果是一样的。

#### 解决办法二：修改STM32H750VBTX_FLASH.ld

难道真的没有办法了吗？然后我想为什么不同的声明会占用RAM或者FLASH呢？根本的原因压根不在具体的声明上，而是在**STM32H750VBTX_FLASH.ld**上。

`const uint8_t array[DATA_SIZE] = { 1 }; // DATA_SIZE = 1016 * 1024`的声明只占用了FLASH，因为**STM32H750VBTX_FLASH.ld**写好了：

```sh
  /* Constant data goes into FLASH */
  .rodata :
  {
	……
  } >FLASH
```

`uint8_t array[DATA_SIZE] = { 1 }; // DATA_SIZE = 1016 * 1024`的声明既占用了FLAHS又占用了RAM，因为**STM32H750VBTX_FLASH.ld**里边写好了的：

```sh
  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
	……
  } >RAM_D1 AT> FLASH
```

`x uint8_t array[DATA_SIZE] = { }; // DATA_SIZE = 1016 * 1024`的声明只占用了RAM，是因为如下：

```sh
 /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
	……
  } >RAM_D1
```

虽然看不明白，但是注释我是看的懂得。所以直接删去

```sh
  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
	……
  } >RAM_D1 AT> FLASH
```

最后的`>RAM_D1`就好了。好像挺有道理的，但实际上代码根本跑不了，不论怎么样往数组里写什么值，数组里的值都是`0`。所以我添加了另外一段定义：

```sh
  .data_flash :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data_flash)           /* .data sections */
    *(.data_flash*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } AT> FLASH
```

然后这样声明数组：

```c
uint8_t  __attribute__((section(".data_flash"))) array[DATA_SIZE] = { 1 };  // DATA_SIZE = 1016 * 1024
```

然后程序就可以跑了。

数组里的值考虑到数组是`uint8_t`，因此数组中每一位的范围是`0~255`，所以记录一下有多少个`0`，然后读取数组的时候验证一下`0`的个数即可。

## 数据验证

验证`0`的个数。如果不对就卡死。

然而实际上基本都会对，不会对的情况是数组大了超过了实际的FLASH大小。比如实际上FLASH并没有2MB，只有1MB，所以你要验证1.5MB就会失败，这种失败很好辩证，本项目下载进去后，灯亮了就是还没有到FLASH的极限。可以采用二分法，自己修改`DATA_SIZE`，不断地下载尝试。

# 总结

最后我测量出来的FLASH大小是1016KB，多一点就进了`HardFault_Handler`。超了的表现是：代码能够下载进去，但是调试就崩了。由于我在**while**中加了一段点灯程序，所以不用下载后都要调试一下，只需要看灯闪烁没闪烁即可。

# 参考

[Array initialization - cppreference.com](https://en.cppreference.com/w/c/language/array_initialization)