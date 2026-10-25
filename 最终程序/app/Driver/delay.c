#include "delay.h"

void delay_us(uint32_t xus)
{
	SysTick->LOAD = 73 * xus;				
	SysTick->VAL = 0x00;					
	SysTick->CTRL = 0x00000005;				
	while(!(SysTick->CTRL & 0x00010000));	
	SysTick->CTRL = 0x00000004;				
}

void systick_start(void){
	SysTick->LOAD = 0x00FFFFFF;				
	SysTick->VAL = 0x00;					
	SysTick->CTRL = 0x00000005;
}

void systick_stop(void){
	SysTick->CTRL = 0x00000004;	
}

uint32_t get_tick(void){
	uint32_t tick = 0x00FFFFFF - (SysTick->VAL);
	return tick;
}

void delay_ms(uint32_t xms){
	while(xms--){
		delay_us(1000);
	}
}


void delay_s(uint32_t xs){
	while(xs--){
		delay_ms(1000);
	}
}
