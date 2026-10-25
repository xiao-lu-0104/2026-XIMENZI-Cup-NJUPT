#include "Function.h"
#include "flash_map.h"
#include "app_param.h"
#include "gd32f4xx_rtc.h"
#include "gd32f4xx_pmu.h"
#include <string.h>
#include <stdio.h>
#include "protocol_frame.h"

#define FIRMWARE_MAGIC       0x5AA5C33CUL
#define OTA_BUF_SIZE         8192

ring_buffer_t usart1_rx_buf;

typedef void (*pFunction)(void);
static pFunction jump2app;

char buf[64];
/********** 函数声明 **********/
uint32_t crc32_calc(uint8_t *data, uint32_t len);
void mcu_software_reset(void);
void iap_load_app(uint32_t appxaddr);
void jump_to_app(void);
static void OTA_Reset(void);
static void OTA_FlushBuffer(void);
/********** OTA状态机 **********/
typedef enum { OTA_IDLE, OTA_RECEIVING, OTA_COMPLETE } ota_state_t;

static volatile ota_state_t g_ota_state        = OTA_IDLE;
static volatile uint8_t     g_ota_data_started = 0;
static          uint32_t    g_ota_addr         = 0;
static          uint32_t    g_ota_total        = 0;
static volatile uint32_t    g_ota_idle         = 0;
static          uint8_t     g_ota_buf[OTA_BUF_SIZE];
static volatile uint16_t    g_ota_buf_cnt      = 0;
static          uint32_t    g_ota_erased       = 0;

int OTA_IsReceiving(void) 
{
    return (g_ota_state == OTA_RECEIVING);
}

void OTA_ProcessByte(uint8_t byte)
{
    if (g_ota_state != OTA_RECEIVING)
        return;

    if (g_ota_buf_cnt < OTA_BUF_SIZE)
    {
        g_ota_buf[g_ota_buf_cnt++] = byte;
        g_ota_idle = 0;
        g_ota_data_started = 1;
    }
    else
    {
        g_ota_state = OTA_IDLE;
    }
}


void System_Init(void)
{
    systick_config();
    LED_Init();
    OLED_Init();
    OLED_Clear();
    OLED_ShowNum(0, 0, 2026409373, 10, 16);
    OLED_ShowString(0, 16, "Bootloader", 16);
    OLED_Refresh();
    ring_buffer_init(&usart1_rx_buf);
    AppParam_Init();

    uint32_t baud = AppParam_BaudCodeToValue(g_param.baudrate_code);
    USART1_Init(baud);
}

void UsrFunction(void)
{
    if (RTC_BKP1 == 0xA55A5AA5)
    {
        rcu_periph_clock_enable(RCU_PMU);
        pmu_backup_write_enable();
        RTC_BKP1 = 0;

        OLED_Clear();
        OLED_ShowNum(0, 0, 2026409373, 10, 16);
        OLED_ShowString(0, 16, "Bootloader", 16);
        OLED_Refresh();

        uint16_t my_id = g_param.device_id;
        int ota_complete = 0;
        uint32_t countdown = 10;
        uint32_t countdown_ticks = 0;
				//擦除OTA区域
        OTA_Reset();

        while (!ota_complete)
        {
            if (ring_buffer_get_data_count(&usart1_rx_buf) > 0)		//协议帧
                protocol_frame_parser(&usart1_rx_buf);

            /* OTA 接收 */
						if (g_ota_state == OTA_RECEIVING)
						{
								if (g_ota_data_started)
										g_ota_idle++;
                if (g_ota_idle >= 50)     /* 50*10 = 500ms */
                {
                    OTA_FlushBuffer();
                    g_ota_state = OTA_COMPLETE;

                    uint8_t *b = (uint8_t *)APP_DOWNLOAD_ADDR;
                    uint32_t magic = ((uint32_t)b[0] << 24)
                                   | ((uint32_t)b[1] << 16)
                                   | ((uint32_t)b[2] << 8)
                                   | b[3];
                    if (magic == FIRMWARE_MAGIC) {
                        Protocol_SendOK(my_id, 0x0502);
                        countdown = 0;      /* 等0503 */
                    } else {
                        Protocol_SendError_0502(my_id);
                    }
                }
            }
						
            if (g_ota_state != OTA_RECEIVING && countdown > 0)
            {
                countdown_ticks++;
                if (countdown_ticks >= 100)
                {
                    countdown_ticks = 0;
                    countdown--;
                    int len = sprintf(buf,
                        "wait for start Application(%ds)......\r\n", countdown);
                    USART1_SendData((uint8_t *)buf, len);
                    if (countdown == 0) 
										{
                        break;
                    }
                }
            }

            delay_1ms(10);
        }
    }
    else
    {
        delay_1ms(5000);
    }

    jump_to_app();
}

/********** OTA内部 **********/
static void OTA_Reset(void)
{
    g_ota_state        = OTA_IDLE;
    g_ota_addr         = APP_DOWNLOAD_ADDR;
    g_ota_total        = 0;
    g_ota_idle         = 0;
    g_ota_buf_cnt      = 0;
    g_ota_erased       = APP_DOWNLOAD_ADDR;
    g_ota_data_started = 0;
}

static void OTA_EnsureErased(uint32_t needed_addr)
{
    while (needed_addr >= g_ota_erased) {
        __disable_irq();
        internal_flash_erase(g_ota_erased);
        __enable_irq();
        g_ota_erased += 4096;
    }
}

static void OTA_FlushBuffer(void)
{
    if (g_ota_buf_cnt == 0)
        return;

    if (g_ota_buf_cnt > OTA_BUF_SIZE)
    {
        g_ota_buf_cnt = 0;
        g_ota_state = OTA_IDLE;
        return;
    }

    uint16_t n = g_ota_buf_cnt;

    g_ota_buf_cnt = 0;

    OTA_EnsureErased(g_ota_addr + n - 1);

    __disable_irq();
    internal_flash_write_str_Char(g_ota_addr, g_ota_buf, n);
    __enable_irq();

    g_ota_addr  += n;
    g_ota_total += n;

}

/********** BOOTLOADER的协议解析 **********/
void Protocol_ParseBinary(uint8_t *bin, uint16_t len)
{
    uint16_t header   = (bin[0] << 8) | bin[1];
    uint16_t dev_id   = (bin[2] << 8) | bin[3];
    uint8_t  type     = bin[4];
    uint16_t cmd      = (bin[5] << 8) | bin[6];
    uint8_t  data_len = bin[7];
    uint16_t my_id    = g_param.device_id;

    if (header != 0xA5B6) return;
    if (bin[len - 2] != 0xB6 || bin[len - 1] != 0xA5) return;
    if (len != (uint16_t)(9 + data_len + 2 + 2)) return;
    if (dev_id != 0xFFFF && dev_id != my_id) return;

    switch (type)
    {
        case 0x01:
            switch (cmd)
            {
                case 0x0111:
                {
                    uint8_t d[2];
                    d[0] = (my_id >> 8) & 0xFF;
                    d[1] = my_id & 0xFF;
                    Protocol_SendFrame(my_id, 0x02, cmd, d, 2);
                    break;
                }
                case 0x0502:
                {
                    OTA_Reset();
                    internal_flash_erase(APP_DOWNLOAD_ADDR);
                    g_ota_erased = APP_DOWNLOAD_ADDR + 4096;
                    g_ota_state  = OTA_RECEIVING;
                    g_ota_addr   = APP_DOWNLOAD_ADDR;
                    break;
                }
                case 0x0503:
                {
                    Protocol_SendOK(my_id, cmd);

                    uint8_t *b = (uint8_t *)APP_DOWNLOAD_ADDR;
                    uint32_t magic = ((uint32_t)b[0] << 24)
                                   | ((uint32_t)b[1] << 16)
                                   | ((uint32_t)b[2] << 8) | b[3];
                    if (magic != FIRMWARE_MAGIC || g_ota_total == 0) break;

                    g_param.boot_flag = BOOT_FLAG_NORMAL;
                    AppParam_Save(&g_param);

                    uint32_t header   = 4;   //魔数字4个字节
                    uint32_t app_size = g_ota_total - header;
										/* 写到 APP 区 */
                    uint32_t dest     = APP_START_ADDR;  

                    __disable_irq();
                    uint32_t pages = (app_size + 4095) / 4096;
                    for (uint32_t i = 0; i < pages; i++)
                        internal_flash_erase(dest + i * 4096);
                    for (uint32_t i = 0; i < app_size; i++) {
                        uint8_t ch = internal_flash_read_Char(APP_DOWNLOAD_ADDR + header + i);
                        internal_flash_write_str_Char(dest + i, &ch, 1);
                    }

                    NVIC_SystemReset();
                    while (1);
                    break;
                }
                default: Protocol_SendError(my_id); break;
            }
            break;

        case 0x05:
            if (cmd == 0xFFFF)
                Protocol_SendFrame(my_id, 0x05, 0x8888, NULL, 0);
            break;

        default: break;
    }
}

/**
  * @brief 		CRC32校验
  * @param  	数据
  * @param  	长度
  * @retval 	校验码
  */
uint32_t crc32_calc(uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint32_t j = 0; j < 8; j++)
            crc = (crc & 1) ? ((crc >> 1) ^ 0xEDB88320) : (crc >> 1);
    }
    return crc ^ 0xFFFFFFFF;
}

void mcu_software_reset(void)
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}

void iap_load_app(uint32_t appxaddr)
{
    uint32_t app_stack = *(__IO uint32_t *)appxaddr;
    uint32_t app_reset = *(__IO uint32_t *)(appxaddr + 4);
    if ((app_stack & 0x2FFE0000) == 0x20000000)
    {
        __disable_irq();

        SysTick->CTRL = 0; SysTick->LOAD = 0; SysTick->VAL = 0;

        for (uint32_t i = 0; i < 8; i++) {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }
        __DSB(); __ISB();

        SCB->VTOR = appxaddr;
        __set_MSP(*(__IO uint32_t *)appxaddr);
        jump2app = (pFunction)(*(__IO uint32_t *)(appxaddr + 4));
        jump2app();
        while (1);
    }
}

void jump_to_app(void)
{
    uint32_t sp  = *(__IO uint32_t *)(APP_START_ADDR);
    uint32_t rv  = *(__IO uint32_t *)(APP_START_ADDR + 4);
    if ((rv >= 0x08000000) && ((sp & 0x2FFE0000) == 0x20000000))
        iap_load_app(APP_START_ADDR);
    else 
		{ 
			while (1); 
		}
}
