#ifndef __HEADERFILES_H
#define __HEADERFILES_H

/************************* 头文件 *************************/

#include "gd32f4xx.h"
#include "gd32f4xx_libopt.h"
#include "systick.h"
#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include <stdbool.h>
/*************************Driver*************************/
#include "LED.h"
#include "TIM.h"
#include "DAC.h"
#include "USART.h"
#include "ADC.h"
#include "OLED.h"
#include "rom.h"
#include "AD3344.h"
#include "ring_buff.h"
#include "event_buff.h"
#include "flash_map.h"
/*************************Protocol*************************/
#include "protocol_crc.h"
#include "protocol_ascii.h"
#include "protocol_frame.h"
#include "protocol_cmd.h"
#include "protocol_frame_parser.h"
/*************************Function*************************/
#include "Function.h"     //执行函数头文件
#include "app_param.h"
#include "app_sample.h"
#endif

/****************************End*****************************/

