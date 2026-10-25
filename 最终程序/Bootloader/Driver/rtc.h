#ifndef __RTC_H
#define __RTC_H

#include "HeaderFiles.h"

#define Alarm_Time 0x03

typedef struct
{
    uint8_t year;      
    uint8_t month;     
    uint8_t date;      
    uint8_t week;      

    uint8_t hour;      
    uint8_t minute;    
    uint8_t second;    
} my_rtc_time_t;


void my_rtc_init(void);
ErrStatus my_rtc_set_time(my_rtc_time_t *time);
void my_rtc_get_time(my_rtc_time_t *time);
uint8_t my_rtc_is_cold_start(void);   

#endif
