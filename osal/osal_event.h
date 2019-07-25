#ifndef  OSAL_EVENT_H
#define  OSAL_EVENT_H

#include "type.h"
#include "osal_timer.h"

typedef void (*pTaskInitFn)(uint8 task_id);
typedef uint16(*pTaskEventHandlerFn)(uint8 task_id, uint16 task_event);

/**
 * @brief 任务链表
 */
typedef struct OSALTaskREC
{
    struct  OSALTaskREC  *next;
    pTaskInitFn          pfnInit;               //任务初始化函数指针
    pTaskEventHandlerFn  pfnEventProcessor;     //任务事件处理函数指针
    uint8                taskID;                //任务ID
    uint8                taskPriority;          //任务优先级
    uint16               events;                //任务事件
} OsalTadkREC_t;

extern OsalTadkREC_t  *TaskActive;

extern void osal_start_system(void);
extern void osal_add_Task(pTaskInitFn pfnInit, pTaskEventHandlerFn pfnEventProcessor, uint8 taskPriority);
extern void osal_Task_init(void);
extern void osal_init_TaskHead(void);
extern OsalTadkREC_t *osalNextActiveTask(void);
extern OsalTadkREC_t *osalFindTask(uint8 taskID);
extern uint8 osal_set_event(byte task_id, uint16 event_flag);
extern uint8 osal_clear_event(uint8 task_id, uint16 event_flag);

#endif
