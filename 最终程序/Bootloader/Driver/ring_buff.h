#ifndef __RING_BUFF_H
#define __RING_BUFF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define USART_RX_BUFF_LEN	512

typedef struct
{
	uint8_t buffer[USART_RX_BUFF_LEN];
	volatile uint16_t head;
	volatile uint16_t tail;
	volatile uint16_t datacount;
} ring_buffer_t;

void ring_buffer_init(ring_buffer_t *rb);
bool ring_buffer_is_empty(ring_buffer_t *rb);
uint16_t ring_buffer_get_data_count(ring_buffer_t *rb);
bool ring_buffer_itr(ring_buffer_t *rb, uint8_t byte);
uint16_t ring_buffer_read_safe(ring_buffer_t *rb, uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif
