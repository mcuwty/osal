# OSAL

OSAL(operating system abstraction layer)，操作系统抽象层，是一种以实现多任务为核心的系统资源管理机制，实现了类似操作系统的某些功能，但并不能称之为真正意义上的操作系统。本OSAL仓库源码来源于TI CC2530的zigbee协议栈Z-Stack中，剥离了其应用相关及不通用的功能模块，提取其最核心的事件驱动型多任务内核。OSAL的实现源码非常精简高效，总共约1100多行，全部纯C语言实现，最小资源占用要求为RAM约512Byte，ROM约2KB。理论上可以移植至全部支持C语言的芯片平台。

## OSAL移植的接口

| OSAL接口 | 说明 |
| -- | -- |
| Message Management API | 消息管理 |
| Task Synchronization API | 任务同步 |
| Timer Management API | 定时器管理 |
| Memory Management API | 内存管理 |

## 移植说明

1. 完成hal\timer.c文件，为系统提供滴答时钟，建议滴答心跳的周期为1～10ms，并对应修改hal\timer.h中的宏定义TICK_PERIOD_MS为相应心跳毫秒值；
2. 修改osal\type.h文件中的全局中断开关宏定义（可为空），根据需要修改数据类型的宏定义，根据实际芯片字长修改“halDataAlign_t”类型；
3. 根据需要修改osal\osal_memory.h文件中的内存池大小定义，默认最大为32768字节，osal\osal_memory.c中osalMemHdr_t类型需要确保长度为16bit或以上，非8位单片机需要设定内存池的字节对齐；
4. 添加任务函数中的任务优先级数值大的任务则优先级高；
5. 根据需要修改osal\osal_memory.h文件中的OSALMEM_METRICS定义，有效则开启内存统计功能；

各API的使用可参考doc下的官方API手册《OSAL_API.pdf》。

## 动态内存管理拓展说明

OSAL中默认使用15位的数据标识管理内存，最大能管理32768字节，需要增加管理更多的动态内存可按照以下方式拓展：

1. 注释掉osal_memory.c中的内存大小编译限制；
2. 替换osal_memory.c中的全部uint16为osalMemHdr_t；
3. 修改osal_memory.h中的osalMemHdr_t类型宏为halDataAlign_t，确保芯片字长halDataAlign_t为32bit；
4. 修改osal_memory.c中的宏定义OSALMEM_IN_USE为0x80000000；

## 编译运行

本仓库在linux下可以直接编译运行基础例程，例程定义了两个任务，任务一使用定时器API进行定时触发打印事件，并累计打印次数，每累计5次就会向任务二发送统计事件，任务二接收任务一发送的统计事件后进行统计结果的打印输出。

编译：

```shell
wat@wat:~$ make
building ./app/main.c
building ./app/osal_main.c
building ./app/print_task.c
building ./app/statistics_task.c
building ./hal/timer.c
building ./osal/osal_msg.c
building ./osal/osal_event.c
building ./osal/osal_timer.c
building ./osal/osal_memory.c
building ./osal/osal.c
linking object to linux-osal-example.elf

real    0m0.585s
user    0m0.332s
sys     0m0.242s
```

运行：

```shell
wat@wat:~$ ./linux-osal-example.elf
Init hal timer ok !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Statistics task receive print task printf count : 5
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Statistics task receive print task printf count : 10
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
......
```
