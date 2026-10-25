#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "HeaderFiles.h"

void System_Init(void);
void UsrFunction(void);

/* Protocol 接口（链接 app/Protocol 的 .c 文件） */
void protocol_frame_parser(ring_buffer_t *buf);
void Protocol_SendFrame(uint16_t dev_id, uint8_t type, uint16_t cmd, uint8_t *data, uint8_t len);
void Protocol_SendOK(uint16_t dev_id, uint16_t cmd);
void Protocol_SendError(uint16_t dev_id);

/* OTA 状态机接口（USART ISR 调用） */
int  OTA_IsReceiving(void);
void OTA_ProcessByte(uint8_t byte);

#endif
