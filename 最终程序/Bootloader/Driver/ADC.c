#include "ADC.h"

volatile uint16_t S_uAdcData[3] = {0};
volatile uint16_t ad_buffer[1024];

//软件触发采样连续
void ADC_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_ADC0);
    adc_clock_config(ADC_ADCCK_PCLK2_DIV8);
    adc_deinit();
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0);
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 1);
    // ADC0 规则通道 0 配置为 ADC_CHANNEL_10
    adc_routine_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_56);
    adc_external_trigger_config(ADC0, ADC_ROUTINE_CHANNEL, DISABLE);
    adc_enable(ADC0);
    delay_1ms(1);
    // ADC 校准
    adc_calibration_enable(ADC0);
    // 软件触发一次，之后连续转换
    adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);
}

uint16_t ADC_Read(void)
{
    uint16_t value;
    while(!adc_flag_get(ADC0, ADC_FLAG_EOC));
    value = adc_routine_data_read(ADC0);
    adc_flag_clear(ADC0, ADC_FLAG_EOC);
    return value;
}

//软件触发采样多通道连续转换,三通道
void ADC_Multi_channel(void)
{
    dma_single_data_parameter_struct dma_InitStructure;
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_DMA1);
    adc_clock_config(ADC_ADCCK_PCLK2_DIV8);
    adc_deinit();
    dma_deinit(DMA1, DMA_CH0);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_2);

    dma_single_data_para_struct_init(&dma_InitStructure);
    dma_InitStructure.direction = DMA_PERIPH_TO_MEMORY;
    dma_InitStructure.memory0_addr = (uint32_t)S_uAdcData;
    dma_InitStructure.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_InitStructure.number = 3;
    dma_InitStructure.periph_addr = (uint32_t)&ADC_RDATA(ADC0);
    dma_InitStructure.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_InitStructure.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;
    dma_InitStructure.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(DMA1, DMA_CH0, &dma_InitStructure);
    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);
    dma_circulation_enable(DMA1, DMA_CH0);

    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 3);
    adc_routine_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_56);
    adc_routine_channel_config(ADC0, 1, ADC_CHANNEL_11, ADC_SAMPLETIME_56);
    adc_routine_channel_config(ADC0, 2, ADC_CHANNEL_12, ADC_SAMPLETIME_56);
    adc_external_trigger_config(ADC0, ADC_ROUTINE_CHANNEL, DISABLE);

    adc_dma_mode_enable(ADC0);
    adc_dma_request_after_last_enable(ADC0);
    dma_channel_enable(DMA1, DMA_CH0);
    adc_enable(ADC0);
    delay_1ms(1);
    adc_calibration_enable(ADC0);
    adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);
}

//定时器\DMA触发采样
void ADC_TIM_DMA_Init(void)
{
    dma_single_data_parameter_struct dma_InitStructure;
    timer_parameter_struct tim_InitStructure;
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_DMA1);
    rcu_periph_clock_enable(RCU_TIMER2);

    adc_clock_config(ADC_ADCCK_PCLK2_DIV8);
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    adc_deinit();
    dma_deinit(DMA1, DMA_CH0);
    timer_deinit(TIMER2);

    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0);

    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);
    adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);

    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 1);
    adc_routine_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_56);

    adc_external_trigger_source_config(ADC0, ADC_ROUTINE_CHANNEL, ADC_EXTTRIG_ROUTINE_T2_TRGO);
    adc_external_trigger_config(ADC0, ADC_ROUTINE_CHANNEL, ENABLE);

    adc_dma_mode_enable(ADC0);
    adc_dma_request_after_last_enable(ADC0);

    adc_enable(ADC0);
    delay_1ms(1);
    adc_calibration_enable(ADC0);

    dma_single_data_para_struct_init(&dma_InitStructure);

    dma_InitStructure.direction = DMA_PERIPH_TO_MEMORY;              // 外设到内存
    dma_InitStructure.memory0_addr = (uint32_t)ad_buffer;
    dma_InitStructure.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_InitStructure.number = 1024;
    dma_InitStructure.periph_addr = (uint32_t)&ADC_RDATA(ADC0);
    dma_InitStructure.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_InitStructure.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;
    dma_InitStructure.priority = DMA_PRIORITY_ULTRA_HIGH;

    dma_single_data_mode_init(DMA1, DMA_CH0, &dma_InitStructure);

    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);
    dma_circulation_disable(DMA1, DMA_CH0);

    timer_struct_para_init(&tim_InitStructure);

    tim_InitStructure.prescaler = 0;
    tim_InitStructure.period = 2400 - 1;

    timer_init(TIMER2, &tim_InitStructure);

    timer_master_output_trigger_source_select(TIMER2, TIMER_TRI_OUT_SRC_UPDATE);
    timer_update_event_enable(TIMER2);
}

void adc_start(void)
{
    timer_disable(TIMER2);
    dma_channel_disable(DMA1, DMA_CH0);

    dma_flag_clear(DMA1, DMA_CH0, DMA_FLAG_FTF);
    dma_transfer_number_config(DMA1, DMA_CH0, 1024);

    dma_channel_enable(DMA1, DMA_CH0);
    timer_enable(TIMER2);
}

void adc_stop(void)
{
    timer_disable(TIMER2);
    dma_channel_disable(DMA1, DMA_CH0);
}
