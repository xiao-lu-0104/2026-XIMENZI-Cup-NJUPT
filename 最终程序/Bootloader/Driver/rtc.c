#include "rtc.h"

/* 备份寄存器魔数，用于判断备份域是否掉电过 */
#define RTC_BKP_MAGIC   0xB5A7U    
#define RTC_BKP_MARKER  RTC_BKP0    

/* 记录本次是否为冷启动（1=冷启动，0=热启动） */
static uint8_t s_cold_start_flag = 0;


/**
 * @brief 十进制转BCD
 * @param 十进制数据
 * @return BCD数据
 */
static uint8_t dec_to_bcd(uint8_t val)
{
    return (uint8_t)(((val / 10U) << 4U) | (val % 10U));
}


/**
 * @brief 	BCD转十进制
 * @param 	BCD数据
 * @return 十进制数据
 */
static uint8_t bcd_to_dec(uint8_t val)
{
    return (uint8_t)(((val >> 4U) * 10U) + (val & 0x0FU));
}


/**
  * @brief 		RTC初始化
  * @param  	无
  * @retval 	无
  */
void my_rtc_init(void)
{
    rtc_parameter_struct rtc_initpara;
    rtc_alarm_struct rtc_alarm;


    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();


    if(RTC_BKP_MAGIC != RTC_BKP_MARKER)
    {

        rcu_bkp_reset_enable();
        rcu_bkp_reset_disable();

        rcu_periph_clock_enable(RCU_RTC);

        rcu_osci_on(RCU_LXTAL);
        if(SUCCESS == rcu_osci_stab_wait(RCU_LXTAL))
        {
            /* LXTAL 起振成功 */
            rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
            rtc_initpara.factor_asyn = 0x7F;    
            rtc_initpara.factor_syn  = 0xFF;    
            printf("RTC: use LXTAL 32.768KHz\r\n");
        }
        else
        {
            /* LXTAL 起振失败 */
            rcu_osci_on(RCU_IRC32K);
            rcu_osci_stab_wait(RCU_IRC32K);
            rcu_rtc_clock_config(RCU_RTCSRC_IRC32K);
            rtc_initpara.factor_asyn = 0x7F;    
            rtc_initpara.factor_syn  = 0xF9;    
            printf("RTC: LXTAL failed, fallback to IRC32K\r\n");
        }

        rtc_initpara.year           = 0x26;          
        rtc_initpara.day_of_week    = RTC_THURSDAY;  
        rtc_initpara.month          = RTC_JUN;
        rtc_initpara.date           = 0x04;
        rtc_initpara.display_format = RTC_24HOUR;
        rtc_initpara.am_pm          = RTC_AM;
        rtc_initpara.hour           = 0x00;
        rtc_initpara.minute         = 0x00;
        rtc_initpara.second         = 0x00;

        if(SUCCESS == rtc_init(&rtc_initpara))
        {
            /* 配置闹钟 */
            rtc_alarm_disable(RTC_ALARM0);	//写之前先关闹钟
            rtc_alarm.alarm_mask = RTC_ALARM_DATE_MASK | RTC_ALARM_HOUR_MASK | RTC_ALARM_MINUTE_MASK;
            rtc_alarm.weekday_or_date = RTC_ALARM_DATE_SELECTED;
            rtc_alarm.alarm_day    = 0x31;
            rtc_alarm.am_pm        = RTC_AM;
            rtc_alarm.alarm_hour   = 0x00;
            rtc_alarm.alarm_minute = 0x00;
            rtc_alarm.alarm_second = Alarm_Time;
            rtc_alarm_config(RTC_ALARM0, &rtc_alarm);
            rtc_flag_clear(RTC_FLAG_ALRM0);
            rtc_interrupt_enable(RTC_INT_ALARM0);
            rtc_alarm_enable(RTC_ALARM0);
						rtc_flag_clear(RTC_FLAG_ALRM0);

            RTC_BKP_MARKER = RTC_BKP_MAGIC;

            s_cold_start_flag = 1;

            printf("RTC: cold start, backup domain initialized\r\n");
        }
        else
        {
            printf("RTC: init FAILED!\r\n");
            s_cold_start_flag = 0;
        }
    }
    else
    {
        rcu_periph_clock_enable(RCU_RTC);

        rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

        s_cold_start_flag = 0;
        printf("RTC: warm start, time preserved\r\n");
    }
}

/**
  * @brief  查询本次上电是否为冷启动
  * @param  无
  * @retval 1 = 冷启动
  *         0 = 热启动
  */
uint8_t my_rtc_is_cold_start(void)
{
    return s_cold_start_flag;
}


/**
 * @brief 设置RTC日期和时间
 * @param time RTC时间结构体，使用十进制
 * @return SUCCESS or ERROR
 */
ErrStatus my_rtc_set_time(my_rtc_time_t *time)
{
    ErrStatus error_status = ERROR;
    uint32_t reg_time = 0U, reg_date = 0U;

    if(time == NULL)
    {
        return ERROR;
    }

    if(time->year > 99U)
    {
        return ERROR;
    }

    if((time->month < 1U) || (time->month > 12U))
    {
        return ERROR;
    }

    if((time->date < 1U) || (time->date > 31U))
    {
        return ERROR;
    }

    if(time->hour > 23U)
    {
        return ERROR;
    }

    if(time->minute > 59U)
    {
        return ERROR;
    }

    if(time->second > 59U)
    {
        return ERROR;
    }

    reg_date = (DATE_YR(dec_to_bcd(time->year)) | \
                DATE_DOW(time->week) | \
                DATE_MON(dec_to_bcd(time->month)) | \
                DATE_DAY(dec_to_bcd(time->date)));

    reg_time = (RTC_AM | \
                TIME_HR(dec_to_bcd(time->hour))  | \
                TIME_MN(dec_to_bcd(time->minute)) | \
                TIME_SC(dec_to_bcd(time->second)));


    rtc_alarm_disable(RTC_ALARM0);      //解除写保护

    RTC_WPK = RTC_UNLOCK_KEY1;
    RTC_WPK = RTC_UNLOCK_KEY2;

    //进入init模式前必须先写闹钟影子寄存器
    RTC_ALRM0TD = 0xFFFF;
    RTC_ALRM1TD = 0xFFFF;
    RTC_ALRM0SS = 0x0022;
    RTC_ALRM1SS = 0x0022;

    error_status = rtc_init_mode_enter();

    if(ERROR != error_status) {
        RTC_TIME = (uint32_t)reg_time;
        RTC_DATE = (uint32_t)reg_date;

        RTC_CTL &= (uint32_t)(~RTC_CTL_CS);
        RTC_CTL |=  RTC_24HOUR;

        rtc_init_mode_exit();

        error_status = rtc_register_sync_wait();
    }

    //重启写保护
    RTC_WPK = RTC_LOCK_KEY;

    rtc_alarm_enable(RTC_ALARM0);

    return error_status;
}


/**
  * @brief 		读取RTC时间和日期
  * @param  	time RTC时间结构体
  * @retval 	返回十进制时间
  */
void my_rtc_get_time(my_rtc_time_t *time)
{
    rtc_parameter_struct rtc_getpara;

    if(time == NULL)
    {
        return;
    }

    rtc_current_time_get(&rtc_getpara);
    //RTC返回的是BCD格式，转换为十进制
    time->year   = bcd_to_dec(rtc_getpara.year);
    time->month  = bcd_to_dec(rtc_getpara.month);
    time->date   = bcd_to_dec(rtc_getpara.date);
    time->week   = rtc_getpara.day_of_week;

    time->hour   = bcd_to_dec(rtc_getpara.hour);
    time->minute = bcd_to_dec(rtc_getpara.minute);
    time->second = bcd_to_dec(rtc_getpara.second);
}
