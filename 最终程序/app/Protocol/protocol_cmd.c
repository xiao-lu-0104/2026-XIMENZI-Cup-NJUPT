#include "protocol_cmd.h"
#include "protocol_frame.h"
#include "app_sample.h"
#include "event_buff.h"
#include "app_param.h"
#include <string.h>

extern uint32_t crc32_calc(uint8_t *data, uint32_t len);
extern Eventqueue_t my_event_queue_t;

extern uint32_t g_rtc_time;

uint8_t g_report_interval = 0;   // 上报间隔：0=停止, 1=1s, 2=3s, 3=5s
uint8_t g_auto_report = 0;       // 上报标志位。0=停止, 1=运行中
uint32_t g_report_tick = 0;

extern uint16_t g_alarm_count;
extern char g_alarm_records[10][64];
extern uint16_t g_alarm_index;

/* ====================工具层==================== */

/**
  * @brief      单精度浮点数转大端字节序
  * @param      单精度浮点数
  * @param      4字节大端序数组
  * @retval     无
  */
void float_to_be(float value, uint8_t out[4])
{
    uint8_t *p = (uint8_t *)&value;
    out[0] = p[3];
    out[1] = p[2];
    out[2] = p[1];
    out[3] = p[0];
}

/**
  * @brief      大端字节序转单精度浮点数
  * @param      4字节大端序数组
  * @retval     单精度浮点数
  */
float be_to_float(uint8_t in[4])
{
    float val;
    uint8_t *p = (uint8_t *)&val;
    p[3] = in[0];
    p[2] = in[1];
    p[1] = in[2];
    p[0] = in[3];
    return val;
}
/* ====================API==================== */

void Protocol_ParseBinary(uint8_t *bin, uint16_t len)
{
    uint16_t header = (bin[0] << 8) | bin[1];
    uint16_t dev_id = (bin[2] << 8) | bin[3];
    uint8_t  type = bin[4];
    uint16_t cmd = (bin[5] << 8) | bin[6];
    uint8_t  data_len = bin[7];
    uint8_t *data = &bin[9];

    //校验帧头帧尾
    if (header != 0xA5B6) return;
    if (bin[len - 2] != 0xB6 || bin[len - 1] != 0xA5) return;

    
    uint16_t expected_len = 9 + data_len + 2 + 2;		//校验长度
    if (len != expected_len) return;
    if (dev_id != 0xFFFF && dev_id != g_param.device_id) return;

    /* 自动上报期间，只响应 0x0303 */
    if (g_auto_report)
    {
        if (cmd == 0x0303)
        {
            g_auto_report = 0;
            Protocol_SendOK(g_param.device_id, cmd);

            Event e0;
            e0.type = EVENT_AUTO_REPORT_STOP;
            e0.param1 = 0;
            e0.param2 = 0;
            event_queue_enter(&my_event_queue_t, e0);
        }
        return;
    }

    switch (type)
    {
        case FRAME_TYPE_CMD:
        {
            switch (cmd)
            {
                case 0x0101:
                {
                    Protocol_SendOK(g_param.device_id, cmd);
                    delay_1ms(100);

                    Event e1;
                    e1.type = EVENT_SOFT_Reset;
                    e1.param1 = 0;
                    e1.param2 = 0;
                    event_queue_enter(&my_event_queue_t, e1);
                    break;
                }

                case 0x0104:
                {
                    uint8_t ver[4] = {0x02, 0x00, 0x01, 0x00};
                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, ver, 4);
                    break;
                }

                case 0x0105:
                {
                    if (data_len != 4) break;

                    g_rtc_time = ((uint32_t)data[0] << 24) |
                                 ((uint32_t)data[1] << 16) |
                                 ((uint32_t)data[2] << 8)  |
                                 ((uint32_t)data[3]);

                    Protocol_SendOK(g_param.device_id, cmd);
                    break;
                }

                case 0x0106:
                {
                    uint8_t time_data[4];

                    time_data[0] = (g_rtc_time >> 24) & 0xFF;
                    time_data[1] = (g_rtc_time >> 16) & 0xFF;
                    time_data[2] = (g_rtc_time >> 8)  & 0xFF;
                    time_data[3] = g_rtc_time & 0xFF;

                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, time_data, 4);
                    break;
                }

                case 0x0111:
                {
                    uint8_t id_data[2];

                    id_data[0] = (g_param.device_id >> 8) & 0xFF;
                    id_data[1] = g_param.device_id & 0xFF;

                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, id_data, 2);
                    break;
                }

                case 0x01A1:
                {
                    if (data_len != 2) break;

                    uint16_t new_id = (data[0] << 8) | data[1];

                    if (new_id < 0x0001 || new_id > 0xFFFE) break;

                    g_param.device_id = new_id;
                    AppParam_Save(&g_param);

                    Protocol_SendOK(new_id, cmd);
                    break;
                }

                case 0x0112:
                {
                    uint8_t baud = g_param.baudrate_code;
                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, &baud, 1);
                    break;
                }

                case 0x01A2:
                {
                    if (data_len != 1) break;

                    uint8_t new_baud = data[0];

                    if (new_baud < 0x11 || new_baud > 0x14) break;

                    g_param.baudrate_code = new_baud;
                    AppParam_Save(&g_param);

                    Protocol_SendOK(g_param.device_id, cmd);
                    delay_1ms(200);

                    Event e2;
                    e2.type = EVENT_SOFT_Reset;
                    e2.param1 = 0;
                    e2.param2 = 0;
                    event_queue_enter(&my_event_queue_t, e2);
                    break;
                }

                case 0x0201:
                {
                    uint8_t d[4];
                    float_to_be(AppSample_GetCH0(), d);
                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, d, 4);
                    break;
                }

                case 0x0202:
                {
                    uint8_t d[4];
                    float_to_be(AppSample_GetCH1(), d);
                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, d, 4);
                    break;
                }

                case 0x0221:
                {
                    uint8_t d[4];
                    float_to_be(AppSample_GetCH2(), d);
                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, d, 4);
                    break;
                }

                case 0x0241:
                {
                    if (data_len != 4) break;

                    g_param.ch0_ratio = be_to_float(data);
                    AppParam_Save(&g_param);

                    Protocol_SendOK(g_param.device_id, cmd);
                    break;
                }

                case 0x0242:
                {
                    if (data_len != 4) break;

                    g_param.ch1_ratio = be_to_float(data);
                    AppParam_Save(&g_param);

                    Protocol_SendOK(g_param.device_id, cmd);
                    break;
                }

                case 0x0261:
                {
                    if (data_len != 1) break;

                    uint8_t interval = data[0];

                    if (interval < 1 || interval > 3) break;

                    g_report_interval = interval;

                    Protocol_SendOK(g_param.device_id, cmd);
                    break;
                }

                case 0x0301:
                {
                    if (data_len != 2) break;

                    uint16_t val = (data[0] << 8) | data[1];
                    BspDac_SetValue(val);

                    Protocol_SendOK(g_param.device_id, cmd);
                    break;
                }

                case 0x0302:
                {
                    g_auto_report = 1;
                    g_report_tick = 0;

                    Protocol_SendOK(g_param.device_id, cmd);

                    Event e3;
                    e3.type = EVENT_AUTO_REPORT_START;
                    e3.param1 = 0;
                    e3.param2 = 0;
                    event_queue_enter(&my_event_queue_t, e3);
                    break;
                }

                case 0x0303:
                {
                    g_auto_report = 0;

                    Protocol_SendOK(g_param.device_id, cmd);

                    Event e4;
                    e4.type = EVENT_AUTO_REPORT_STOP;
                    e4.param1 = 0;
                    e4.param2 = 0;
                    event_queue_enter(&my_event_queue_t, e4);
                    break;
                }

                case 0x03AA:
                {
                    Protocol_SendOK(g_param.device_id, cmd);
                    delay_1ms(10);

                    Event e;
                    e.type = EVENT_ENTER_SLEEP;
                    e.param1 = 0;
                    e.param2 = 0;
                    event_queue_enter(&my_event_queue_t, e);
                    break;
                }

                case 0x0400:
                {
                    uint8_t d[8];

                    float_to_be(g_param.ch0_threshold, &d[0]);
                    float_to_be(g_param.ch1_threshold, &d[4]);

                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, d, 8);
                    break;
                }

                case 0x0401:
                {
                    uint8_t d[4];

                    float_to_be(g_param.ch0_threshold, d);
                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, d, 4);
                    break;
                }

                case 0x0402:
                {
                    uint8_t d[4];

                    float_to_be(g_param.ch1_threshold, d);
                    Protocol_SendFrame(g_param.device_id, FRAME_TYPE_ACK, cmd, d, 4);
                    break;
                }
								
                case 0x0411:
                {
                    if (data_len != 4) break;

                    g_param.ch0_threshold = be_to_float(data);
                    AppParam_Save(&g_param);

                    Protocol_SendOK(g_param.device_id, cmd);
                    break;
                }

                case 0x0412:
                {
                    if (data_len != 4) break;

                    g_param.ch1_threshold = be_to_float(data);
                    AppParam_Save(&g_param);

                    Protocol_SendOK(g_param.device_id, cmd);
                    break;
                }

                case 0x0501:
                {
                    Protocol_SendOK(g_param.device_id, cmd);
                    rcu_periph_clock_enable(RCU_PMU);		//！！！用 RTC 备份寄存器传升级标志，不擦 Flash，保护参数区
                    pmu_backup_write_enable();
                    RTC_BKP1 = 0xA55A5AA5;

                    delay_1ms(100);

                    Event e5;
                    e5.type = EVENT_SOFT_Reset;
                    e5.param1 = 0;
                    e5.param2 = 0;
                    event_queue_enter(&my_event_queue_t, e5);
                    break;
                }

                case 0x0601:
                {
                    if (data_len != 1) break;

                    uint8_t mode = data[0];

                    if (mode != 0x01 && mode != 0x02) break;

                    g_param.alarm_enable = mode;
                    AppParam_Save(&g_param);

                    Protocol_SendOK(g_param.device_id, cmd);
                    break;
                }

                case 0x0602:
                {
                    Event e_a;
                    e_a.type = EVENT_ALARM_MSG;
                    e_a.param1 = 0;
                    e_a.param2 = 0;
                    event_queue_enter(&my_event_queue_t, e_a);
                    break;
                }

                case 0x0603:
                {
                    g_alarm_count = 0;
                    g_alarm_index = 0;
                    memset(g_alarm_records, 0, sizeof(g_alarm_records));

                    Protocol_SendOK(g_param.device_id, cmd);
                    break;
                }

                default:
                {
                    Protocol_SendError(g_param.device_id);
                    break;
                }
            }

            break;
        }

        case FRAME_TYPE_HEART:
        {
            if (cmd == CMD_BROADCAST)
            {
                Protocol_SendFrame(g_param.device_id, FRAME_TYPE_HEART, CMD_HEARTBEAT, NULL, 0);
            }
            break;
        }

        default:
        {
            Protocol_SendError(g_param.device_id);
            break;
        }
    }
}
