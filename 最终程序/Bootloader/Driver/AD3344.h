#ifndef	__AD3344_H
#define __AD3344_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "HeaderFiles.h"

/*****************宏定义*****************/
#define AD3344_PORT				GPIOA
#define AD3344_SCK_PIN		GPIO_PIN_5
#define AD3344_NSS_PIN		GPIO_PIN_4
#define AD3344_MISO_PIN		GPIO_PIN_6
#define AD3344_MOSI_PIN		GPIO_PIN_7

#define  SPI_AD3344_CS_LOW()            gpio_bit_reset(GPIOA, GPIO_PIN_4)
#define  SPI_AD3344_CS_HIGH()           gpio_bit_set(GPIOA, GPIO_PIN_4)

#define 	AMP		20			//前级放大倍数
/*********AD3344配置结构体、枚举定义*********/

/* Register 0x00 (CONVERSION) definition
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |  Bit 15  |  Bit 14  |  Bit 13  |  Bit 12  |  Bit 11  |  Bit 10  |   Bit 9  |   Bit 8  |   Bit 7  |   Bit 6  |   Bit 5  |   Bit 4  |   Bit 3  |   Bit 2  |   Bit 1  |   Bit 0  |
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                    CONV[15:0]                                                                                   |
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */


typedef enum
{
  GD30AD3344_MUX_AIN0_AIN1 = 0,
  GD30AD3344_MUX_AIN0_AIN3 = 1,
  GD30AD3344_MUX_AIN1_AIN3 = 2,
  GD30AD3344_MUX_AIN2_AIN3 = 3,
  GD30AD3344_MUX_AIN0_GND  = 4,
  GD30AD3344_MUX_AIN1_GND  = 5,
  GD30AD3344_MUX_AIN2_GND  = 6,
  GD30AD3344_MUX_AIN3_GND  = 7	
}GD30AD3344_MUX_TypeDef;

typedef enum
{
  GD30AD3344_PGA_6V144 = 0,
  GD30AD3344_PGA_4V096 = 1,
  GD30AD3344_PGA_2V048 = 2,
  GD30AD3344_PGA_1V024 = 3,
  GD30AD3344_PGA_0V512 = 4,
  GD30AD3344_PGA_0V256 = 5,
  GD30AD3344_PGA_0V064 = 6,
  GD30AD3344_PGA_RESERVED = 7
}GD30AD3344_PGA_TypeDef;

typedef enum
{
  GD30AD3344_MODE_CONTINUOUS = 0,
  GD30AD3344_MODE_SINGLE     = 1
}GD30AD3344_MODE_TypeDef;

typedef enum
{
  GD30AD3344_DR_6_25SPS = 0,
  GD30AD3344_DR_12_5SPS = 1,
  GD30AD3344_DR_25SPS   = 2,
  GD30AD3344_DR_50SPS   = 3,
  GD30AD3344_DR_100SPS  = 4,
  GD30AD3344_DR_250SPS  = 5,
  GD30AD3344_DR_500SPS  = 6,
  GD30AD3344_DR_1000SPS = 7
}GD30AD3344_DR_TypeDef;

typedef struct
{
	uint16_t CS;														//bit[15:15]
	GD30AD3344_MUX_TypeDef MUX;							//bit[14:12]
	GD30AD3344_PGA_TypeDef PGA;							//bit[11:9]
	GD30AD3344_MODE_TypeDef MODE;						//bit[8:8]
	GD30AD3344_DR_TypeDef DR;								//bit[7:5]
	uint16_t RESERVED_1;										//bit[4:4]
	uint16_t PULL_UP_EN;										//bit[3:3]
	uint16_t NOP;														//bit[2:1]
	uint16_t RESERVED_0;										//bit[0:0]
}AD3344_struct;
/*****************外部包含*****************/
extern AD3344_struct AD3344_init_struct;
/*****************构建寄存器值*****************/
#define AD3344_InitStruct_Value                                      			 \
    (uint16_t)(                                                            \
        (((uint16_t)AD3344_init_struct.CS         & 0x01U) << 15) |        \
        (((uint16_t)AD3344_init_struct.MUX        & 0x07U) << 12) |        \
        (((uint16_t)AD3344_init_struct.PGA        & 0x07U) << 9)  |        \
        (((uint16_t)AD3344_init_struct.MODE       & 0x01U) << 8)  |        \
        (((uint16_t)AD3344_init_struct.DR         & 0x07U) << 5)  |        \
        (((uint16_t)AD3344_init_struct.RESERVED_1 & 0x01U) << 4)  |        \
        (((uint16_t)AD3344_init_struct.PULL_UP_EN & 0x01U) << 3)  |        \
        (((uint16_t)AD3344_init_struct.NOP        & 0x03U) << 1)  |        \
        (((uint16_t)AD3344_init_struct.RESERVED_0 & 0x01U) << 0)           \
    )
/***************内部函数定义***************/
static uint8_t SPI_Send_Byte(uint8_t Byte);
static uint16_t SPI_Send_Half_Word(uint16_t halfword);
static float PGA_SET(GD30AD3344_PGA_TypeDef PGA);
static float AD3344_read_vol(void);
static float get_midian_num(float *array, uint8_t len);
/*****************函数定义*****************/
void AD3344_Init(void);
void ADC_Config(void);
void AD3344_process(void);
void AD3344_ExtRef(void);
void AD3344_Sample(void);
float AD3344_read_temp(void);

#ifdef __cplusplus
}
#endif

#endif
/*********************************END*************************************/
