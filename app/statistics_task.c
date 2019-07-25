/**
 * @file statistics_task.c
 * @brief 统计任务
 * @version 0.1
 * @date 2019-07-25
 * @author WatWu
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "task_event.h"

uint8 statistics_task_id;                    //记录统计任务的任务ID

/**
 * @brief 任务初始化
 * @param task_id [初始化时分配给当前任务的任务ID，标记区分每一个任务]
 */
void statistics_task_init(uint8 task_id)
{
    statistics_task_id = task_id;
}

/**
 * @brief 当前任务的事件回调处理函数
 * @param task_id       [任务ID]
 * @param task_event    [收到的本任务事件]
 * @return uint16       [未处理的事件]
 */
uint16 statistics_task_event_process(uint8 task_id, uint16 task_event)
{
    if(task_event & SYS_EVENT_MSG)       //判断是否为系统消息事件
    {
        osal_sys_msg_t *msg_pkt;
        msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);      //从消息队列获取一条消息

        while(msg_pkt)
        {
            switch(msg_pkt->hdr.event)      //判断该消息事件类型
            {
                case PRINTF_STATISTICS:
                {
                    int count = *(int*)(((general_msg_data_t*)msg_pkt)->data);
                    printf("Statistics task receive print task printf count : %d\n", count);
                    break;
                }

                default:
                    break;
            }

            osal_msg_deallocate((uint8 *)msg_pkt);                  //释放消息内存
            msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);  //读取下一条消息
        }

        // return unprocessed events
        return (task_event ^ SYS_EVENT_MSG);
    }

    return 0;
}
