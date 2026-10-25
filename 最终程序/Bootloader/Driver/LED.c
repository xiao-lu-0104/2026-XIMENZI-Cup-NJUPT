#include "LED.h"

/**
  * @brief 		LED初始化
  * @param  	无
  * @retval 	无
  */
void LED_Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOE);
	gpio_mode_set(GPIOE,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,GPIO_PIN_2);
	gpio_output_options_set(GPIOE,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_2);
	gpio_bit_reset(GPIOE,GPIO_PIN_2);
	
	gpio_mode_set(GPIOE,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,GPIO_PIN_3);
	gpio_output_options_set(GPIOE,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_3);
	gpio_bit_reset(GPIOE,GPIO_PIN_3);
}

/**
  * @brief 		LED翻转函数（可以加static）
  * @param  	LED端口
  * @param  	LED引脚
  * @retval 	无
  */
void LED_Toggle(uint32_t gpio_periph, uint32_t pin)
{
	GPIO_TG(gpio_periph) = pin;			//等价于gpio_bit_toggle(gpio_periph, pin);
}

/**
  * @brief 		LED闪烁函数
  * @param  	无
  * @retval 	无
  */
void LED_Flash(void)
{
	gpio_bit_set(LED1_periph,LED1_pin);
	gpio_bit_reset(LED2_periph,LED2_pin);
	delay_1ms(500);
	LED_Toggle(LED1_periph,LED1_pin);
	LED_Toggle(LED2_periph,LED2_pin);
	delay_1ms(500);
}

/**
  * @brief 		LED亮灯函数
  * @param  	LED端口
  * @param  	LED引脚
  * @retval 	无
  */
void LED_Set(uint32_t gpio_periph,uint32_t pin)
{
	gpio_bit_set(gpio_periph,pin);
}

/**
  * @brief 		LED灭灯函数
  * @param  	LED端口
  * @param  	LED引脚
  * @retval 	无
  */
void LED_Reset(uint32_t gpio_periph,uint32_t pin)
{
	gpio_bit_reset(gpio_periph,pin);
}
