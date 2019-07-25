#ifndef TYPE_H
#define TYPE_H

//此处添加实际使用芯片的对应头文件
// #include "stm32f0xx.h"

#define ZSUCCESS                  1
#define INVALID_TASK              2
#define INVALID_MSG_POINTER       3
#define INVALID_EVENT_ID          4
#define NO_TIMER_AVAIL            5
#define TASK_NO_TASK              6
#define MSG_BUFFER_NOT_AVAIL      7

typedef unsigned char       BOOL;

//芯片硬件字长
typedef unsigned int        halDataAlign_t;

// Unsigned numbers
typedef unsigned char       uint8;
typedef unsigned char       byte;
typedef unsigned short      uint16;
typedef unsigned short      int16U;
typedef unsigned int        uint32;
typedef unsigned int        int32U;

// Signed numbers
typedef signed char         int8;
typedef signed short        int16;
typedef signed int          int32;

#ifndef FALSE
#define FALSE       0
#endif

#ifndef ARRAY_NULL
#define ARRAY_NULL '\0'
#endif

#ifndef TRUE
#define TRUE       1
#endif

#ifndef OPEN
#define OPEN       1
#endif

#ifndef CLOSE
#define CLOSE      0
#endif

#ifndef NULL
#define NULL       ((void*) 0 )
#endif

#ifndef HIGH
#define HIGH       1
#endif

#ifndef LOW
#define LOW        0
#endif

#ifndef SUCCESS
#define SUCCESS     1
#endif

#ifndef ERROR
#define ERROR       0
#endif

//#define CLI()         __set_PRIMASK(1)              // Disable Interrupts
//#define SEI()         __set_PRIMASK(0)              // Enable Interrupts
#define CLI()         ;              // Disable Interrupts
#define SEI()         ;              // Enable Interrupts

#define HAL_ENABLE_INTERRUPTS()         SEI()       // Enable Interrupts
#define HAL_DISABLE_INTERRUPTS()        CLI()       // Disable Interrupts
#define HAL_INTERRUPTS_ARE_ENABLED()    SEI()       // Enable Interrupts

#define HAL_ENTER_CRITICAL_SECTION()    CLI()
#define HAL_EXIT_CRITICAL_SECTION()     SEI()

#endif
