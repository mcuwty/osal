/****************************************************************************************
 * 文件名  ：serial_task.c
 * 描述    ：系统串口通信任务
 * 开发平台：
 * 库版本  ：
 ***************************************************************************************/
#include "application.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define PRINTF_LEN	200					//串口打印最大长度定义

uint8 Serial_TaskID;					//系统串口通信任务ID

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/
//串口通信任务初始化
void Serial_Task_Init(uint8 task_id)
{
	Serial_TaskID = task_id;

	//串口配置初始化
}

//串口通信任务事件处理
uint16 Serial_Task_EventProcess(uint8 task_id,uint16 task_event)
{
	if ( task_event & SYS_EVENT_MSG )   	//判断系统消息事件
  	{
  		osal_sys_msg_t *MSGpkt;    			//定义一个指向接受系统消息结构体的指针
	    //从消息队列获取消息  
	    MSGpkt = (osal_sys_msg_t *)osal_msg_receive( task_id ); 
    
	    while ( MSGpkt )
	    {
	      	switch ( MSGpkt->hdr.event )  	//判断消息事件
	      	{
	          	case OSAL_PRINTF:
	          		break;

	        	default:
	          		break;
	      	}

	      	// Release the memory
	      	osal_msg_deallocate( (uint8 *)MSGpkt );

	      	// Next  获取下一个消息
	      	MSGpkt = (osal_sys_msg_t *)osal_msg_receive( task_id );
	    }

    	// return unprocessed events
    	return (task_event ^ SYS_EVENT_MSG);
  	}
  	
	if(task_event & PRINTF_STR)
	{
	  	return task_event ^ PRINTF_STR;
	}

	return 0;
}

//消息邮箱使用例子
void osal_printf(char *format, ...)
{
	__va_list arg_ptr;
    General_SerialData_t *string;

    string = (General_SerialData_t*)osal_msg_allocate( sizeof(General_SerialData_t) + PRINTF_LEN );

	if(string != NULL)
	{	
		//打印消息结构体的数据指针偏移至申请到的内存的数据段
		//string->Data = (unsigned char*)( string + sizeof(osal_event_hdr_t) );
		string->Data = (unsigned char*)( string + 1 );

		va_start(arg_ptr, format);
	 	vsprintf((char*)string->Data, format, arg_ptr);
	 	va_end(arg_ptr);

		string->hdr.event = OSAL_PRINTF;
		string->hdr.status = 0;
		osal_msg_send( Serial_TaskID, (uint8 *)string );
	}	
}

