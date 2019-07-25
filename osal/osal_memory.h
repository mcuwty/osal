#ifndef OSALMEM_METRICS_H
#define OSALMEM_METRICS_H

#include "type.h"

#define MAXMEMHEAP              1024*6       //内存池大小，单位字节

#define OSALMEM_METRICS         1            //定义有效则开启内存统计

void osal_mem_init(void);
void osal_mem_kick(void);
void *osal_mem_alloc(uint16 size);
void osal_mem_free(void *ptr);

#if OSALMEM_METRICS
uint16 osal_heap_block_max(void);
uint16 osal_heap_block_cnt(void);
uint16 osal_heap_block_free(void);
uint16 osal_heap_mem_used(void);
uint16 osal_heap_high_water(void);
uint16 osal_heap_mem_usage_rate(void);
#endif

#endif
