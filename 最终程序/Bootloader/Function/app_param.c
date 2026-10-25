#include "app_param.h"
#include "rom.h"
#include <string.h>

AppParam_t g_param;

extern uint32_t crc32_calc(uint8_t *data, uint32_t len);

/**
  * @brief      计算AppParam的CRC32
  * @param      参数结构体指针
  * @retval     CRC32加盐值
  */
uint32_t AppParam_CalcCrc(AppParam_t *param)
{
    return crc32_calc((uint8_t *)param, sizeof(AppParam_t) - 4);
}

/**
  * @brief      加载默认参数
  * @param      参数结构体指针
  * @retval     无
  */
void AppParam_LoadDefault(AppParam_t *param)
{
    memset(param, 0, sizeof(AppParam_t));

    param->magic         = PARAM_MAGIC;
    param->version       = 0x0001;
    param->structSize    = sizeof(AppParam_t);

    param->device_id     = 0x0001;
    param->baudrate_code = 0x13;

    param->alarm_enable  = 0x02;

    param->ch0_ratio     = 1.0f;
    param->ch1_ratio     = 1.0f;

    param->ch0_threshold = 3.0f;
    param->ch1_threshold = 3.0f;
    param->ch2_threshold = 100.0f;

    param->boot_flag     = BOOT_FLAG_NORMAL;
    param->dac_setpoint  = 0;

    param->alarm_write_idx = 0;
    param->alarm_count     = 0;

    param->crc = 0;
    param->crc = AppParam_CalcCrc(param);
}

/**
  * @brief      检查参数是否有效
  * @param      参数结构体指针
  * @retval     1有效，0无效
  */
int AppParam_CheckValid(AppParam_t *param)
{
    if (param->magic != PARAM_MAGIC)
        return 0;

    if (param->structSize != sizeof(AppParam_t))
        return 0;

    uint32_t old_crc = param->crc;
    param->crc = 0;
    uint32_t calc_crc = AppParam_CalcCrc(param);
    param->crc = old_crc;

    if (calc_crc != old_crc)
        return 0;

    return 1;
}

/**
  * @brief      从Flash指定地址读取AppParam
  * @param      Flash地址
  * @param      参数结构体指针
  * @retval     无
  */
static void AppParam_ReadFromFlash(uint32_t addr, AppParam_t *param)
{
    uint8_t *p = (uint8_t *)param;

    for (uint32_t i = 0; i < sizeof(AppParam_t); i++)
    {
        p[i] = internal_flash_read_Char(addr + i);
    }
}

/**
  * @brief      从参数区加载参数，无效则加载默认值并保存
  * @param      参数结构体指针
  * @retval     1加载成功，0已加载默认值
  */
int AppParam_Load(AppParam_t *param)
{
    AppParam_t tmp;

    AppParam_ReadFromFlash(PARAM_COPY_A_ADDR, &tmp);

    if (AppParam_CheckValid(&tmp))
    {
        memcpy(param, &tmp, sizeof(AppParam_t));
        return 1;
    }

    AppParam_ReadFromFlash(PARAM_COPY_B_ADDR, &tmp);

    if (AppParam_CheckValid(&tmp))
    {
        memcpy(param, &tmp, sizeof(AppParam_t));
        return 1;
    }

    AppParam_LoadDefault(param);
    AppParam_Save(param);

    return 0;
}

/**
  * @brief      保存参数到 Flash（双备份）
  * @param      参数结构体指针
  * @retval     1成功
  */
int AppParam_Save(AppParam_t *param)
{
    static uint8_t page[PARAM_PAGE_SIZE];
    uint32_t off_a = PARAM_COPY_A_ADDR - PARAM_PAGE_ADDR;
    uint32_t off_b = PARAM_COPY_B_ADDR - PARAM_PAGE_ADDR;

    param->magic      = PARAM_MAGIC;
    param->structSize = sizeof(AppParam_t);
    param->crc = 0;
    param->crc = AppParam_CalcCrc(param);

    for (uint32_t i = 0; i < PARAM_PAGE_SIZE; i++) {
        page[i] = internal_flash_read_Char(PARAM_PAGE_ADDR + i);
    }

    /* 只改 AppParam */
    memcpy(&page[off_a], param, sizeof(AppParam_t));
    memcpy(&page[off_b], param, sizeof(AppParam_t));

    internal_flash_erase(PARAM_PAGE_ADDR);
    internal_flash_write_str_Char(PARAM_PAGE_ADDR, page, PARAM_PAGE_SIZE);

    return 1;
}

/**
  * @brief      波特率编码转实际波特率值
  * @param      波特率编码 (0x11=4800, 0x12=9600, 0x13=19200, 0x14=115200)
  * @retval     实际波特率值
  */
uint32_t AppParam_BaudCodeToValue(uint8_t code)
{
    switch (code)
    {
        case 0x11: return 4800;
        case 0x12: return 9600;
        case 0x13: return 19200;
        case 0x14: return 115200;
        default:   return 19200;
    }
}

/**
  * @brief      App 参数初始化
  * @param      无
  * @retval     无
	* @note				启动时调用
  */
void AppParam_Init(void)
{
    AppParam_Load(&g_param);
}
