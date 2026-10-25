// Function/flash_map.h
#ifndef __FLASH_MAP_H__
#define __FLASH_MAP_H__

#include <stdint.h>

#define FLASH_BOOT_ADDR          0x08000000UL
#define FLASH_PARAM_ADDR         0x08010200UL
#define FLASH_APP_ADDR           0x08011000UL
#define FLASH_APP_BACKUP_ADDR    0x08031000UL
#define FLASH_OTA_TEMP_ADDR      0x08051000UL

#define FLASH_BOOT_SIZE          (64UL  * 1024UL)
#define FLASH_PARAM_SIZE         (4UL   * 1024UL)
#define FLASH_APP_SIZE           (128UL * 1024UL)
#define FLASH_APP_BACKUP_SIZE    (128UL * 1024UL)
#define FLASH_OTA_TEMP_SIZE      (128UL * 1024UL)

#define APP_START_ADDR           FLASH_APP_ADDR
#define APP_BACKUP_ADDR          FLASH_APP_BACKUP_ADDR
#define APP_DOWNLOAD_ADDR        FLASH_OTA_TEMP_ADDR

#define FIRMWARE_MAGIC           0x5AA5C33CUL

#endif
