/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app_voice_controller_v2.c
 *
 */

#include <lvp_app.h>
#include <lvp_buffer.h>
#include "vc_message.h"
#include <lvp_pmu.h>
#include <lvp_board.h>
#include <driver/gx_gpio.h>
#include <autoconf.h>

#define LOG_TAG "[VOICE_CONTROLLER_V2]"
static int pmu_lock = 0;
static char app_init = 0;
//=================================================================================================
#ifdef CONFIG_UART_AWAKE
static int gpio_callback(int port, void *pdata)
{
    printf("%s %d\n", __func__, __LINE__);
}
#endif

static int VoiceControllerSuspend(void *priv)
{
    // BoardSetPowerSavePinMux();//根据具体项目进该函数配置低功耗下管脚模式及状态

    //printf("%s\n", (unsigned char *)priv);
    
#ifdef CONFIG_UART0_AWAKE
    padmux_set(6 , 1);
    gx_gpio_set_direction(6, GX_GPIO_DIRECTION_INPUT);
    gx_gpio_enable_trigger(6, GX_GPIO_TRIGGER_EDGE_FALLING, gpio_callback, NULL);
#endif

#ifdef CONFIG_UART1_AWAKE
    padmux_set(12 , 1);
    gx_gpio_set_direction(12, GX_GPIO_DIRECTION_INPUT);
    gx_gpio_enable_trigger(12, GX_GPIO_TRIGGER_EDGE_FALLING, gpio_callback, NULL);
#endif
    return 0;
 
}

static int VoiceControllerResume(void *priv)
{
    BoardSetUserPinMux();

    //printf("%s\n", (unsigned char *)priv);
    
#ifdef CONFIG_UART0_AWAKE
    padmux_set(6 , 0);
#endif

#ifdef CONFIG_UART1_AWAKE
    padmux_set(12 , 0);
#endif
    return 0;
}

static int VoiceControllerInit(void)
{
    if(!app_init) {
        //printf(LOG_TAG" ---- %s ----\n", __func__);

        VCMessageInit();
        LvpPmuSuspendLockCreate(&pmu_lock);

        app_init = 1;
    }

    return 0;
}

// App Event Process
static int VoiceControllerEventResponse(APP_EVENT *app_event)
{
    int event_id = app_event->event_id;
    if (event_id >= 100) {

        printf("[%s]%d kws: %d\n", __func__, __LINE__, event_id);

        if(event_id == 100) {
        //根据具体项目的event_id执行相应操作

            VCNewMessageNotify(event_id);

        } else if(event_id == 101) {
        //

            VCNewMessageNotify(event_id);

        } else {
        //
            VCNewMessageNotify(event_id);
            
        }
    }

    return 0;
}

static int VoiceControllerTaskLoop(void)
{
    if(VCMessageSessionPoll() > 0) {
        LvpPmuSuspendLock(pmu_lock);
    } else {
        LvpPmuSuspendUnlock(pmu_lock);
    }

    return 0;
}

LVP_APP voice_controller_v2_app = {
    .app_name = "voice controller v2",
    .AppInit = VoiceControllerInit,
    .AppEventResponse = VoiceControllerEventResponse,
    .AppTaskLoop = VoiceControllerTaskLoop,
    .AppSuspend = VoiceControllerSuspend,
    .suspend_priv = "VoiceControllerSuspend",
    .AppResume = VoiceControllerResume,
    .resume_priv = "VoiceControllerResume",
};

LVP_REGISTER_APP(voice_controller_v2_app);

