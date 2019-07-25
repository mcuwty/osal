#ifndef OSAL_TIMER_H
#define OSAL_TIMER_H

#include "type.h"
#include "timer.h"

#define TIMER_DECR_TIME       	1 	//任务定时器更新时自减的数值单位

extern void osalTimerInit(void);
extern uint8 osal_start_timerEx(uint8 task_id, uint16 event_id, uint16 timeout_value);
extern uint8 osal_start_reload_timer(uint8 taskID, uint16 event_id, uint16 timeout_value);
extern uint8 osal_stop_timerEx(uint8 task_id, uint16 event_id);
extern uint16 osal_get_timeoutEx(uint8 task_id, uint16 event_id);
extern uint8 osal_timer_num_active(void);
extern uint32 osal_GetSystemClock(void);
extern void osal_update_timers(void);

#endif
