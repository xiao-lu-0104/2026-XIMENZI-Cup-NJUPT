#ifndef __PROTOCOL_CRC_H__
#define __PROTOCOL_CRC_H__

#include "HeaderFiles.h"

uint16_t crc16_modbus(uint8_t *data, uint16_t len);

#endif
