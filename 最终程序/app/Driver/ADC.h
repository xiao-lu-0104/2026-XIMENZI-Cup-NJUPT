#ifndef __ADC_H
#define __ADC_H


#include "HeaderFiles.h"


void ADC_Init(void);     
uint16_t ADC_Read(void);

void ADC_Multi_channel(void);

void ADC_TIM_DMA_Init(void);
void adc_start(void);
void adc_stop(void);
				    
#endif
