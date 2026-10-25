#include "AD3344.h"

#define LEN	5
//==================变量==================//
static float s_vol[LEN] = {0};
AD3344_struct AD3344_init_struct;
//=================工具层=================//
/**
	* @brief 		SPI发送字节，服务SPI发送半字
  * @param  	要发送的字节
  * @retval 	返回的字节
  */
static uint8_t SPI_Send_Byte(uint8_t Byte)
{
  while (!spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
  spi_i2s_data_transmit(SPI0, Byte);

  while (!spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
  return (uint8_t)spi_i2s_data_receive(SPI0);
}
/**
  * @brief 		SPI发送半字
  * @param  	需要发送的半字
  * @retval 	返回的半字
  */
static uint16_t SPI_Send_Half_Word(uint16_t halfword)
{
	uint8_t rx_h = SPI_Send_Byte((halfword >> 8) & 0xFF);
	uint8_t rx_l = SPI_Send_Byte(halfword & 0xFF);
	
  return ((uint16_t)rx_h << 8) | rx_l;
}

/**
  * @brief 		量程系数计算
  * @param  	PGA模式
  * @retval 	返回的量程
  */
static float PGA_SET(GD30AD3344_PGA_TypeDef PGA)
{
	switch(PGA)
	{
		case GD30AD3344_PGA_6V144:
			return 6.144f;
		case GD30AD3344_PGA_4V096:
			return 4.096f;
		case GD30AD3344_PGA_2V048:
			return 2.048f;
		case GD30AD3344_PGA_1V024:
			return 1.024f;		
		case GD30AD3344_PGA_0V512:
			return 0.512f;
		case GD30AD3344_PGA_0V256:
			return 0.256f;
		case GD30AD3344_PGA_0V064:
			return 0.064f;
		default:
			return 2.048f;			
	}
}

/**
  * @brief 		获取中位数算法
  * @param  	需要获取中位数的数组
  * @retval 	中位数
  */
static float get_midian_num(float *array, uint8_t len)
{
	float vol[LEN];
	float tmp;
	uint8_t i,j;
	if(len == 0)
	{
		return 0.0f;
	}
	//复制数组
  for (i = 0; i < len; i++) 
	{
      vol[i] = array[i];
  }	
	
	for(i = 0; i < len - 1; i++)
	{
    for (j = i + 1; j < len; j++) 
		{
			if (vol[i] > vol[j]) 
			{
        tmp = vol[i];
        vol[i] = vol[j];
        vol[j] = tmp;
      }
    }
	}
	
	return vol[len / 2];
}
//=================配置层=================//
/**
  * @brief 		AD3344工作模式配置
  * @param  	无
  * @retval 	无
  */
void ADC_Config(void)
{
	AD3344_init_struct.CS = 0;
	AD3344_init_struct.MUX = GD30AD3344_MUX_AIN0_GND;
	AD3344_init_struct.DR = GD30AD3344_DR_12_5SPS;
	AD3344_init_struct.MODE = GD30AD3344_MODE_CONTINUOUS;
	AD3344_init_struct.NOP = 1;
	AD3344_init_struct.PGA = GD30AD3344_PGA_4V096;
	AD3344_init_struct.PULL_UP_EN = 1;
	AD3344_init_struct.RESERVED_0 = 1;
	AD3344_init_struct.RESERVED_1 = 0;
	
	SPI_AD3344_CS_LOW();
	delay_1ms(1);
	(void)SPI_Send_Half_Word(AD3344_InitStruct_Value);
	SPI_AD3344_CS_HIGH();
	delay_1ms(1);
}

/**
  * @brief 		AD3344初始化
  * @param  	无
  * @retval 	无
  */
void AD3344_Init(void)
{
	/*GPIO配置*/
	rcu_periph_clock_enable(RCU_SPI0);
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_af_set(AD3344_PORT, GPIO_AF_5, AD3344_SCK_PIN | AD3344_MISO_PIN | AD3344_MOSI_PIN);
	//CS->PA4
  gpio_mode_set(AD3344_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, AD3344_NSS_PIN);
  gpio_output_options_set(AD3344_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, AD3344_NSS_PIN);	
	//SCL-> PA5, MISO-> PA6, MOSI-> PA7
	gpio_mode_set(AD3344_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, AD3344_SCK_PIN | AD3344_MISO_PIN | AD3344_MOSI_PIN);
  gpio_output_options_set(AD3344_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, AD3344_SCK_PIN | AD3344_MISO_PIN | AD3344_MOSI_PIN);
	/*SPI0配置*/
	spi_parameter_struct spi_ad3344_struct;
	spi_struct_para_init(&spi_ad3344_struct);
	spi_ad3344_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
	spi_ad3344_struct.device_mode = SPI_MASTER;
	spi_ad3344_struct.frame_size = SPI_FRAMESIZE_8BIT;
	spi_ad3344_struct.nss = SPI_NSS_SOFT;
	spi_ad3344_struct.prescale = SPI_PSC_64;
	spi_ad3344_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
	spi_ad3344_struct.endian = SPI_ENDIAN_MSB;
  spi_init(SPI0, &spi_ad3344_struct);
	/*使能SPI*/
	spi_enable(SPI0);
}

/**
  * @brief 		解锁步骤
  * @param  	无
  * @retval 	无
  */
void AD3344_process(void)
{
  uint16_t addr,val;
  uint16_t tx_data;
  
  addr = 0x10 + 0x02;
  val = 0xACCA;
  
  SPI_AD3344_CS_LOW();
  delay_1ms(1);
  
  tx_data = 0x8100;
  SPI_Send_Half_Word(tx_data);
  
  tx_data = addr;
  SPI_Send_Half_Word(tx_data);
  
  tx_data = val;
  SPI_Send_Half_Word(tx_data);
  delay_1ms(1);
  
  SPI_AD3344_CS_HIGH();
  delay_1ms(1);
}

/**
	* @brief 		设置AIN3为外部基准源
  * @param  	无
  * @retval 	无
  */
void AD3344_ExtRef(void)
{
   uint16_t addr,val,rdval;
   uint16_t tx_data;
   
   addr = 0x10 + 0x4;
   
   SPI_AD3344_CS_LOW();
   delay_1ms(1);
   
   tx_data = 0x8106;
   SPI_Send_Half_Word(tx_data);
   delay_1ms(1);
   
   tx_data = addr;
   SPI_Send_Half_Word(tx_data);
   delay_1ms(1);
   
   rdval = SPI_Send_Half_Word(0x00);
   delay_1ms(1);
   
   SPI_AD3344_CS_HIGH();
   delay_1ms(1);
   
   val = rdval | 0x40;
   
   SPI_AD3344_CS_LOW();
   delay_1ms(1);
   
   tx_data = 0x8100;
   SPI_Send_Half_Word(tx_data);
   
   tx_data = addr;
   SPI_Send_Half_Word(tx_data);
   
   tx_data = val;
   SPI_Send_Half_Word(tx_data);
   delay_1ms(1);
   
   SPI_AD3344_CS_HIGH();
   delay_1ms(1);
}
//=================驱动层=================//
/**
  * @brief 		AD3344读取电压
  * @param  	无
  * @retval 	返回的电压
  */
static float AD3344_read_vol(void)
{
	uint16_t raw_data;
	GD30AD3344_PGA_TypeDef PGA = AD3344_init_struct.PGA;
	SPI_AD3344_CS_LOW();
	delay_1ms(1);	
	raw_data = SPI_Send_Half_Word(AD3344_InitStruct_Value);
	SPI_AD3344_CS_HIGH();
	delay_1ms(1);
	
	return raw_data * PGA_SET(PGA) / 32768.0f;
}

/**
  * @brief 		采样函数，频率10Hz
  * @param  	无
  * @retval 	无
  */
void AD3344_Sample(void)
{
	static uint8_t count = 0;		//静态循环计数器
	float vol_t = 0;
	vol_t = AD3344_read_vol();
	s_vol[count] = vol_t;
	count = (count + 1) % LEN;
}
//=================API=================//
/**
  * @brief 		AD3344转换温度
  * @param  	无
  * @retval 	温度
  */
float AD3344_read_temp(void)
{
	float vol = 0;
	float temp = 0;
	float R = 0;
	
	vol = get_midian_num(s_vol, LEN);
	
	R = vol / (AMP * 0.001);		//R = 电压 / 前级放大倍数 / 1mA
	temp = (R - 100.0f) / 0.385055f;
	
	return temp;
}

/*********************************END*************************************/
