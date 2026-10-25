#include "TIM.h"

void TIM2_Init(uint16_t psc, uint16_t arr)
{
	timer_parameter_struct	timer2_initpara;		//定义定时器结构体
	rcu_periph_clock_enable(RCU_TIMER2);
	rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);	//APB1分频为4，CK_APB倍频为4，频率240M
	
	timer_deinit(TIMER2);
	timer2_initpara.prescaler = psc - 1;
	timer2_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer2_initpara.counterdirection = TIMER_COUNTER_UP;
	timer2_initpara.period = arr - 1;
	
	timer2_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer2_initpara.repetitioncounter = 0;
	
	timer_init(TIMER2,&timer2_initpara);		//初始化定时器

	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);	
	nvic_irq_enable(TIMER2_IRQn,3,2);
	timer_interrupt_enable(TIMER2,TIMER_INT_UP);
	timer_interrupt_flag_clear(TIMER2,TIMER_INT_FLAG_UP);
	
	timer_enable(TIMER2);
}

/**
  * @brief 		定时器5初始化
  * @param  	预分频系数
  * @param  	自动重装值
  * @retval 	无
  */
void TIM5_Init(uint16_t psc, uint16_t arr)
{
	timer_parameter_struct	timer5_initpara;		//定义定时器结构体
	rcu_periph_clock_enable(RCU_TIMER5);
	rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);	//APB1分频为4，CK_APB倍频为4，频率240M
	
	timer_deinit(TIMER5);
	timer5_initpara.prescaler = psc - 1;
	timer5_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer5_initpara.counterdirection = TIMER_COUNTER_UP;
	timer5_initpara.period = arr - 1;
	
	timer5_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer5_initpara.repetitioncounter = 0;
	
	timer_init(TIMER5,&timer5_initpara);		//初始化定时器5
	
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);	
	nvic_irq_enable(TIMER5_DAC_IRQn,2,2);
	timer_interrupt_enable(TIMER5,TIMER_INT_UP);
	timer_interrupt_flag_clear(TIMER5,TIMER_INT_FLAG_UP);
	
	timer_enable(TIMER5);
}

/**
  * @brief 		定时器6初始化
  * @param  	预分频系数
  * @param  	自动重装值
  * @retval 	无
  */
void TIM6_Init(uint16_t psc, uint16_t arr)
{
	timer_parameter_struct	timer_initpara;		//定义定时器结构体
	rcu_periph_clock_enable(RCU_TIMER6);
	rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);	//APB1分频为4，CK_APB倍频为4，频率240M
	
	timer_deinit(TIMER6);
	timer_initpara.prescaler = psc - 1;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = arr - 1;
	
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	
	timer_init(TIMER6,&timer_initpara);		//初始化定时器
	
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);	
	nvic_irq_enable(TIMER6_IRQn,1,2);
	timer_interrupt_enable(TIMER6,TIMER_INT_UP);
	timer_interrupt_flag_clear(TIMER6,TIMER_INT_FLAG_UP);
	
	timer_enable(TIMER6);
}


