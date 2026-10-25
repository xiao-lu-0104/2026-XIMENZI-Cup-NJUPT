#include "app_sample.h"
#include "app_param.h"
#include "DAC.h"

extern volatile uint16_t S_uAdcData[3];    // CH10=CH0, CH11=CH1, CH12=CH2

/**
  * @brief 		CH0电位器
  * @param  	无
  * @retval 	电压值
  */
float AppSample_GetCH0(void)
{
  float value = S_uAdcData[0] * 3.3f / 4095.0f;
  value = value * g_param.ch0_ratio;
  return value;
}

/**
  * @brief 		CH1 DAC闭环
  * @param  	无
  * @retval 	电压值
  */float AppSample_GetCH1(void)
{
  float value = S_uAdcData[1] * 3.3f / 4095.0f;
  value = value * g_param.ch1_ratio;
  return value;
}

/**
  * @brief 		CH2 PT100
  * @param  	无
  * @retval 	电压值
  */float AppSample_GetCH2(void)
{
  // S_uAdcData[2] 对应 CH12，PT100 温度转换公式根据实际传感器调整
  float voltage = S_uAdcData[2] * 3.3f / 4095.0f;
  // PT100 简化换算（示例，根据实际电路调整）
  float temp = voltage * 100.0f;   // 1V ≈ 100℃ 示例
  return temp;
}

/**
  * @brief 		应用层DAC输出
  * @param  	输出的电压（0~4095，映射到0~3.3）
  * @retval 	无
  */
void BspDac_SetValue(uint16_t value)
{
  if (value > 4095) value = 4095;
  dac_data_set(DAC0, DAC_OUT0, DAC_ALIGN_12B_R, value);
  dac_software_trigger_enable(DAC0, DAC_OUT0);
}
