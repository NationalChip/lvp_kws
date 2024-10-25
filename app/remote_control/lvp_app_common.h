#ifndef LVP_APP_COMMON_H
#define LVP_APP_COMMON_H

#include <lvp_system_init.h>
#include <driver/gx_clock/gx_clock_v2.h>
#include <driver/gx_flash.h>
#include <driver/gx_gpio.h>
#include <lvp_pmu.h>
#include <driver/gx_rtc.h>
#include <common.h>
#include <driver/gx_delay.h>
#include <driver/gx_flash.h>
#include <driver/gx_irq.h>
#include "lvp_button.h"
#include "flexible_button.h"


/*
*      k1 ------ GPIO09-------全关
       k2--------GPIO08-------消毒
       k3--------GPIO00-------热风
       K4--------GPIO07-------上升
       K5--------GPIO04-------下降
       K6--------GPIO01-------风干
       k8--------GPIO12-------照明
*/

// #define DEBUG_PRINTFc
/* LED */
#define LED_GPIO 10
/* RF */
#define RF_GPIO  2

#define BUTTON_UP     7
#define BUTTON_DOWN   4
#define BUTTON_POWER  9
#define BUTTON_MING   12
#define BUTTON_STOP   11
#define BUTTON_HONG   0
#define BUTTON_DU     8


extern int pmu_lock;
extern int wakeUpFlag;
extern unsigned long endTime;

#define TIME_OUT_MS 10
#define LOW_FREQUENCE CPU_FREQUENCE_4M
#define SEND_TIMES       4            // 连续发码的次数


typedef struct{
    unsigned int    event_id;                 //  语音事件ID
    unsigned char   name[20];                 //  语音名字，长度：20byte
    unsigned char   data_code;                //  数据码
}Voive_InitTypeDef;


#endif