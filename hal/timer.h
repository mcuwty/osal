#ifndef TIMER_H
#define TIMER_H

#include "type.h"

#define TICK_PERIOD_MS      10

extern void OSAL_TIMER_TICKINIT(void);
extern void OSAL_TIMER_TICKSTART(void);
extern void OSAL_TIMER_TICKSTOP(void);

#endif
