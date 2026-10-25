#include "protocol_frame.h"
#include "protocol_crc.h"
#include "protocol_ascii.h"

/**
  * @brief 		发送帧协议
  * @param  	设备ID
  * @param  	帧类型
  * @param  	命令字
  * @param  	内容
  * @param  	报文长度
  * @retval 	
  */
void Protocol_SendFrame(uint16_t dev_id, uint8_t type, uint16_t cmd, uint8_t *data, uint8_t len)
{
  uint8_t bin_buf[256];
  uint8_t ascii_buf[512];
  uint16_t bin_len;
  uint16_t crc;
  uint16_t ascii_len = 0;

  //拼接帧
  bin_buf[0] = FRAME_HEADER0;
  bin_buf[1] = FRAME_HEADER1;
  bin_buf[2] = (dev_id >> 8) & 0xFF;
  bin_buf[3] = dev_id & 0xFF;
  bin_buf[4] = type;
  bin_buf[5] = (cmd >> 8) & 0xFF;
  bin_buf[6] = cmd & 0xFF;
  bin_buf[7] = len;
  bin_buf[8] = 0x02; // 协议版本

  if (len > 0 && data != NULL)
  {
    memcpy(&bin_buf[9], data, len);
  }

  bin_len = 9 + len;

  /* 计算 CRC16 */
  crc = crc16_modbus(bin_buf, bin_len);

  bin_buf[bin_len++] = (crc >> 8) & 0xFF;
  bin_buf[bin_len++] = crc & 0xFF;

  bin_buf[bin_len++] = FRAME_FOOTER0;
  bin_buf[bin_len++] = FRAME_FOOTER1;

  
  for (uint16_t i = 0; i < bin_len; i++)
  {
    byte_to_ascii_hex(bin_buf[i], &ascii_buf[ascii_len]);
    ascii_len += 2;
  }

  USART1_SendData(ascii_buf, ascii_len);
}

/**
  * @brief 		发送OK
  * @param  	ID
  * @param  	指令
  * @retval 	无
  */
void Protocol_SendOK(uint16_t dev_id, uint16_t cmd)
{
  uint8_t ok = 0xFF;
  Protocol_SendFrame(dev_id, FRAME_TYPE_ACK, cmd, &ok, 1);
}

/**
  * @brief 		发送错误应答
  * @param  	ID
  * @retval 	无
  */
void Protocol_SendError(uint16_t dev_id)
{
  Protocol_SendFrame(dev_id, FRAME_TYPE_ERROR, CMD_ERROR_REPLY, NULL, 0);
}

/**
  * @brief 		发送0502应答
  * @param  	ID
  * @retval 	无
  */
void Protocol_SendError_0502(uint16_t dev_id)
{
  Protocol_SendFrame(dev_id, FRAME_TYPE_ERROR, CMD_ERROR_0502, NULL, 0);
}
