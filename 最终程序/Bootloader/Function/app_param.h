#ifndef __APP_PARAM_H__
#define __APP_PARAM_H__

#include <stdint.h>

#define PARAM_PAGE_ADDR         0x08010000UL
#define PARAM_PAGE_SIZE         4096UL
#define PARAM_COPY_A_ADDR       0x08010200UL
#define PARAM_COPY_B_ADDR       0x08010600UL

#define PARAM_MAGIC             0xA5A5C33CUL

#define BOOT_FLAG_NORMAL        0xFFFFFFFFUL
#define BOOT_FLAG_UPGRADE       0xA55A5AA5UL

#define BAUD_CODE_4800          0x11
#define BAUD_CODE_9600          0x12
#define BAUD_CODE_19200         0x13
#define BAUD_CODE_115200        0x14

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t structSize;

    uint16_t device_id;
    uint8_t  baudrate_code;
    uint8_t  alarm_enable;

    float ch0_ratio;
    float ch1_ratio;

    float ch0_threshold;
    float ch1_threshold;
    float ch2_threshold;

    uint32_t boot_flag;

    uint32_t alarm_write_idx;
    uint32_t alarm_count;

    uint16_t dac_setpoint;
    uint16_t dac_pad;
    uint32_t reserved[6];

    uint32_t crc;
} AppParam_t;

extern AppParam_t g_param;

uint32_t AppParam_CalcCrc(AppParam_t *param);
void AppParam_LoadDefault(AppParam_t *param);
int AppParam_CheckValid(AppParam_t *param);
int AppParam_Load(AppParam_t *param);
int AppParam_Save(AppParam_t *param);

uint32_t AppParam_BaudCodeToValue(uint8_t code);
void AppParam_Init(void);

#endif
