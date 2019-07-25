#ifndef OSAL_MSG_H
#define OSAL_MSG_H

#include "osal.h"
#include "type.h"

#define OSAL_MSG_NEXT(msg_ptr) 	((osal_msg_hdr_t *) (msg_ptr) - 1)->next
#define OSAL_MSG_ID(msg_ptr) 	((osal_msg_hdr_t *) (msg_ptr) - 1)->dest_id

extern uint8 * osal_msg_allocate(uint16 len);
extern uint8 osal_msg_deallocate(uint8 *msg_ptr);
extern uint8 osal_msg_send(uint8 destination_task, uint8 *msg_ptr);
extern uint8 *osal_msg_receive(uint8 task_id);
extern osal_event_hdr_t *osal_msg_find(uint8 task_id, uint8 event);
extern void osal_msg_enqueue(osal_msg_q_t *q_ptr, void *msg_ptr);
extern uint8 osal_msg_enqueue_max(osal_msg_q_t *q_ptr, void *msg_ptr, uint8 max);
extern void *osal_msg_dequeue(osal_msg_q_t *q_ptr);
extern void osal_msg_push(osal_msg_q_t *q_ptr, void *msg_ptr);
extern void osal_msg_extract(osal_msg_q_t *q_ptr, void *msg_ptr, void *prev_ptr);

#endif
