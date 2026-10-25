#ifndef __APP_SAMPLE_H__
#define __APP_SAMPLE_H__

#include <stdint.h>

float AppSample_GetCH0(void);
float AppSample_GetCH1(void);
float AppSample_GetCH2(void);
void BspDac_SetValue(uint16_t value);

#endif
