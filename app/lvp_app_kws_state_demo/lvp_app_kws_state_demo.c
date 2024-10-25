/* Voice Signal Preprocess
* Copyright (C) 2001-2021 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* lvp_app_kws_state_demo.c
*
*/

#include <lvp_app.h>
#include <lvp_buffer.h>
#include <decoder.h>
#include <driver/gx_rtc.h>
#include <lvp_system_init.h>
#include <driver/gx_gpio.h>
#include <driver/gx_clock/gx_clock_v2.h>
#include <lvp_context.h>
#include <lvp_board.h>

#include "vc_led.h"
#define LOG_TAG "[KWS_STATE_DEMO_APP]"

// #define DO_NOT_SLEEP_WHEN_WAKING_UP     // 定义后唤醒词唤醒后mcu不会进入休眠，超时之后才会进入休眠，有些客户会有这个需求

#ifdef DO_NOT_SLEEP_WHEN_WAKING_UP
#include <lvp_pmu.h>
#endif

#ifdef CONFIG_MCU_LOW_FREQUENCY_4M
#define __LOW_FREQUENCE CPU_FREQUENCE_4M
#elif defined CONFIG_MCU_LOW_FREQUENCY_8M
#define __LOW_FREQUENCE CPU_FREQUENCE_8M
#elif defined CONFIG_MCU_LOW_FREQUENCY_12M
#define __LOW_FREQUENCE CPU_FREQUENCE_12M
#elif defined CONFIG_MCU_LOW_FREQUENCY_24M
#define __LOW_FREQUENCE CPU_FREQUENCE_24M
#endif

#define __TIME_OUT_S    10
#define __WAKE_UP_KWS_1 100
#define __WAKE_UP_KWS_2 100

typedef struct TIMEOUT_STANDBY{
    unsigned long first_time;
    unsigned long next_time;
    unsigned char timeout_flag;
}TIMEOUT_STANDBY;

static TIMEOUT_STANDBY kws_state_time = {0, 0, 1};  // 超时标志位设置为1，初始状态为超时状态
static int kws_state_init_flag = 0;
#ifdef DO_NOT_SLEEP_WHEN_WAKING_UP
static int lock = 0;
#endif
//=================================================================================================

static int KwsStateDemoAppSuspend(void *priv)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
    BoardSetPowerSavePinMux();
    return 0;
}

static int KwsStateDemoAppResume(void *priv)
{
    BoardSetUserPinMux();
    printf(LOG_TAG" ---- %s ----\n", __func__);
    if (kws_state_time.timeout_flag)
        LvpDynamiciallyAdjustCpuFrequency(__LOW_FREQUENCE);
    printf("*************** cpu frequency is %dHz *************\n",gx_clock_get_module_frequence(CLOCK_MODULE_SCPU));
    return 0;
}

static int KwsStateDemoAppInit(void)
{
    if (!kws_state_init_flag)
    {
#ifdef DO_NOT_SLEEP_WHEN_WAKING_UP
        LvpPmuSuspendLockCreate(&lock);
#endif
        LvpSetVuiKwsStates(VUI_KWS_VAD_STATE);   // 框架默认为active状态
        kws_state_init_flag = 1;
        gx_rtc_get_tick(&kws_state_time.first_time);
    }
    return 0;
}

// App Event Process
static int KwsStateDemoAppEventResponse(APP_EVENT *app_event)
{
    // 超时检测
    if (!kws_state_time.timeout_flag) {
    gx_rtc_get_tick(&kws_state_time.next_time);
    if (kws_state_time.next_time - kws_state_time.first_time > __TIME_OUT_S) {
        kws_state_time.timeout_flag = 1;
        LvpSetVuiKwsStates(VUI_KWS_VAD_STATE);
        LvpDynamiciallyAdjustCpuFrequency(__LOW_FREQUENCE);
        printf("*****************cpu frequency is %dHz ************\n",gx_clock_get_module_frequence(CLOCK_MODULE_SCPU));
        printf("timeout! ---> next_time%ds - first_time%ds = %ds \n", kws_state_time.next_time,\
                kws_state_time.first_time, kws_state_time.next_time - kws_state_time.first_time);
#ifdef DO_NOT_SLEEP_WHEN_WAKING_UP
        LvpPmuSuspendUnlock(lock);      // 超时后解锁
#endif
        }
    }



    if (app_event->event_id < 100)
        return 0;

    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(app_event->ctx_index, &context, &ctx_size);

    if (app_event->event_id == __WAKE_UP_KWS_1 || app_event->event_id == __WAKE_UP_KWS_2) {
        if(kws_state_time.timeout_flag){
            LvpSetVuiKwsStates(VUI_KWS_ACTIVE_STATE);
            LvpDynamiciallyAdjustCpuFrequency(CPU_FREQUENCE_DEFAULT);
#ifdef DO_NOT_SLEEP_WHEN_WAKING_UP
            LvpPmuSuspendLock(lock);    // 唤醒后上锁
#endif
        }
        printf("*************** cpu frequency is %dHz *************\n",gx_clock_get_module_frequence(CLOCK_MODULE_SCPU));
    }

#ifdef CONFIG_APP_VC_HAS_LED
    EnableGpioLed();
    KwsLedFlicker(200, 0, 1);
#endif

    gx_rtc_get_tick(&kws_state_time.first_time);
    kws_state_time.timeout_flag = 0;
    printf(LOG_TAG"event_id %d\n", app_event->event_id);

    return 0;
}

// APP Main Loop
static int KwsStateDemoAppTaskLoop(void)
{
    return 0;
}


LVP_APP kws_state_demo_app = {
    .app_name = "kws state demo app",
    .AppInit = KwsStateDemoAppInit,
    .AppEventResponse = KwsStateDemoAppEventResponse,
    .AppTaskLoop = KwsStateDemoAppTaskLoop,
    .AppSuspend = KwsStateDemoAppSuspend,
    .suspend_priv = "KwsStateDemoAppSuspend",
    .AppResume = KwsStateDemoAppResume,
    .resume_priv = "KwsStateDemoAppResume",
};

LVP_REGISTER_APP(kws_state_demo_app);

