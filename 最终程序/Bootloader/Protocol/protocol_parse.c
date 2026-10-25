#include "protocol_parse.h"

extern uint8_t config_buf[CONFIG_APP_SIZE];
extern uint32_t crc32_calc(uint8_t* data, uint32_t len);
extern void mcu_software_reset(void);

void protocol_parse(ring_buffer_t* buf, Parameter_t* param)
{
    uint8_t recv_buf[10 * 1024];
    uint16_t recv_len = 0;
    uint8_t byte;

    // 从传入的环形队列取数据
    while(ring_buffer_get_data_count(buf) > 0)
    {
        ring_buffer_read_safe(buf, &byte, 1);
        recv_buf[recv_len++] = byte;

        if(recv_len >= sizeof(recv_buf)) break;
    }

    if(recv_len < 8) return;

    uint32_t magic_value = 0;
    uint32_t appVersion = 0;

    // 字节序交换
    uint32_t tmp;
    tmp = recv_buf[0];
    recv_buf[0] = recv_buf[1];
    recv_buf[1] = tmp;
    tmp = recv_buf[2];
    recv_buf[2] = recv_buf[3];
    recv_buf[3] = tmp;
    tmp = recv_buf[0];
    recv_buf[0] = recv_buf[2];
    recv_buf[2] = tmp;
    tmp = recv_buf[1];
    recv_buf[1] = recv_buf[3];
    recv_buf[3] = tmp;
    tmp = recv_buf[4];
    recv_buf[4] = recv_buf[5];
    recv_buf[5] = tmp;
    tmp = recv_buf[6];
    recv_buf[6] = recv_buf[7];
    recv_buf[7] = tmp;
    tmp = recv_buf[4];
    recv_buf[4] = recv_buf[6];
    recv_buf[6] = tmp;
    tmp = recv_buf[5];
    recv_buf[5] = recv_buf[7];
    recv_buf[7] = tmp;

    memcpy(&magic_value, recv_buf, 4);
    memcpy(&appVersion, recv_buf + 4, 4);

    // 判断是.bin还是协议帧
    if (magic_value == 0x5AA5C33C)
    {
        //.bin
        uint32_t my_crc32;

        printf("magic: 0x%08X, version: 0x%08X\r\n", magic_value, appVersion);

        recv_len -= 8;
        my_crc32 = crc32_calc(recv_buf + 8, recv_len);
        printf("recv %d bytes, CRC32: 0x%08X\r\n", recv_len, my_crc32);

        // 擦除固件暂存区 128KB
        for (uint8_t i = 0; i < 32; i++)
        {
            internal_flash_erase(APP_DOWNLOAD_ADDR + i * 4096);
        }
        internal_flash_write_str_Char(APP_DOWNLOAD_ADDR, recv_buf + 8, recv_len);

        // 读参数区并更新
        Read_ParamPage();
        memcpy(param, config_buf, sizeof(Parameter_t));

        param->BootParam.magicWord    = 0x5AA5C33C;
        param->BootParam.appStartAddr = APP_START_ADDR;
        param->BootParam.appVersion   = appVersion;
        param->BootParam.updateFlag   = 0x5A;
        param->BootParam.updateStatus = 0x01;
        param->BootParam.appSize      = recv_len;
        param->BootParam.appCRC32     = my_crc32;

        Write_ParamPage(param);
        mcu_software_reset();
    }
    else
    {
        // ====== 协议帧（后续写） ======
        // handle_protocol_frame(recv_buf, recv_len, param);
    }
}
