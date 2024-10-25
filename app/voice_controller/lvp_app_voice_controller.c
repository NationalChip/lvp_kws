/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app_voice_controller.c
 *
 */

#include <lvp_app.h>
#include <lvp_buffer.h>
#include "vc_led.h"
#include "vc_message.h"
#include <lvp_pmu.h>
#include <lvp_board.h>
#include <lvp_system_init.h>
#include <multi_button/src/multi_button.h>
#include <driver/gx_gpio.h>
#include <autoconf.h>

#define LOG_TAG "[VOICE_CONTROLLER]"

static int pmu_lock = 0;
static char app_init = 0;
//=================================================================================================

static char power_save_mode = 0;

static void leavePowSaveMode(void)
{
    BoardSetUserPinMux();
    power_save_mode = 0;
}

static void enterPowSaveMode(void)
{
    BoardSetPowerSavePinMux();
    power_save_mode = 1;
}

static int VoiceControllerSuspend(void *priv)
{
    PickupLedOff();

    //printf("%s\n", (unsigned char *)priv);

    return 0;
}

static int VoiceControllerResume(void *priv)
{
    if(power_save_mode == 1) {
        enterPowSaveMode();
    }

    PickupLedOn();

    //printf("%s\n", (unsigned char *)priv);

    return 0;
}
#ifdef CONFIG_BOARD_SUPPORT_MULTIBOOT
static Button denoise_toggle_key;
static unsigned char _readToggleKeyValue(void)
{
    unsigned char ret = gx_gpio_get_level(2);
    return ret;
}

static void _toggleKeySingleClickHandler(void *btn)
{
    SwitchAnotherFirmeware();
}

static void _toggleKeyInit(void)
{
    gx_gpio_set_direction(2, GX_GPIO_DIRECTION_INPUT);
    button_init(&denoise_toggle_key, _readToggleKeyValue, 0);
    button_attach(&denoise_toggle_key, SINGLE_CLICK, _toggleKeySingleClickHandler);
    button_start(&denoise_toggle_key);
    LvpButtonInit();
}
#endif

static int VoiceControllerInit(void)
{
#ifdef CONFIG_BOARD_SUPPORT_MULTIBOOT
    _toggleKeyInit();
#endif
    if(!app_init) {
        //printf(LOG_TAG" ---- %s ----\n", __func__);
        PickupLedOn();

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
        VCNewMessageNotify(event_id);
/*
        if(event_id == 107) {
            leavePowSaveMode();
            //open led
            EnableGpioLed();
            PickupLedOn();
            KwsLedFlicker(800, 0, 1);
        } else if(event_id == 108) {
            enterPowSaveMode();
            //close led
            KwsLedFlicker(150, 150, 2);
            PickupLedOff();
            DisableGpioLed();
        } else {
            VCNewMessageNotify(event_id);
            KwsLedFlicker(300, 0, 1);
        }*/
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

LVP_APP voice_controller_app = {
    .app_name = "voice controller",
    .AppInit = VoiceControllerInit,
    .AppEventResponse = VoiceControllerEventResponse,
    .AppTaskLoop = VoiceControllerTaskLoop,
    .AppSuspend = VoiceControllerSuspend,
    .suspend_priv = "VoiceControllerSuspend",
    .AppResume = VoiceControllerResume,
    .resume_priv = "VoiceControllerResume",
};

LVP_REGISTER_APP(voice_controller_app);

