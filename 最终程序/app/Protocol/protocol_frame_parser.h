#ifndef __PROTOCOL_FRAME_PARSER_H__
#define __PROTOCOL_FRAME_PARSER_H__

#include <stdint.h>
#include "ring_buff.h"

void protocol_frame_parser(ring_buffer_t *buf);

#endif
