#ifndef __BOOTCONFIG_H__
#define __BOOTCONFIG_H__

/************************* 头文件 *************************/
#include <stdint.h>

/************************* 宏定义 *************************/
#define BOOT_CONFIG_ADDR        0x08010000  //参数区地址 4KB, 块1扇4
#define APP_START_ADDR          0x08011000  //App起始地址
#define APP_DOWNLOAD_ADDR       0x08051000  //固件暂存区地址

#define CONFIG_SIZE             4096        //参数区总大小 4KB
#define CONFIG_APP_SIZE         4096

#define ALARM_RECORD_MAX        10          //最多10条告警记录
#define APP_VERSION             0x02000100  //固件版本 2.0.1.0

/************************ 结构体定义 ************************/

/**
 * @brief 单条告警记录（32字节）
 */
typedef struct __attribute__((packed))
{
    uint32_t timestamp;         //告警时间戳(UTC)
    uint8_t  alarm_type;        //告警类型: 0=CH0, 1=CH1
    uint8_t  alarm_level;       //告警级别: 0=警告, 1=严重
    uint8_t  reserved[2];
    float    ch0_value;         //告警时CH0值(乘变比后)
    float    ch1_value;         //告警时CH1值(乘变比后)
    float    threshold_value;   //触发阈值
    uint32_t reserved2;
} AlarmRecord_t;  // 32字节

/**
 * @brief app参数（256字节）
 * @note  升级固件不会擦除，CRC32保护
 */
typedef struct __attribute__((packed))
{
    uint32_t magic;             // 魔数: 0xA5A5C33C
    uint16_t version;           // 参数版本: 0x0001
    uint16_t structSize;        // 结构体大小: 256
    uint32_t boot_flag;         // ★ 启动标志: 0xFFFFFFFF=正常, 0xA55A5AA5=升级
    uint16_t device_id;         // 设备ID
    uint8_t  baudrate_code;     // 波特率编码: 11=4800,12=9600,13=19200,14=115200
    uint8_t  reserved;
    float    ch0_ratio;         // CH0变比
    float    ch0_threshold;     // CH0阈值
    float    ch1_ratio;         // CH1变比
    float    ch1_threshold;     // CH1阈值
    uint8_t  alarm_enable;      // 告警使能: 0x01=主动上报, 0x02=不主动上报
    uint8_t  reserved2[3];
    uint32_t alarm_count;       // 现在的告警数量
    uint32_t alarm_write_idx;   // 告警写指针
    uint8_t  reserved3[188];    // ★ 192-4=188，保持总大小256
    uint32_t crc;               // CRC32(前252字节)
} AppParam_t;  // 256字节

/**
 * @brief Bootloader参数（256字节）
 * @note  只保留升级必需的字段
 */
typedef struct __attribute__((packed))
{
    uint32_t magicWord;         //魔数: 0x5AA5C33C
    uint16_t version;           //参数版本
    uint16_t structSize;        //结构体大小
    uint32_t reserved0;
    uint8_t  updateFlag;        //升级标志: 0x5A=升级, 0x00=正常
    uint8_t  updateMode;        //升级模式
    uint8_t  updateStatus;      //升级状态: 0x01=正在升级
    uint8_t  updateProgress;    //升级进度
    uint32_t updateCount;       //升级次数
    uint32_t lastUpdateTime;    //最后升级时间戳
    uint32_t appSize;           //App大小
    uint32_t appCRC32;          //App CRC32
    uint32_t appVersion;        //App版本
    uint32_t reserved1;
    uint32_t appStartAddr;      //App起始地址 0x08011000
    uint32_t appEntryAddr;      //App入口地址
    uint32_t appStackAddr;      //App栈地址
    uint32_t reserved2;
    uint32_t bootVersion;       //Bootloader版本
    uint32_t bootCRC32;
    uint32_t bootSize;          //Bootloader大小 64K
    uint32_t reserved3;
    uint32_t resetCount;        //复位次数
    uint32_t bootFailCount;     //启动失败次数
    uint8_t  deviceID[12];      //设备ID字符串(赛题要求存在参数区)
    uint8_t  reserved4[116];    //保留
    uint32_t paramCRC32;        //参数区CRC32
    uint32_t tailMagic;         //尾部魔数: 0xA5A5C3C3
} BootParam_t;  // 256字节

/**
 * @brief 参数区总结构体（4KB = 4096字节）
 * @note  布局:
 *        0x000-0x0FF  BootParam_t       256B  Bootloader管理参数
 *        0x100-0x1FF  BootParam_t备份   256B  双备份
 *        0x200-0x2FF  AppParam_t        256B  应用参数(变比/阈值/ID/波特率)
 *        0x300-0x43F  AlarmRecord_t[10] 320B  告警记录(10条)
 *        0x440-0xFFF  保留              3008B 保留
 */
typedef struct __attribute__((packed))
{
    BootParam_t     BootParam;              // 256B
    BootParam_t     BootParam_Reserved;     // 256B 双备份
    AppParam_t      AppParam;               // 256B 应用参数
    AlarmRecord_t   AlarmRecord[10];        // 320B 告警记录(10条)
    uint8_t         reserved[3008];         // 3008B 保留
} Parameter_t;  // 4096字节

/************************ 函数声明 ************************/
void bootloader_config_init(BootParam_t* param);
void app_param_init(AppParam_t* param);
void alarm_record_init(AlarmRecord_t* records);
void Read_ParamPage(void);
void Write_ParamPage(Parameter_t* param);
int  alarm_record_add(AppParam_t* app_param, AlarmRecord_t* record);

#endif
