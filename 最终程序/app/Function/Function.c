#include "Function.h"
#include "gd32f4xx_rcu.h"
#include "gd32f4xx_rtc.h"
#include "gd32f4xx_pmu.h"
#define RCU_MODIFY_4(__delay)   do{                                     \
                                    volatile uint32_t i, reg;           \
                                    if(0 != __delay){                   \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                        reg = RCU_CFG0;                 \
                                        reg &= ~(RCU_CFG0_AHBPSC);      \
                                        reg |= RCU_AHB_CKSYS_DIV2;      \
                                        /* AHB = SYSCLK/2 */            \
                                        RCU_CFG0 = reg;                 \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                        reg = RCU_CFG0;                 \
                                        reg &= ~(RCU_CFG0_AHBPSC);      \
                                        reg |= RCU_AHB_CKSYS_DIV4;      \
                                        /* AHB = SYSCLK/4 */            \
                                        RCU_CFG0 = reg;                 \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                        reg = RCU_CFG0;                 \
                                        reg &= ~(RCU_CFG0_AHBPSC);      \
                                        reg |= RCU_AHB_CKSYS_DIV8;      \
                                        /* AHB = SYSCLK/8 */            \
                                        RCU_CFG0 = reg;                 \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                        reg = RCU_CFG0;                 \
                                        reg &= ~(RCU_CFG0_AHBPSC);      \
                                        reg |= RCU_AHB_CKSYS_DIV16;     \
                                        /* AHB = SYSCLK/16 */           \
                                        RCU_CFG0 = reg;                 \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                    }                                   \
                                }while(0)

/***************变量声明****************/
uint32_t my_crc32 = 0;									//CRC校验码					
ring_buffer_t usart1_rx_buf;

Eventqueue_t my_event_queue_t;		
Event e;

uint32_t g_rtc_time;
extern uint8_t  g_report_interval;
extern uint8_t  g_auto_report;
static uint8_t  g_auto_running = 0;
static uint32_t g_auto_tick = 0;
																
static uint8_t AD3344_flag = 0;
static uint8_t AD3344_sample_flag = 0;
static uint8_t AD3344_cnt = 0;
float   g_ch2_temp = 0;
																
uint8_t  g_alarm_enable = 0x02;    //0x01=主动上报, 0x02=不主动
uint32_t g_alarm_time = 0;
uint16_t g_alarm_count = 0;
char g_alarm_records[10][64];  //最多10条报警
uint16_t g_alarm_index = 0;
/************变量声明结束*************/

/**********函数声明**********/
static void enter_sleep(void);
uint32_t crc32_calc(uint8_t* data , uint32_t len);
void mcu_software_reset(void);
extern void float_to_be(float value, uint8_t out[4]);
static void _soft_delay_(uint32_t time);
static void rcu_config(void);
static void rtc_alarm_wakeup_10s_config(void);
/*********函数声明结束*********/

void System_Init(void)
{
	//关停
	__disable_irq();
	SCB->VTOR = FLASH_APP_ADDR;
	for (uint32_t i = 0; i < 8; i++)
	{
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}
	__DSB();
	__ISB();
	__enable_irq();
	
	//启动
  systick_config();     // 系统时钟初始化
	AppParam_Init();			//先读参数区
	//设置波特率
  uint32_t baud;
  switch (g_param.baudrate_code)
  {
    case 0x11: baud = 4800;   break;	//0x11 -> 4800
    case 0x12: baud = 9600;   break;	//0x12 -> 9600
    case 0x13: baud = 19200;  break;	//0x13 -> 19200
    case 0x14: baud = 115200; break;	//0x14 -> 115200
    default:   baud = 19200;  break;
  }
	
	//Pheriph_Init;
	USART1_Init(baud);
	LED_Init();
	OLED_Init();
	ADC_Multi_channel();
	DAC_Init();
	BspDac_SetValue(g_param.dac_setpoint);		// 恢复断电前的 DAC 设定值
	ring_buffer_init(&usart1_rx_buf);		//环形队列初始化
	
	AD3344_Init();
	AD3344_process();
	AD3344_ExtRef();
	ADC_Config();
	for (uint8_t k = 0; k < 5; k++) {
		AD3344_Sample();
		delay_1ms(100);
	}
	
	TIM2_Init(10000, 24000);						//f = 1Hz
	TIM6_Init(4800, 10000);						// 50Hz AD3344 采样
	
	OLED_ShowNum(0, 0, 2026409373, 10, 16);
	OLED_ShowString(0, 16, "IDLE", 16);
	OLED_Refresh();
}

void UsrFunction(void)
{   
	Protocol_SendFrame(g_param.device_id, FRAME_TYPE_HEART, CMD_HEARTBEAT, NULL, 0);	//心跳交互标志帧
  while(1)
  {
		if (ring_buffer_get_data_count(&usart1_rx_buf) > 0)
		{
			protocol_frame_parser(&usart1_rx_buf);
		}		

		/* AD3344 PT100 采样处理 */
		if (AD3344_sample_flag == 1)
		{
			AD3344_sample_flag = 0;
			AD3344_Sample();
		}
		if (AD3344_flag == 1)
		{
			AD3344_flag = 0;
			g_ch2_temp = AD3344_read_temp();

			/* 每 30 次读取（~3s）重配置 AD3344，防止寄存器被干扰 */
			static uint8_t reinit_cnt = 0;
			reinit_cnt++;
			if (reinit_cnt >= 30) {
				reinit_cnt = 0;
				ADC_Config();
				AD3344_ExtRef();
			}
		}
		while(event_queue_out(&my_event_queue_t, &e))
		{
			switch(e.type)
			{
				case EVENT_LED_Toggle:		
					LED_Toggle(LED1_periph, LED1_pin);
					break;
				case EVENT_SOFT_Reset:
					mcu_software_reset();
					break;
				case EVENT_AUTO_REPORT_START:
					g_auto_running = 1;
					g_auto_tick = 0;
					OLED_Clear();
					OLED_ShowNum(0, 0, 2026409373, 10, 16);					
					OLED_ShowString(0, 16, "AutoSample", 16);
					OLED_Refresh();
					LED_Set(LED2_periph, LED2_pin);		//亮灯
					break;
				case EVENT_AUTO_REPORT_STOP:
					g_auto_running = 0;
					OLED_Clear();
					OLED_ShowNum(0, 0, 2026409373, 10, 16);					
					OLED_ShowString(0, 16, "IDLE", 16);
					OLED_Refresh();
					LED_Reset(LED2_periph, LED2_pin);		//亮灯
					break;
				case EVENT_AUTO_REPORT_SEND:
				{
					uint8_t d[12];
					d[0] = (g_rtc_time >> 24) & 0xFF;
					d[1] = (g_rtc_time >> 16) & 0xFF;
					d[2] = (g_rtc_time >> 8)  & 0xFF;
					d[3] = g_rtc_time & 0xFF;
					float_to_be(AppSample_GetCH0(), &d[4]);
					float_to_be(AppSample_GetCH1(), &d[8]);
					Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, 0x0302, d, 12);
					break;
				}
				case EVENT_ALARM_CHECK:
				{
					float ch0 = AppSample_GetCH0();
					float ch1 = AppSample_GetCH1();

					if (ch0 > g_param.ch0_threshold)
					{
						uint32_t t = g_rtc_time;
						int len = sprintf(g_alarm_records[g_alarm_index % 10],
										"%04u-%02u-%02u %02u:%02u:%02u | CH0 | %.2f | %.2f\r\n",
										1970 + t/31536000, (t%31536000)/2592000+1, (t%2592000)/86400+1,
										(t%86400)/3600+8, (t%3600)/60, t%60,
										g_param.ch0_threshold, ch0);

						if (g_param.alarm_enable == 0x01)
						{
								USART1_SendData((uint8_t *)g_alarm_records[g_alarm_index % 10], len);
						}

						g_alarm_index++;
						if (g_alarm_count < 10) g_alarm_count++;
					}

					if (ch1 > g_param.ch1_threshold)
					{
						uint32_t t = g_rtc_time;
						int len = sprintf(g_alarm_records[g_alarm_index % 10],
										"%04u-%02u-%02u %02u:%02u:%02u | CH1 | %.2f | %.2f\r\n",
										1970 + t/31536000, (t%31536000)/2592000+1, (t%2592000)/86400+1,
										(t%86400)/3600+8, (t%3600)/60, t%60,
										g_param.ch1_threshold, ch1);

						if (g_param.alarm_enable == 0x01)
						{
								USART1_SendData((uint8_t *)g_alarm_records[g_alarm_index % 10], len);
						}

						g_alarm_index++;
						if (g_alarm_count < 10) g_alarm_count++;
					}

					/* CH2 PT100 温度告警 */
					{
						float ch2 = AppSample_GetCH2();
						if (ch2 > g_param.ch2_threshold)
						{
							uint32_t t = g_rtc_time;
							int len = sprintf(g_alarm_records[g_alarm_index % 10],
											"%04u-%02u-%02u %02u:%02u:%02u | CH2 | %.2f | %.2f\r\n",
											1970 + t/31536000, (t%31536000)/2592000+1, (t%2592000)/86400+1,
											(t%86400)/3600+8, (t%3600)/60, t%60,
											g_param.ch2_threshold, ch2);

							if (g_param.alarm_enable == 0x01)
							{
									USART1_SendData((uint8_t *)g_alarm_records[g_alarm_index % 10], len);
							}

							g_alarm_index++;
							if (g_alarm_count < 10) g_alarm_count++;
						}
					}
					break;
				}
				case EVENT_ALARM_MSG:
				{
						if (g_alarm_count == 0)
						{
								USART1_SendData((uint8_t *)"empty\r\n", 7);
						}
						else
						{
								for (int i = 0; i < g_alarm_count; i++)
								{
										int idx = (g_alarm_index - 1 - i + 10) % 10;
										USART1_SendData((uint8_t *)g_alarm_records[idx],
																		strlen(g_alarm_records[idx]));
										delay_1ms(10);
								}
						}
						break;
				}
				case EVENT_ENTER_SLEEP:
				{
					enter_sleep();
					break;
				}
				default:
					break;
			}
		}
  }
}

/**
  * @brief 		CRC32校验，遍历每个字节做8次移位异或
  * @param  	数据
  * @param  	长度
  * @retval 	返回校验值
  */
uint32_t crc32_calc(uint8_t* data , uint32_t len)
{
	uint32_t crc = 0xFFFFFFFF;
	uint32_t i , j;

	for (i = 0; i < len; i++)
	{
		crc ^= data[i];
		for (j = 0; j < 8; j++)
		{
			if (crc & 1)
				crc = (crc >> 1) ^ 0xEDB88320;
			else
				crc >>= 1;
		}
	}

	return crc ^ 0xFFFFFFFF;
}

/**
	* @brief 		软件复位
  * @param  	无
  * @retval 	无
  */
void mcu_software_reset(void)
{
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}

static void enter_sleep(void)
{
    /*
       1. 确保 OK 应答已经发完
       你的协议层收到 0x03AA 时应该先 Protocol_SendOK(0x03AA);
       然后再投递 EVENT_ENTER_SLEEP。
    */
    delay_1ms(50);

    /* 2. 关闭部分外设 */
    usart_disable(USART1);

    timer_disable(TIMER2);

    dma_channel_disable(DMA1, DMA_CH0);

    adc_disable(ADC0);
    dac_disable(DAC0, DAC_OUT0);

    /*
       3. 配置 RTC Alarm 10 秒唤醒
       注意：必须在进入 deepsleep 前完成
    */
    rtc_alarm_wakeup_10s_config();

    /*
       4. 关闭 SysTick，避免 SysTick 干扰低功耗
    */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /*
       5. 清除可能残留的 pending
    */
    exti_interrupt_flag_clear(EXTI_17);
    rtc_flag_clear(RTC_FLAG_ALRM0);

    __DSB();
    __ISB();

    /*
       6. 进入深睡
       RTC Alarm 到来后通过 EXTI_17 唤醒
    */
    pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, PMU_LOWDRIVER_ENABLE, WFI_CMD);

    /*
       7. 从这里继续执行，说明已经被唤醒
    */
    rcu_config();
    systick_config();

    /*
       8. 恢复外设
       注意：这里最好使用当前参数波特率，而不是固定 19200。
       但 J 测试在 M 改波特率前，固定 19200 通常也能过。
    */
    uint32_t baud;

    switch (g_param.baudrate_code)
    {
        case 0x11: baud = 4800;   break;
        case 0x12: baud = 9600;   break;
        case 0x13: baud = 19200;  break;
        case 0x14: baud = 115200; break;
        default:   baud = 19200;  break;
    }

    USART1_Init(baud);

    LED_Init();
    OLED_Init();
    ADC_Multi_channel();
    DAC_Init();
    BspDac_SetValue(g_param.dac_setpoint);		// 恢复断电前的 DAC 设定值

    ring_buffer_init(&usart1_rx_buf);

    TIM2_Init(10000, 24000);

    OLED_Clear();
    OLED_ShowNum(0, 0, 2026409373, 10, 16);
    OLED_ShowString(0, 16, "IDLE", 16);
    OLED_Refresh();

    /*
       9. 唤醒字符串，非协议帧
    */
    USART1_SendData((uint8_t *)"instrument wakeup\r\n", 19);
}

static void rtc_alarm_wakeup_10s_config(void)
{
    rtc_parameter_struct rtc_initpara;
    rtc_alarm_struct rtc_alarm;

    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();

    rcu_bkp_reset_enable();
    rcu_bkp_reset_disable();

    rcu_osci_on(RCU_IRC32K);
    while (SUCCESS != rcu_osci_stab_wait(RCU_IRC32K))
    {
    }

    rcu_rtc_clock_config(RCU_RTCSRC_IRC32K);
    rcu_periph_clock_enable(RCU_RTC);

    //1Hz
    rtc_initpara.factor_asyn = 0x7F;
    rtc_initpara.factor_syn  = 0xF9;

    rtc_initpara.year        = 0x26;
    rtc_initpara.month       = RTC_JAN;
    rtc_initpara.date        = 0x01;
    rtc_initpara.day_of_week = RTC_THURSDAY;
    rtc_initpara.display_format = RTC_24HOUR;
    rtc_initpara.am_pm       = RTC_AM;
    rtc_initpara.hour        = 0x00;
    rtc_initpara.minute      = 0x00;
    rtc_initpara.second      = 0x00;

    rtc_init(&rtc_initpara);

    rtc_alarm_disable(RTC_ALARM0);
    rtc_interrupt_disable(RTC_INT_ALARM0);

    rtc_flag_clear(RTC_FLAG_ALRM0);
    exti_interrupt_flag_clear(EXTI_17);

    rtc_alarm.alarm_mask = RTC_ALARM_DATE_MASK |
                           RTC_ALARM_HOUR_MASK |
                           RTC_ALARM_MINUTE_MASK;

    rtc_alarm.weekday_or_date = RTC_ALARM_DATE_SELECTED;
    rtc_alarm.alarm_day       = 0x01;
    rtc_alarm.am_pm           = RTC_AM;
    rtc_alarm.alarm_hour      = 0x00;
    rtc_alarm.alarm_minute    = 0x00;
    rtc_alarm.alarm_second    = 0x10;   // 注意：BCD 10 秒，不是十进制 10

    rtc_alarm_config(RTC_ALARM0, &rtc_alarm);

    exti_interrupt_flag_clear(EXTI_17);
    exti_init(EXTI_17, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_enable(EXTI_17);

    rtc_interrupt_enable(RTC_INT_ALARM0);
    rtc_alarm_enable(RTC_ALARM0);

    nvic_irq_enable(RTC_Alarm_IRQn, 0, 0);
}

static void _soft_delay_(uint32_t time)
{
	__IO uint32_t i;
	for (i = 0; i < time * 10; i++)
	{
	}
}

static void rcu_config(void)
{
	uint32_t timeout = 0U;
	uint32_t stab_flag = 0U;

	//!时钟切换demo里面的
	/* It is strongly recommended to include it to avoid issues caused by self-removal. */
	RCU_MODIFY_4(0x50);
	/* select HXTAL as system clock source, deinitialize the RCU */
	rcu_system_clock_source_config(RCU_CKSYSSRC_HXTAL);
	/* It is strongly recommended to include it to avoid issues caused by self-removal. */
	_soft_delay_(200);
	rcu_deinit();

	/* enable HXTAL */
	RCU_CTL |= RCU_CTL_HXTALEN;

	/* wait until HXTAL is stable or the startup time is longer than HXTAL_STARTUP_TIMEOUT */
	do
	{
		timeout++;
		stab_flag = (RCU_CTL & RCU_CTL_HXTALSTB);
	} while ((0U == stab_flag) && (HXTAL_STARTUP_TIMEOUT != timeout));

	/* if fail */
	if (0U == (RCU_CTL & RCU_CTL_HXTALSTB))
	{
		while (0U == (RCU_CTL & RCU_CTL_HXTALSTB))
		{
		}
	}

	RCU_APB1EN |= RCU_APB1EN_PMUEN;
	PMU_CTL |= PMU_CTL_LDOVS;

	/* HXTAL is stable */
	/* AHB = SYSCLK */
	RCU_CFG0 |= RCU_AHB_CKSYS_DIV1;
	/* APB2 = AHB/2 */
	RCU_CFG0 |= RCU_APB2_CKAHB_DIV2;
	/* APB1 = AHB/4 */
	RCU_CFG0 |= RCU_APB1_CKAHB_DIV4;

	/* Configure the main PLL, PSC = 25, PLL_N = 480, PLL_P = 2, PLL_Q = 10 */
	RCU_PLL = (25U | (480U << 6U) | (((2U >> 1U) - 1U) << 16U) |
		(RCU_PLLSRC_HXTAL) | (10U << 24U));

	/* enable PLL */
	RCU_CTL |= RCU_CTL_PLLEN;

	/* wait until PLL is stable */
	while (0U == (RCU_CTL & RCU_CTL_PLLSTB))
	{
	}

	/* Enable the high-drive to extend the clock frequency to 240 Mhz */
	PMU_CTL |= PMU_CTL_HDEN;
	while (0U == (PMU_CS & PMU_CS_HDRF))
	{
	}

	/* select the high-drive mode */
	PMU_CTL |= PMU_CTL_HDS;
	while (0U == (PMU_CS & PMU_CS_HDSRF))
	{
	}

	/* select PLL as system clock */
	RCU_CFG0 &= ~RCU_CFG0_SCS;
	RCU_CFG0 |= RCU_CKSYSSRC_PLLP;

	/* wait until PLL is selected as system clock */
	while (0U == (RCU_CFG0 & RCU_SCSS_PLLP))
	{
	}
}

/**
  * @brief 		LED1s持续翻转 定时器注册中断回调
  * @param  	无
  * @retval 	无
  */
void TIMER2_IRQHandler(void)
{
	if(timer_interrupt_flag_get(TIMER2,TIMER_INT_FLAG_UP) == SET)
	{
		g_rtc_time++;
		timer_interrupt_flag_clear(TIMER2,TIMER_INT_FLAG_UP);
		Event local_e1;
		local_e1.type = EVENT_LED_Toggle;
		local_e1.param1 = 0;
		local_e1.param2 = 0;
		event_queue_enter(&my_event_queue_t, local_e1);
		
		g_alarm_time++;
		if(g_alarm_time >= 2)
		{
			g_alarm_time = 0;
			Event local_e3;
			local_e3.type = EVENT_ALARM_CHECK;
			local_e3.param1 = 0;
			local_e3.param2 = 0;
			event_queue_enter(&my_event_queue_t, local_e3);				
		}

		
		if(g_auto_running)
		{
			g_auto_tick++;
			
      uint32_t interval_s = 1;
			switch(g_report_interval)
			{
				case 1: interval_s = 1; break;
				case 2: interval_s = 3; break;
				case 3: interval_s = 5; break;
			}
			
			if(g_auto_tick >= interval_s)
			{
			  g_auto_tick = 0;
				
				Event local_e2;
				local_e2.type = EVENT_AUTO_REPORT_SEND;
				local_e2.param1 = 0;
				local_e2.param2 = 0;
				event_queue_enter(&my_event_queue_t, local_e2);				
			}
		}
	}
}

void TIMER6_IRQHandler(void)
{
	if(timer_interrupt_flag_get(TIMER6,TIMER_INT_FLAG_UP) == SET)
	{
		timer_interrupt_flag_clear(TIMER6,TIMER_INT_FLAG_UP);
		AD3344_sample_flag = 1;
		AD3344_cnt++;
		if(AD3344_cnt >= 5 && AD3344_flag == 0)	// 10Hz 读取温度
		{
			AD3344_flag = 1;
		}
		AD3344_cnt %= 5;
	}
}

void RTC_Alarm_IRQHandler(void)
{
    if (rtc_flag_get(RTC_FLAG_ALRM0) != RESET)
    {
        rtc_flag_clear(RTC_FLAG_ALRM0);
        rtc_alarm_disable(RTC_ALARM0);
    }

    exti_interrupt_flag_clear(EXTI_17);
}
