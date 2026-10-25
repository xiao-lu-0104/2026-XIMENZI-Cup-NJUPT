#include "ring_buff.h"

/**
  * @brief 		环形队列初始化
  * @param  	环形队列结构体指针
  * @retval 	无
  */
void ring_buffer_init(ring_buffer_t *rb)
{
	rb ->datacount = 0;
	rb ->head = 0;
	rb ->tail = 0;
}

/*****************逻辑层*****************/

/**
	* @brief 		环形队列是否空
  * @param  	环形队列结构体指针
  * @retval 	true or false
  */
bool ring_buffer_is_empty(ring_buffer_t *rb)
{
	return (rb ->datacount == 0);
}

/*****************读写层*****************/
/**
  * @brief 		获取队列中数据量
  * @param  	环形队列结构体指针
  * @retval 	数据量
  */
uint16_t ring_buffer_get_data_count(ring_buffer_t *rb)
{
	return rb ->datacount;
}

/**
  * @brief 		串口中断原子操作单字节入队
  * @param  	环形队列结构体指针
  * @param  	入队字
  * @retval 	true or false
  */
bool ring_buffer_itr(ring_buffer_t *rb, uint8_t byte)
{
	if(rb ->datacount >= USART_RX_BUFF_LEN)						//没福我硬享，没余量我不进
	{
		return false;
	}
	
	rb ->buffer[rb ->head] = byte;										//单字节入队
	rb ->head = (rb ->head + 1) % USART_RX_BUFF_LEN;	//头指针顺次移动
	rb ->datacount++;																	//数据自增
	
	return true;
}

/**
  * @brief 		主循环中安全出队函数（屏蔽了中断）
  * @param  	环形队列结构体指针
  * @param  	读取的数据的指针
  * @param  	希望读取的长度
  * @retval 	最后读取的长度
  */
uint16_t ring_buffer_read_safe(ring_buffer_t *rb, uint8_t *data, uint8_t len)
{
	uint8_t read_len;
	uint16_t tail_pri;
	uint8_t TTE;
	
	__disable_irq();																						//关中断，防止中断改变队列后恢复现场时数据出错
	
	read_len = (len < rb ->datacount) ? len : rb ->datacount;		//要读的长度应该是：长度现在是不是小于已有的数据？是的话我就读你想要的长度，如果大了那就有多少我丢给你多少QAQ
	tail_pri = rb ->tail;																			  //锁定读指针
	
	TTE = USART_RX_BUFF_LEN - tail_pri;													//tail到End的余量
	if(read_len < TTE)	//case1
	{
		memcpy(data, &(rb ->buffer[tail_pri]), read_len);
	}	
	else								//case2
	{
		memcpy(data, &(rb ->buffer[tail_pri]), TTE);
		memcpy(data + TTE, &(rb ->buffer[0]), read_len - TTE); 
	}
	
	rb ->tail = (tail_pri + read_len) % USART_RX_BUFF_LEN;			//更新tail指针，应该是上一次的tail加上取出的总个数之后再对于总量取余
	rb ->datacount -= read_len; 																//数据量减去从队列中取出的量
	
	__enable_irq();																							//重新打开中断
	return read_len;
}
