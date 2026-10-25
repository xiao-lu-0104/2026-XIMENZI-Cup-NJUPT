#ifndef __PROTOCOL_ASCII_H__
#define __PROTOCOL_ASCII_H__

#include "HeaderFiles.h"

uint8_t ascii_hex_to_byte(uint8_t h, uint8_t l);
void byte_to_ascii_hex(uint8_t byte, uint8_t *out);

#endif
