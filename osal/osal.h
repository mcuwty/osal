#ifndef OSAL_H
#define OSAL_H

#include "type.h"

typedef struct
{
    void   *next;
    uint16 len;
    uint8  dest_id;
} osal_msg_hdr_t;

typedef struct
{
    uint8 event;
    uint8 status;
} osal_event_hdr_t;

typedef struct
{
    osal_event_hdr_t hdr;
    uint8* Data_t;
} osal_sys_msg_t;                       //默认系统消息结构体

typedef void *osal_msg_q_t;

extern osal_msg_q_t osal_qHead;
extern uint8 tasksCnt;                  //任务数量统计

extern uint8 osal_init_system(void);
int osal_strlen(char *pString);
void *osal_memcpy(void *dst, const void *src, unsigned int len);
void *osal_revmemcpy(void *dst, const void *src, unsigned int len);
void *osal_memdup(const void *src, unsigned int len);
uint8 osal_memcmp(const void *src1, const void *src2, unsigned int len);
void *osal_memset(void *dest, uint8 value, int len);

#endif
