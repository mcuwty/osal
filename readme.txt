移植说明
1、完成OSAL\hal\timer.c文件，为系统提供时钟；
2、修改OSAL\include\type.h文件中的全局中断开关函数（可以为空），添加芯片对应的头文件，根据需要修改数据类型重定义，根据芯片字长修改halDataAlign_t类型；
3、根据需要修改OSAL\osal\osal_memory.h文件中的内存池大小定义，文件中osalMemHdr_t类型需要确保长度为16bit或以上，非8位单片机需要设定内存池的字节对齐；
4、添加任务函数中的任务优先级数值大的任务则优先级高；
5、根据需要修改OSAL\include\osal_memory.h文件中的OSALMEM_METRICS定义，有效则开启内存统计功能；