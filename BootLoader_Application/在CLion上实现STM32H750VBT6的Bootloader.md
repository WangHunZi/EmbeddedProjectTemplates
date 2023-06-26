# 说明

**起因：** 平常使用的环境是CLion，在使用WeAct的STM32H750VBT6的时候发现这个芯片内部的Flash太小，根本不能移植一些库，如LVGL。

**本文内容：** 事无巨细地介绍了实现Bootloader的全部过程，并且还包含有对使用openocd的一些问题的解决办法。所有代码提供百度网盘以及Github仓库链接。

**目的：** 目的在于让大家少踩坑，帮助大家实现Bootloader。自己也记录一下学习中的困难和感受。

**关键字：** CLion STM32H750VBT6 WeAct 反客 Flash Bootloader openocd perl 

**软件环境：** Windows 11、CLion 2023.1.4 、STM32CubeMX 6.8.1、openocd 0.12.0

**仓库链接：** [suguguan/EmbeddedProjectTemplates (github.com)](https://github.com/suguguan/EmbeddedProjectTemplates)

**百度网盘链接：** 

# Bootloader的基本概念

由于自带的Flash太小，我们代码只能存到外部的Flash中去。这里存在一个问题：

> 假设我们已经通过某种方式将代码下载到了外部的Flash中，单片机应该如何去执行外部Flash中的代码？

我实现的Bootloader存在两个工程，一个为Bootloader另一个为Application。Bootloader执行跳转，Application中实现实际的代码。

单片机上电后会自己执行内部Flash中的代码，为了能够执行外部Flash的代码，我们要在内部Flash中增加一段代码，使其能够跳转到外部Flash中去，像是函数跳转一样，这是Bootloader要做的工作。跳转过去后，Application要重新初始化，接管控制权，相当于进入了第二个**main**函数，且不再返回。其实这个过程的内部执行还是挺复杂的。

# Bootloader

配置好QSPI和串口，然后跑一下外部Flash的测试程序。确保自己能够读取到外部Flash的ID，能读能写，并且确保能够进入内存映射模式。

然后清除测试程序，在跳转程序之前执行内存映射函数，然后在while(1)之前执行跳转程序。其他不做改变。

也要注意mmu，cache，我们要关闭这两个功能。

Bootloader的cfg文件内容如下：

```perl
source [find interface/stlink.cfg]
source [find target/stm32h7x.cfg]
```

# Application

正如之前所说，我们需要在main函数的一开始，就执行一些代码接管来自Bootloader的跳转后的控制权，让单片机按照外部Flash的地址顺次执行藏于中间的指令。

![image-20230626161411982](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626161411982.png)

除此之外我们还需要修改**STM32H750VBTX_FLASH.ld**这个文件的FLASH的地址和大小，分别为`0x90000000`和`8192K`。

![image-20230626161258832](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626161258832.png)

# STM32的QSPI

## 配置QSPI

配置里有个东西我未作验证——时钟频率。我不确定时钟频率会不会影响Bootloader，因此我也没有配置为480MHz，具体可以查看ioc文件里的配置。但是我后面会做验证，并且更新文档和仓库。

## Application的cfg文件

这是整个Bootloader工程实现的关键步骤。这里我会详细阐述。

### RCC Registers Configure

![image-20230626154530618](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626154530618.png)

这里的内容来源于**stm32h7x_dual_qspi.cfg**，该文件位于**openocd\share\openocd\scripts\board\stm32h7x_dual_qspi.cfg**。

### QSPI Registers Configure

#### 引脚寄存器

通过查看WeAct Studio提供的原理图，以及STM32CubeMX生成的代码，我们可以知道对应的引脚以及功能：

| 引脚 | 功能            | AFx  |
| :--: | :-------------- | :--: |
| PB6  | QUADSPI_BK1_NCS | AF10 |
| PB2  | QUADSPI_CLK     | AF09 |
| PD11 | QUADSPI_BK1_IO0 | AF09 |
| PD12 | QUADSPI_BK1_IO1 | AF09 |
| PE2  | QUADSPI_BK1_IO2 | AF09 |
| PD13 | QUADSPI_BK1_IO3 | AF09 |

引脚、功能、AFx均可以在quadspi.c的HAL_QSPI_MspInit函数中进行查看。这对我们理解下图也就是cfg中的这段配置项有帮助。

![image-20230626144007878](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626144007878.png)
假设你的引脚对应的功能和上面的不同，我们又应该怎么样去生成这样的配置项呢？

1. 下载并安装perl工具以及并这个git仓库中[openocd/contrib/loaders/flash/stmqspi at master · openocd-org/openocd · GitHub](https://github.com/openocd-org/openocd/tree/master/contrib/loaders/flash/stmqspi)下载**gpio_conf_stm32.pl**

   - 在Terminal中执行**perl -v**，查看是否安装好perl工具

2. 修改下载好的**gpio_conf_stm32.pl**中的第30行到32行为如下内容

   - ```perl
     my $GPIO_BASE = 0x58020000;
     my $Conf = "PB06:AF10:H, PB02:AF09:H, PD13:AF09:H, PE2:AF09:H, PD12:AF09:H, PD11:AF9:H";
     my $STM32F1 = 0;
     ```

   - **GPIO_BASE**好理解，就是GPIO的基地址，用笨办法比如直接查看HAL库，比如查看芯片手册等也是可以计算出来，但是后面还有很多外设的基地址，没有取巧的办法好像会比较困难。这一点参见后文的**在CLion中调试**，这一节。

   - 第二行就要用到刚才提到的引脚、功能、AFx了。但是**H**是什么意思？如果你仔细看了**gpio_conf_stm32.pl**最开始的注释就会知道，这是引脚的速率。

     ![image-20230626150709577](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626150709577.png)

   - 第三行用以变量声明，默认为0，配置为1就会按照F1来对寄存器的地址进行计算，后面的代码有写：

     ![image-20230626151029058](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626151029058.png)

   - 小提示：你可以查看该文件的注释部分，获取编写规范和更多信息。

3. 修改好以后，在Terminal中执行`perl .\gpio_conf_stm32.pl`

   ![image-20230626151449173](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626151449173.png)

   - 这里的主要作用是对QSPI的对应寄存器写具体的值，mmw的作用是给特定地址设置值和清除值。

#### 功能寄存器

![image-20230626155146692](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626155146692.png)

这部分也是由**stm32h7x_dual_qspi.cfg**修改而来，需要注意这是**dual qspi**，因此我们需要做一些改动，使之成为**single qspi**，感觉需要对QSPI的寄存器有一定的了解，因此这部分参考了[RT-Thread-使用openOCD擦写ART_Pi外部qspi_flashRT-Thread问答社区 - RT-Thread](https://club.rt-thread.org/ask/article/95a03d2494e01ada.html)

## 其他型号的单片机

> H743和H750系列没有octoflash，后面新出的系列带，这个ST官方出的板子带，你可以参考玩下，有对应的例子。他们用的旺宏的，你配置为华邦的也很方便。
>
> [STM32H7 OCTOSPI驱动W25Q128 - STM32H7 - 硬汉嵌入式论坛 - Powered by Discuz! (armbbs.cn)](https://www.armbbs.cn/forum.php?mod=viewthread&tid=110668)

**反客：**

比如你是STM32H7B0VBT6等非H743和H750的芯片，则可以参考这篇文章[使用OpenOCD+VSCode一键烧录Boot+App到内置+外置flash | Haobo's Blog (haobogu.github.io)](https://haobogu.github.io/posts/keyboard/openocd-ospi-flash/)和这个仓库**[Peakors/STM32H7B0VBT6_Template](https://github.com/Peakors/STM32H7B0VBT6_Template)**。因为这是OSCTOFlASH，整个寄存器存在差异。配置文件可以查看openocd\share\openocd\scripts\board\stm32h7b3i-disco.cfg。

**RT-Thread ART-PI** 和 **WeAct**

H743【未验证】和H750就可以使用本工程，否则要对应做参考和修改。也需要注意，可能QSPI的引脚存在着差别，需要自己根据前文编写脚本，并生成对应内容。【比如ART-PI和WeAct的STM32H750VBT6最小系统板引脚就不同】，可以参考这篇文章[RT-Thread-使用openOCD擦写ART_Pi外部qspi_flashRT-Thread问答社区 - RT-Thread](https://club.rt-thread.org/ask/article/95a03d2494e01ada.html)

# 问题和错误

## Openocd相关

### Openocd下载限制

比如STM32H750存在2MB内存，我还未做实验，但是已经和反客技术交流群的朋友[@Peakors](https://github.com/Peakors)交流了一下：

1. http://www.wujique.com/2020/03/22/%E5%9C%A8stm32cubeide%E4%B8%AD%E7%94%A8openocd%E8%B0%83%E8%AF%95stm32h750/

   https://m.newsmth.net/article/Circuit/350573

2. 总结就是通过修改**stm32h7x.cfg**的第90行的`flash bank $_CHIPNAME.bank1.cpu0 stm32h7x 0x08000000 0 0 0 $_CHIPNAME.cpu0`内容为`flash bank $_CHIPNAME.bank1.cpu0 stm32h7x 0x08000000 0x200000 0 0 $_CHIPNAME.cpu0`

   也可以修改为

   `flash bank $_CHIPNAME.bank1.cpu0 stm32h7x 0x08000000 $_FLASH_SIZE 0 0 $_CHIPNAME.cpu0`

   然后在**自己的**cfg文件中进行写入一句`set FLASH_SIZE 0x200000`，本质还是一样的，换汤不换药。

   最后都还是要修改一下**ld**文件的FLASH大小，以免编译时出现超了内存的错误。

3. 如何验证2MB空间确实可用？

   - 定义一个声明在0x08100000这个地址的数组（只要是超出0x80020000这个地址就应该可以）
   - 定义一个大数组，中间插入某个数据，看能不能正常读写

   以上两种方式都可行。

### CLion下载出现问题

CLion通过openocd进行下载的时候会有某些情况下载不进去的问题。可以通过在下载的时候长按reset按键，下载的半途中松开，注意是半途中松手，不要一直按着，至于半途是多久，需要自己多次实验。

这样的方式很不优雅，也可以通过注释一些语句实现，但是有时候还是得手动reset。reset确实很头疼，连Openocd的文档也提到了这一点：

![image-20230626165737620](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626165737620.png)

主要和如下语句有关系：

1. reset_config srst_only

   ![image-20230626165347573](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626165347573.png)

   这好像主要和JTAG有关

2. transport select hla_swd

   ![image-20230626165955149](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626165955149.png)

   然后往上看，看到

   ![image-20230626170153823](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626170153823.png)

   继续往上看

   ![image-20230626170220853](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626170220853.png)

   在使用OpenOCD调试工具时，根据使用的OpenOCD版本和调试适配器，可以有多种传输方式可用于与调试目标进行通信，或者用于编程闪存存储器。感觉也是可以注释掉的。

### 命令行的下载方式出现问题

问题一：telnet连接后，执行flash write_image命令时，告知文件不存在【其实是存在的】

1. 确保路径没有写错，也要区分反斜杠和斜杠，windows中是斜杠，openocd是反斜杠
2. 用管理员打开终端，再次执行命令。

问题二：telnet连接后，执行flash info 1出现问题

1. 首先要执行执行halt

2. 执行过了halt再执行还是出现问题绝大可能是cfg文件编写得不正确

   - 也包括Bootloader没有弄好内存映射就下载Application的bin文件

3. flash info 1能够读取正确，但是flash image_write就出现下载失败的问题

   现在想这个问题还是觉得配置文件存在问题。

问题三：如何命令行下载？

看[openocd手册](https://openocd.org/doc-release/pdf/openocd.pdf)，第12页开始。

### 重复下载

下载成功一次后，Bootloader能够正常使用了，就不用再每次编写完成Application这个工程后先下载Bootloader这个工程再下载了Application的bin文件。我们只需要修改Application的代码，然后直接烧录进单片机即可，如果不修改Bootloader，就不用再下载Bootloader这个工程。

## 最小系统板

原理图中引脚没注意看，其实和STM32CubeMX的默认引脚位置不一样，导致QSPI访问不到外部Flash。

因为没有CH340，不得已自己接了一个USB转TTL，发现串口无法输出，检查后才看清楚，原来板子上的USART1不在最后一排，在倒数第二排。最后一排的丝印因为开孔和上面的丝印没有对称，导致看漏了。

## 在CLion中调试

### 查看外设地址

![image-20230626163703429](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626163703429.png)

1. 打断点然后进入调试

2. 点击**Load .svd file**后进入到**Keil\Arm\Packs\Keil\STM32H7xx_DFP\3.1.0\CMSIS\SVD\\**中找到**STM32H750.svd**，然后选择对应外设，比如RCC、QSPI……

   <img src="https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626164011223.png" alt="image-20230626164011223" style="zoom:25%;" />

   ![image-20230626164251058](https://wanower.oss-cn-beijing.aliyuncs.com/img/image-20230626164251058.png)

   这样就可以查看寄存器的对应地址以及对应的值了，主要的作用是可以很方便的比对cfg文件里的对应地址了，虽然不太理解写入寄存器的值会有什么效果，但是好歹能够通过cfg文件的注释和CLion查看到的寄存器地址进行比对确认不是不是同一个芯片或者不同芯片能否套用……

### 在外部Flash中进行debug

很自然地就可以打上断点然后进行debug，没有需要特殊配置的地方。但前提是你跑通了整个流程，这也是显然的。

# 参考

[使用OpenOCD+VSCode一键烧录Boot+App到内置+外置flash | Haobo's Blog (haobogu.github.io)](https://haobogu.github.io/posts/keyboard/openocd-ospi-flash/)

[OpenOCD | Haobo's Blog (haobogu.github.io)](https://haobogu.github.io/posts/keyboard/openocd/)

[RT-Thread-使用openOCD擦写ART_Pi外部qspi_flashRT-Thread问答社区 - RT-Thread](https://club.rt-thread.org/ask/article/95a03d2494e01ada.html)

[STM32H7 OCTOSPI驱动W25Q128 - STM32H7 - 硬汉嵌入式论坛 - Powered by Discuz! (armbbs.cn)](https://www.armbbs.cn/forum.php?mod=viewthread&tid=110668)

[openocd手册](https://openocd.org/doc-release/pdf/openocd.pdf)
