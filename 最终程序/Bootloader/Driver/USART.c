#include "USART.h"

extern ring_buffer_t usart1_rx_buf;
uint8_t usart1_recv_flag = 0;

/**
  * @brief 		串口1初始化
  * @param  	无
  * @retval 	无
  */
void USART1_Init(uint32_t baudrate)
{
  rcu_periph_clock_enable(RCU_USART1);
  rcu_periph_clock_enable(RCU_GPIOD);
  rcu_periph_clock_enable(RCU_GPIOE);
	
	gpio_af_set(GPIOD, GPIO_AF_7, GPIO_PIN_5 | GPIO_PIN_6);
	//PD5 == TX
	gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
  gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
	//PD6 = RX
	gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
  gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
	//PE8 = CS
  gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
  gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_8);
	gpio_bit_reset(GPIOE, GPIO_PIN_8);
	
	usart_deinit(USART1);
	usart_baudrate_set(USART1, baudrate);
	usart_parity_config(USART1, USART_PM_NONE);
	usart_word_length_set(USART1, USART_WL_8BIT);	//8位数据位
	usart_stop_bit_set(USART1, USART_STB_1BIT);		//1位停止位
	
	usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);	//CTS失能
	usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);	//RTS失能

  usart_receive_config(USART1, USART_RECEIVE_ENABLE);		//接收使能
  usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);	//发送使能
	
	nvic_irq_enable(USART1_IRQn, 3, 2);
	usart_interrupt_enable(USART1, USART_INT_RBNE);	//接收缓冲区非空
	usart_interrupt_enable(USART1, USART_INT_IDLE);	//总线空闲
	usart_enable(USART1);
}

/**
  * @brief 		RS485发送数据函数（自动控制PE8方向引脚）
  * @param  	要发送的数据（uint8_t数组）
  * @param  	数据长度
  * @retval 	无
  */
void USART1_SendData(uint8_t *Buf, uint16_t len)
{
	uint16_t t;

	gpio_bit_set(GPIOE, GPIO_PIN_8);                            // PE8拉高，RS485进入发送模式
	delay_1ms(1);                                               // 等待收发器使能建立

	for(t = 0; t < len; t++)
	{
		while(usart_flag_get(USART1, USART_FLAG_TC) == RESET);  // 等上一个字节发完
		usart_data_transmit(USART1, Buf[t]);                    // 发送当前字节
	}
	while(usart_flag_get(USART1, USART_FLAG_TC) == RESET);      // 等最后一个字节发完

	delay_1ms(1);                                               // 等总线稳定
	gpio_bit_reset(GPIOE, GPIO_PIN_8);                          // PE8拉低，切回接收模式
}

/**
  * @brief 		串口1注册中断回调
  * @param  	无
  * @retval 	无
  */
void USART1_IRQHandler(void)
{
	if (usart_flag_get(USART1 , USART_FLAG_RBNE) == SET)	//溢出检查
	{
		uint8_t byte = usart_data_receive(USART1);
		if (OTA_IsReceiving())
			OTA_ProcessByte(byte);
		else
			ring_buffer_itr(&usart1_rx_buf, byte);
	}

	if(usart_flag_get(USART1, USART_FLAG_IDLE) == SET)
	{
		(void)usart_data_receive(USART1);
	}
}
