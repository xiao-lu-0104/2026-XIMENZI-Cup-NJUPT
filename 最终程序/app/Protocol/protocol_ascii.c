#include "protocol_ascii.h"

/**
  * @brief 		ASCII转数值
  * @param  	ASCII字
  * @retval 	返回的数值
	* @note			内部用
  */
static uint8_t hex_char_to_val(uint8_t c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

/**
  * @brief 		数值转ASCII
  * @param  	返回的数值
  * @retval 	ASCII字
	* @note			内部用
  */
static uint8_t val_to_hex_char(uint8_t v)
{
  if (v <= 9) return '0' + v;
  return 'A' + v - 10;
}

/**
  * @brief 		HEX转字
  * @param  	高位
  * @param  	低位
  * @retval 	字
  */
uint8_t ascii_hex_to_byte(uint8_t h, uint8_t l)
{
  return (hex_char_to_val(h) << 4) | hex_char_to_val(l);
}

/**
  * @brief 		字HEX
  * @param  	字
  * @param  	转出的字的地址
  * @retval 	无
  */
void byte_to_ascii_hex(uint8_t byte, uint8_t *out)
{
  out[0] = val_to_hex_char((byte >> 4) & 0x0F);
  out[1] = val_to_hex_char(byte & 0x0F);
}
