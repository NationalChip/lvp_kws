/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app_remote_control.c
 *
 */

#include <lvp_app.h>
#include <lvp_buffer.h>
#include <lvp_board.h>
#include <driver/gx_timer.h>
#include <board_config.h>
#include <autoconf.h>
#include <lvp_context.h>
#include <decoder.h>
#include "lvp_app_common.h"
#include "rf_code.h"
#include "led.h"
#include "lvp_audio_in.h"
#include <string.h>

/*****************************************************************************************************
-自测试要求：
-1 每个按键单独测试，短按，双击，长按等都正常
-2 取两个按键组合按键操作，短按，长按等都正常
-3 发码过程，按键或者语音，能够正常打断发码过程，重新发码
-4 持续长按某个按键，可以不进低功耗
-5 在待机下，按键和组合按键能够生效
-6 在进入低功耗的瞬间，按键能够打断和生效
-*****************************************************************************************************/

#define LOG_TAG "[Remote_Control]"
unsigned long endTime = 0;
unsigned long startTime = 0;
int wakeUpFlag = 0;
static char app_init = 0;
static int main_kws = 100;
int pmu_lock = 0;

static int g_single_tick = 0;
#define MAX_COUNT (20)
static int loop_count = MAX_COUNT;
static int start_ms = 0;
#define ENABLE_WATCHDOH

static char *enum_event_string[] = {
    ENUM_TO_STR(FLEX_BTN_PRESS_DOWN),
    ENUM_TO_STR(FLEX_BTN_PRESS_CLICK),
    ENUM_TO_STR(FLEX_BTN_PRESS_DOUBLE_CLICK),
    ENUM_TO_STR(FLEX_BTN_PRESS_REPEAT_CLICK),
    ENUM_TO_STR(FLEX_BTN_PRESS_SHORT_START),
    ENUM_TO_STR(FLEX_BTN_PRESS_SHORT_UP),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_START),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_UP),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_HOLD),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_HOLD_UP),
    ENUM_TO_STR(FLEX_BTN_PRESS_MAX),
    ENUM_TO_STR(FLEX_BTN_PRESS_NONE),
};

static GX_BUTTON_CONFIG btn_table[] = {
/*  按键对应的GPIO  按键名称  按下有效电平 持续长按  持续更长按  持续最长按*/
    {3,             "KEY1",    0,           1000,    3000,       7000},
    {4,             "KEY2",    0,           1000,    3000,       7000},
};


//   语音事件初始化数据
static Voive_InitTypeDef voice_event[] = {
    {.event_id = 100, .name = "xiao hao xiao hao"   , .data_code = 0x71},
    {.event_id = 101, .name = "yi jia shang sheng"  , .data_code = 0x88},
    {.event_id = 102, .name = "yi jia xia jiang"    , .data_code = 0xcc},
    {.event_id = 103, .name = "yi jia ting zhi"     , .data_code = 0xaa},
    {.event_id = 104, .name = "da kai zhao ming"    , .data_code = 0xf0},
    {.event_id = 105, .name = "guan bi zhao ming"   , .data_code = 0xf0},
    {.event_id = 106, .name = "da kai xiao du"      , .data_code = 0xce},
    {.event_id = 107, .name = "guan bi xiao du"     , .data_code = 0xce},
    {.event_id = 108, .name = "da kai feng shan"    , .data_code = 0xae},
    {.event_id = 109, .name = "guan bi feng shan"   , .data_code = 0xae},
    {.event_id = 110, .name = "da kai hong gan"     , .data_code = 0x6e},
    {.event_id = 111, .name = "guan bi hong gan"    , .data_code = 0x6e},
    {.event_id = 112, .name = "gong neng quan guan" , .data_code = 0x0E},
    // {.event_id = 115, .name = "yao kong qi dui ma"  , .data_code = 0x9e}
};

static int RemoteControlSuspend(void *priv)
{
    return 0;
}

static int RemoteControlResume(void *priv)
{
    if(!wakeUpFlag) {
        LvpSetVuiKwsStates(VUI_KWS_VAD_STATE);
        LvpDynamiciallyAdjustCpuFrequency(LOW_FREQUENCE);
#ifdef DEBUG_PRINTF
        printf("*************** cpu frequency is %dHz *************\n",gx_clock_get_module_frequence(CLOCK_MODULE_SCPU));
#endif
    }

    return 0;
}

static int button_state = 0; // 0 代表无按键， 1 代表KEY1单击，2 代表key2双击，3 代表 key0 key1同时按下, 根据项目，自己重新定义实现
static void common_btn_evt_cb(void *arg)
{
    // 该回调是在中断内的，因此不要做或多耗时间的动作
    GX_BUTTON_STATE *btn_event = (GX_BUTTON_STATE *)arg;
    printf("combination = %d, id_1 = %d, id_2 = %d, event = %s\n", btn_event->combination_button, btn_event->id_1, btn_event->id_2, enum_event_string[btn_event->event]);

    // KEY1 发生一次单击事件
    if ((btn_event->id_1 == 3) && (btn_event->combination_button == 0) && (btn_event->event == FLEX_BTN_PRESS_CLICK)) {
        button_state = 1;
    }

     // KEY2 发生一次单击事件
    if ((btn_event->id_1 == 4) && (btn_event->combination_button == 0) && (btn_event->event == FLEX_BTN_PRESS_DOUBLE_CLICK
)) {
        button_state = 2;
    }

    // KEY1 和 KEY2 同时按下
    if ((btn_event->id_1 == 3 && btn_event->id_2 == 4) && (btn_event->combination_button == 1) && (btn_event->event == FLEX_BTN_PRESS_DOWN)) {
        button_state = 3;
    }

#if 0
    // btn_event_handle = btn_event;
    // memcpy(&btn_event_handle, arg, sizeof(btn_event_handle));
#endif

}

static int RemoteControlInit(void)
{
    if(!app_init) {
        app_init = 1;

        LvpPmuSuspendLockCreate(&pmu_lock);
        getUniqueId();
        ledBright(400);
        LvpSetVuiKwsStates(VUI_KWS_VAD_STATE);
        LvpDynamiciallyAdjustCpuFrequency(LOW_FREQUENCE);
#ifdef DEBUG_PRINTF
        printf("*************** cpu frequency is %dHz *************\n",gx_clock_get_module_frequence(CLOCK_MODULE_SCPU));
        printf("time out --st %ld et %ld\n",startTime, endTime);
#endif
    }

    lvp_button_init(btn_table, ARRAY_SIZE(btn_table), common_btn_evt_cb);

    return 0;
}


//  语音唤醒事件
static int voiceEventAction(int event_id)
{
    printf("test event id = %d\n",event_id);

    unsigned int i;
    unsigned char size = sizeof(voice_event)/sizeof(voice_event[0]);

    for(i = 0; i < size ;i ++) {
        if(voice_event[i].event_id == event_id) break;
    }

    rf_send_code(voice_event[i].data_code);

    return 0;
}


// App Event Process
static int RemoteControlEventResponse(APP_EVENT *app_event)
{
    g_single_tick = 0;
    // 误唤醒
    if(app_event->event_id < 100) {
        return -1;
    }

    ledBright(200);
    LvpPmuSuspendLock(pmu_lock);
    // 主唤醒唤醒
    if(app_event->event_id == main_kws) {
        wakeUpFlag = 1;
        gx_rtc_get_tick(&endTime);
        LvpSetVuiKwsStates(VUI_KWS_ACTIVE_STATE);
        LvpDynamiciallyAdjustCpuFrequency(CPU_FREQUENCE_DEFAULT);
        voiceEventAction(app_event->event_id);
#ifdef DEBUG_PRINTF
        printf("*************** cpu frequency is %dHz *************\n",gx_clock_get_module_frequence(CLOCK_MODULE_SCPU));
#endif
    }

    if((wakeUpFlag == 1) && (app_event->event_id != main_kws)) {
        gx_rtc_get_tick(&endTime);
        voiceEventAction(app_event->event_id);
    }

    return 0;
}

static int RemoteControlTaskLoop(void)
{
    if(wakeUpFlag) {
        gx_rtc_get_tick(&startTime);
        if(startTime - endTime > TIME_OUT_MS) {
            wakeUpFlag = 0;
            LvpPmuSuspendUnlock(pmu_lock);
            LvpSetVuiKwsStates(VUI_KWS_VAD_STATE);
            LvpDynamiciallyAdjustCpuFrequency(LOW_FREQUENCE);
#ifdef DEBUG_PRINTF
            printf("*************** cpu frequency is %dHz *************\n",gx_clock_get_module_frequence(CLOCK_MODULE_SCPU));
            printf("time out --st %ld et %ld\n",startTime, endTime);
#endif
        }
    }

    if (button_state != 0) {
        if (button_state == 1) {  // key1 按下
            printf("key1 single click !\n");
            rf_send_code(0x23);
        }else if (button_state == 2) {  // key2按下
            printf("key2 double click !\n");
            rf_send_code(0x24);
        }else if (button_state ==3 ) {  // key1和key2同时按下
            printf("key1 and key2 press down ! \n");
            rf_send_code(0x25);
        }else {}

        button_state = 0;
    }
#if 0
    if (button_state != 0) {

        printf("combination = %d, id_1 = %d, id_2 = %d, event = %s\n", btn_event_handle.combination_button, btn_event_handle.id_1,
        btn_event_handle.id_2, enum_event_string[btn_event_handle.event]);

        button_state = 0;
    }
#endif

    return 0;
}

LVP_APP remote_control_app = {
    .app_name = "remote_control_app",
    .AppInit = RemoteControlInit,
    .AppEventResponse = RemoteControlEventResponse,
    .AppTaskLoop = RemoteControlTaskLoop,
    .AppSuspend = RemoteControlSuspend,
    .suspend_priv = "RemoteControlSuspend",
    .AppResume = RemoteControlResume,
    .resume_priv = "RemoteControlResume",
};

LVP_REGISTER_APP(remote_control_app);
