#include "BootConfig.h"
#include <string.h>

extern uint32_t crc32_calc(uint8_t* data, uint32_t len);
extern uint8_t  internal_flash_read_Char(uint32_t addr);
extern void     internal_flash_erase(uint32_t addr);
extern void     internal_flash_write_str_Char(uint32_t addr, uint8_t* data, uint32_t len);

uint8_t config_buf[CONFIG_APP_SIZE] = {0};

/**
  * @brief 		bootloader参数初始化
  * @param  	需要初始化的参数结构体指针
  * @retval 	无
  */
void bootloader_config_init(BootParam_t* param)
{
    BootParam_t tmp = { 0 };

    tmp.magicWord     = 0x5AA5C33C;
    tmp.version       = 0x0001;
    tmp.structSize    = sizeof(BootParam_t);
    tmp.updateMode    = 0x01;
    tmp.appStartAddr  = APP_START_ADDR;
    tmp.appEntryAddr  = APP_START_ADDR;
    tmp.appStackAddr  = *(uint32_t*)APP_START_ADDR;
    tmp.bootVersion   = 0x01;
    tmp.bootSize      = 64 * 1024;
    tmp.appVersion    = APP_VERSION;
    tmp.tailMagic     = 0xA5A5C3C3;

    memcpy(tmp.deviceID, "202601301528", 12);

    tmp.paramCRC32 = crc32_calc((uint8_t*)&tmp, sizeof(BootParam_t) - 4);
    *param = tmp;
}

/**
  * @brief 		app参数初始化
  * @param  	app参数结构体指针
  * @retval 	无
  */
void app_param_init(AppParam_t* param)
{
    if (param == NULL) return;

    memset(param, 0, sizeof(AppParam_t));

    param->magic         = 0xA5A5C33C;
    param->version       = 0x0001;
    param->structSize    = sizeof(AppParam_t);
    param->device_id     = 0x0001;      //默认设备ID
    param->baudrate_code = 13;          //默认19200
    param->ch0_ratio     = 1.0f;
    param->ch1_ratio     = 1.0f;
    param->ch0_threshold = 3.0f;        //默认阈值
    param->ch1_threshold = 3.0f;
    param->alarm_enable  = 0x02;        //默认不主动上报

    param->crc = crc32_calc((uint8_t*)param, sizeof(AppParam_t) - 4);
}

/**
  * @brief 		告警记录初始化
  * @param  	告警记录结构体指针
  * @retval 	无
  */
void alarm_record_init(AlarmRecord_t* records)
{
    if (records == NULL) return;
    memset(records, 0, sizeof(AlarmRecord_t) * ALARM_RECORD_MAX);
}

/**
  * @brief 		把参数全部读到config_buf去
  * @param  	无
  * @retval 	无
  */
void Read_ParamPage(void)
{
    extern uint8_t config_buf[CONFIG_APP_SIZE];
    for (uint16_t i = 0; i < CONFIG_SIZE; i++)
    {
        config_buf[i] = internal_flash_read_Char(BOOT_CONFIG_ADDR + i);
    }
}

/**
	* @brief 		写入整个参数区
  * @param  	参数结构体指针
  * @retval 	无
  */
void Write_ParamPage(Parameter_t* param)
{
    if (param == NULL) return;
    internal_flash_erase(BOOT_CONFIG_ADDR);
    internal_flash_write_str_Char(BOOT_CONFIG_ADDR, (uint8_t*)param, sizeof(Parameter_t));
}

/**
  * @brief 		添加告警记录
  * @param  	要被添加的app参数结构体指针
  * @param  	添加的告警记录的结构体指针
  * @retval 	是否添加成功 0/-1
  */
int alarm_record_add(AppParam_t* app_param, AlarmRecord_t* record)
{
  uint32_t alarm_offset, alarm_addr;
  uint32_t app_param_offset, app_param_addr;

  if (app_param == NULL || record == NULL)
	{
		return -1;
	}			

  alarm_offset = 256 + 256 + 256 	// 告警区偏移 = BootParam + BootParam_Reserved + AppParam
                 + (app_param->alarm_write_idx % ALARM_RECORD_MAX) * sizeof(AlarmRecord_t);
  alarm_addr = BOOT_CONFIG_ADDR + alarm_offset;

  internal_flash_write_str_Char(alarm_addr, (uint8_t*)record, sizeof(AlarmRecord_t));

  app_param->alarm_write_idx++;
  if (app_param->alarm_count < ALARM_RECORD_MAX)
  {
      app_param->alarm_count++;
  }

  app_param->crc = crc32_calc((uint8_t*)app_param, sizeof(AppParam_t) - 4);

  app_param_offset = 256 + 256;	// AppParam偏移 = BootParam + BootParam_Reserved
  app_param_addr = BOOT_CONFIG_ADDR + app_param_offset;
  internal_flash_write_str_Char(app_param_addr, (uint8_t*)app_param, sizeof(AppParam_t));

  return 0;
}
