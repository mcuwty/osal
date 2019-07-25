/**
 * @file print_task.c
 * @brief 打印任务
 * @version 0.1
 * @date 2019-07-25
 * @author WatWu
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "task_event.h"

uint8 print_task_id;                    //记录打印任务的任务ID

/**
 * @brief 任务初始化
 * @param task_id [初始化时分配给当前任务的任务ID，标记区分每一个任务]
 */
void print_task_init(uint8 task_id)
{
    print_task_id = task_id;

    //开启一个循环定时器，每秒向打印任务发送PRINTF_STR事件
    osal_start_reload_timer(print_task_id, PRINTF_STR, 1000 / TICK_PERIOD_MS);
}

/**
 * @brief 当前任务的事件回调处理函数
 * @param task_id       [任务ID]
 * @param task_event    [收到的本任务事件]
 * @return uint16       [未处理的事件]
 */
uint16 print_task_event_process(uint8 task_id, uint16 task_event)
{
    if(task_event & SYS_EVENT_MSG)       //判断是否为系统消息事件
    {
        osal_sys_msg_t *msg_pkt;
        msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);      //从消息队列获取一条消息

        while(msg_pkt)
        {
            switch(msg_pkt->hdr.event)      //判断该消息事件类型
            {
                default:
                    break;
            }

            osal_msg_deallocate((uint8 *)msg_pkt);                  //释放消息内存
            msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);  //读取下一条消息
        }

        // return unprocessed events
        return (task_event ^ SYS_EVENT_MSG);
    }

    if(task_event & PRINTF_STR)
    {
        static int print_count = 0;
        printf("Print task printing, total memory : %d byte, used memory : %d byte !\n", MAXMEMHEAP, osal_heap_mem_used());

        print_count++;
        if(print_count % 5 == 0 && print_count != 0)
        {
            //向统计任务发送消息
            general_msg_data_t *msg;
            msg = (general_msg_data_t*)osal_msg_allocate(sizeof(general_msg_data_t) + sizeof(int));
            if(msg != NULL)
            {
                //消息结构体的data数据指针偏移至申请到的内存的数据段
                //msg->data = (unsigned char*)( msg + sizeof(osal_event_hdr_t) );
                msg->data = (unsigned char*)(msg + 1);

                msg->hdr.event = PRINTF_STATISTICS;
                msg->hdr.status = 0;
                *((int*)msg->data) = print_count;

                osal_msg_send(statistics_task_id, (uint8*)msg);
            }
        }

        return task_event ^ PRINTF_STR; //处理完后需要清除事件位
    }

    return 0;
}
