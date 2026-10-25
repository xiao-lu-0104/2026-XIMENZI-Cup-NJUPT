#ifndef __PROTOCOL_FRAME_H__
#define __PROTOCOL_FRAME_H__

#include "HeaderFiles.h"   

#define FRAME_HEADER0      0xA5
#define FRAME_HEADER1      0xB6
#define FRAME_FOOTER0      0xB6
#define FRAME_FOOTER1      0xA5

#define FRAME_TYPE_CMD     0x01
#define FRAME_TYPE_ACK     0x02
#define FRAME_TYPE_HEART   0x05
#define FRAME_TYPE_ERROR   0xFF

#define CMD_HEARTBEAT      0x8888
#define CMD_BROADCAST      0xFFFF
#define CMD_ERROR_REPLY    0xEEEE
#define CMD_ERROR_0502		 0X0502

/* 协议帧 */
typedef struct __attribute__((packed))
{
    uint16_t header;        //0xA5B6
    uint16_t device_id;     //ID
    uint8_t  frame_type;    //帧类型
    uint16_t cmd;           //命令字
    uint8_t  data_len;      //内容长度
    uint8_t  version;       //版本
    uint8_t  *data;         //内容
} Frame_t;

/* 函数声明 */
void Protocol_SendFrame(uint16_t dev_id, uint8_t type, uint16_t cmd, uint8_t *data, uint8_t len);
void Protocol_SendOK(uint16_t dev_id, uint16_t cmd);
void Protocol_SendError(uint16_t dev_id);
void Protocol_SendError_0502(uint16_t dev_id);

#endif
