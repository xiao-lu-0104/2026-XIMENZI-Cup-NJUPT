#ifndef __LED_H
#define __LED_H

#include "HeaderFiles.h"

/*********宏定义*********/
#define LED1_periph	GPIOE
#define LED1_pin		GPIO_PIN_2
#define LED2_periph	GPIOE
#define LED2_pin		GPIO_PIN_3

/*********函数定义*********/
void LED_Init(void);
void LED_Flash(void);
void LED_Toggle(uint32_t gpio_periph, uint32_t pin);
void LED_Reset(uint32_t gpio_periph,uint32_t pin);
void LED_Set(uint32_t gpio_periph,uint32_t pin);

#endif

/*********END*********/
