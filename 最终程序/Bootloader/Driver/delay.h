#ifndef __DELAY_H__
#define __DELAY_H__

#include "HeaderFiles.h"

void delay_us(uint32_t xus);
void delay_ms(uint32_t xms);
void delay_s(uint32_t xs);
void systick_start(void);
void systick_stop(void);
uint32_t get_tick(void);

#endif
