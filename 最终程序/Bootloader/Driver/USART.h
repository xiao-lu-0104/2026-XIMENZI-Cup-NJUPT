#ifndef	__USART_H
#define __USART_H

#include "HeaderFiles.h"

/*********外部变量*********/
extern uint8_t data_rcv;

/*********宏定义*********/


/*********函数定义*********/
void USART1_Init(uint32_t baudrate);
void USART1_SendData(uint8_t *Buf, uint16_t len);

#endif
