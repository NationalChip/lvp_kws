/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 *  vc_led.c
 *
 */

#include <autoconf.h>

#include <board_misc_config.h>
#include <gpio_led/gpio_led.h>
#include "vc_led.h"

static unsigned char led_enable = 1;

void PickupLedOn(void)
{
#ifdef CONFIG_APP_VC_HAS_LED
    if(!led_enable)
        return;

    GpioLedOn(CONFIG_APP_VC_LED_1_PIN_ID);
#endif
}

void PickupLedOff(void)
{
#ifdef CONFIG_APP_VC_HAS_LED
    if(!led_enable)
        return;
    GpioLedOff(CONFIG_APP_VC_LED_1_PIN_ID);
#endif
}

void KwsLedFlicker(unsigned short duration_ms, unsigned short interval_ms, unsigned char count)
{
#ifdef CONFIG_APP_VC_HAS_LED
    if(!led_enable)
        return;

    GpioLedFlicker(CONFIG_APP_VC_LED_2_PIN_ID, duration_ms, interval_ms, count);
#endif
}

void EnableGpioLed(void)
{
    led_enable = 1;
}

void DisableGpioLed(void)
{
    led_enable = 0;
}

