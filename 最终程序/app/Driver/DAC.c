#include "DAC.h"

/**
  * @brief 		DAC初始化
  * @param  	无
  * @retval 	无
  */
void DAC_Init(void)
{
	rcu_periph_clock_enable(RCU_DAC);
	rcu_periph_clock_enable(RCU_GPIOA);	//使能时钟
	
	gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4);		//配置为模拟模式
	
	dac_deinit(DAC0);
	dac_trigger_source_config(DAC0, DAC_OUT0, DAC_TRIGGER_SOFTWARE);     //软件触发 
	dac_data_set(DAC0, DAC_OUT0, DAC_ALIGN_12B_R, 0);
	dac_trigger_enable(DAC0, DAC_OUT0);
	dac_wave_mode_config(DAC0, DAC_OUT0, DAC_WAVE_DISABLE);
	dac_output_buffer_disable(DAC0, DAC_OUT0);
	
	dac_enable(DAC0, DAC_OUT0);
}
