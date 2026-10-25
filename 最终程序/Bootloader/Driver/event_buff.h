#ifndef __EVENT_BUFF_H
#define __EVENT_BUFF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define EVENT_QUEUE	32

/*=========事件枚举=========*/
typedef enum
{
	EVENT_NONE = 0,
	EVENT_LED_Toggle,
	EVENT_SOFT_Reset,
	EVENT_AUTO_REPORT_START,
	EVENT_AUTO_REPORT_STOP,
	EVENT_AUTO_REPORT_SEND,
	EVENT_ALARM_CHECK,
	EVENT_ALARM_MSG,
	EVENT_ENTER_SLEEP,
}EventType;

/*=========事件结构体=========*/
typedef struct
{
	EventType type;
	uint16_t param1;
	uint16_t param2;
}Event;

/*=========队列结构体=========*/
typedef struct
{
	Event buffer[EVENT_QUEUE];	//初始化结构体时就已经被栈内定义，无需初始化数组
	volatile uint16_t tail;
	volatile uint16_t head;
	volatile uint16_t datacount;
}Eventqueue_t;
/*=========FIFO=========*/

/*=========函数定义=========*/
void event_queue_init(Eventqueue_t *eq);
bool event_queue_is_empty(Eventqueue_t *eq);
static uint16_t event_queue_get_data_count(Eventqueue_t *eq);
bool event_queue_enter(Eventqueue_t *eq, Event e);
bool event_queue_out(Eventqueue_t *eq, Event *e);

#ifdef __cplusplus
}
#endif

#endif

/****************************End*****************************/
