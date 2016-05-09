#include "osal_event.h"
#include "osal_memory.h"

OsalTadkREC_t *TaskHead;
OsalTadkREC_t *TaskActive;

uint8 Task_id;				//任务ID统计
uint8 tasksCnt;				//任务数量统计

/*********************************************************************
 * @fn      osal_set_event
 *
 * @brief
 *
 *    This function is called to set the event flags for a task.  The
 *    event passed in is OR'd into the task's event variable.
 *
 * @param   byte task_id - receiving tasks ID
 * @param   byte event_flag - what event to set
 *
 * @return  ZSUCCESS, INVALID_TASK
 */
uint8 osal_set_event( byte task_id, uint16 event_flag )
{
  	OsalTadkREC_t *srchTask;

  	srchTask = osalFindTask( task_id );
  	if ( srchTask ) 
	{
	    // Hold off interrupts
	    HAL_ENTER_CRITICAL_SECTION();
	    // Stuff the event bit(s)
	    srchTask->events |= event_flag;
	    // Release interrupts
    	HAL_EXIT_CRITICAL_SECTION();
  	}
  	else
    	return ( INVALID_TASK );

  	return ( ZSUCCESS );
}

/*********************************************************************
 * @fn      osal_clear_event
 *
 * @brief
 *
 *    This function is called to clear the event flags for a task. The
 *    event passed in is masked out of the task's event variable.
 *
 * @param   uint8 task_id - receiving tasks ID
 * @param   uint8 event_flag - what event to clear
 *
 * @return  SUCCESS, INVALID_TASK
 */
uint8 osal_clear_event( uint8 task_id, uint16 event_flag )
{
  	OsalTadkREC_t *srchTask;

  	srchTask = osalFindTask( task_id );
  	if ( srchTask ) 
	{
	    // Hold off interrupts
	    HAL_ENTER_CRITICAL_SECTION();
	    // Stuff the event bit(s)
	    srchTask->events &= ~event_flag;
	    // Release interrupts
    	HAL_EXIT_CRITICAL_SECTION();
  	}
  	else
    	return ( INVALID_TASK );

  	return ( ZSUCCESS );
}
/***************************************************************************
 * @fn       osal_init_TaskHead
 *
 * @brief   init task link's head
 *
 * @param   none
 *
 * @return
 */
void  osal_init_TaskHead( void )
{
	TaskHead   = (OsalTadkREC_t *)NULL;
	TaskActive = (OsalTadkREC_t *)NULL;
	Task_id = 0;
 }

/***************************************************************************
 * @fn       osal_Task_init
 *
 * @brief   init task 
 *
 * @param   none
 *
 * @return
 */
void osal_Task_init(void)
{
	TaskActive = TaskHead;
	while(TaskActive)
	{
	  if(TaskActive->pfnInit)
	  {
		 TaskActive->pfnInit(TaskActive->taskID);
	  }
	  TaskActive = TaskActive->next;
	}
	TaskActive = (OsalTadkREC_t *)NULL;
}
/***************************************************************************
 * @fn       osal_add_Task
 *
 * @brief   osal_add_Task
 *
 * @param   none
 *
 * @return
 */
void  osal_add_Task(pTaskInitFn pfnInit,
                  	pTaskEventHandlerFn pfnEventProcessor,
                  	uint8 taskPriority)
{
	OsalTadkREC_t  *TaskNew;
	OsalTadkREC_t  *TaskSech;
	OsalTadkREC_t  **TaskPTR;
	TaskNew = osal_mem_alloc(sizeof(OsalTadkREC_t));
	if(TaskNew)
	{
		TaskNew->pfnInit = pfnInit;
		TaskNew->pfnEventProcessor = pfnEventProcessor;
		TaskNew->taskID = Task_id++;
		TaskNew->events = 0;
		TaskNew->taskPriority = taskPriority;
		TaskNew->next = (OsalTadkREC_t *)NULL;
		
		TaskPTR = &TaskHead;
	    TaskSech = TaskHead;

	    tasksCnt++;			//任务数量统计
		
		while(TaskSech)
		{
			if(TaskNew->taskPriority > TaskSech->taskPriority)
			{
				TaskNew->next = TaskSech;
				*TaskPTR = TaskNew;
				return;
			}
			TaskPTR = &TaskSech->next;
			TaskSech = TaskSech->next;
		}
		*TaskPTR = TaskNew;
	}
	return;
}

/*********************************************************************
 * @fn      osalNextActiveTask
 *
 * @brief   This function will return the next active task.
 *
 * NOTE:    Task queue is in priority order. We can stop at the
 *          first task that is "ready" (events element non-zero)
 *
 * @param   none
 *
 * @return  pointer to the found task, NULL if not found
 */
OsalTadkREC_t *osalNextActiveTask( void )
{
  	OsalTadkREC_t  *TaskSech;

  	// Start at the beginning
  	TaskSech = TaskHead;

  	// When found or not
  	while ( TaskSech ) 
	{
        if ( TaskSech->events)  
        {
		  	// task is highest priority that is ready
          	return  TaskSech;
      	}
      	TaskSech =  TaskSech->next;
  	}
  	return NULL;
}

/*********************************************************************
 * @fn      osalFindActiveTask
 *
 * @brief   This function will return the taskid task.
 *
 * NOTE:    Task queue is in priority order. We can stop at the
 *          first task that is "ready" (events element non-zero)
 *
 * @param   task_id
 *
 * @return  pointer to the found task, NULL if not found
 */

OsalTadkREC_t *osalFindTask( uint8 taskID )
{
	OsalTadkREC_t *TaskSech;
	TaskSech = TaskHead;
	while(TaskSech)
	{
		if(TaskSech->taskID == taskID)
		{
			return (TaskSech);
		}
		TaskSech = TaskSech->next;
	}
	return ((OsalTadkREC_t *)NULL);
}
