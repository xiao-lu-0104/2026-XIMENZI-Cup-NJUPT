#include "event_buff.h"

/**
  * @brief 		消息队列初始化
  * @param  	需要被初始化的消息队列
  * @retval 	无
  */
void event_queue_init(Eventqueue_t *eq)
{
	eq->tail = 0;
	eq->head = 0;
	eq->datacount = 0;
}

/*****************逻辑层*****************/

/**
  * @brief 		检测当前队列是否空
  * @param  	需要检测的消息队列
  * @retval 	空则返回true，非空则返回false
  */
bool event_queue_is_empty(Eventqueue_t *eq)
{
	if(!(eq->datacount))
	{
		return true;
	}
	else
		return false;
}

/*****************读写层*****************/

/**
  * @brief 		读取当前队列中的事件数量
  * @param  	需要读取的消息队列
  * @retval 	当前队列中的数量
	* @notice		内部服务函数static
  */
static uint16_t event_queue_get_data_count(Eventqueue_t *eq)
{
	return (eq->datacount);
}

/**
  * @brief 		消息队列入队
  * @param  	需要入的消息队列
  * @param  	需要入队的事件
  * @retval 	入队成功返回true，入队失败返回false
  */
bool event_queue_enter(Eventqueue_t *eq, Event e)
{
	if(event_queue_get_data_count(eq) >= EVENT_QUEUE)	//没空余位置直接丢弃消息，勒令重发
	{
		return false;
	}
	else
	{
		eq->buffer[eq->head] = e;
		eq->head = (eq->head + 1) % EVENT_QUEUE;
		eq->datacount++; 
		return true;
	}
}

/**
  * @brief 		消息队列安全出队（非阻塞）
	* @param  	需要离开的队伍
  * @param  	需要出队的事件指针
  * @retval 	出成功返回true，入队失败返回false
  */
bool event_queue_out(Eventqueue_t *eq, Event *e)
{
	if(event_queue_is_empty(eq))
	{
		return false;
	}
	
	else
	{
		__disable_irq();	//关中断保证安全
		
		*e = eq->buffer[eq->tail];
		eq->tail = (eq->tail + 1) % EVENT_QUEUE;
		eq->datacount--;
		
		__enable_irq();
		return true;		
	}

}

/****************************End*****************************/
