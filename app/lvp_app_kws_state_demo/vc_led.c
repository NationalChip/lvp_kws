/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 *  vc_led.c
 *
 */

#include <autoconf.h>

#include <driver/gx_gpio.h>

#include <board_misc_config.h>
#include <gpio_led/gpio_led.h>
#include "vc_led.h"

static unsigned char led_enable = 1;

void KwsLedFlicker(unsigned short duration_ms, unsigned short interval_ms, unsigned char count)
{
#ifdef CONFIG_APP_VC_HAS_LED
    if(!led_enable)
        return;
    gx_gpio_set_direction(CONFIG_APP_VC_KWS_LED_PIN_ID, GX_GPIO_DIRECTION_OUTPUT);
    GpioLedFlicker(CONFIG_APP_VC_KWS_LED_PIN_ID, duration_ms, interval_ms, count);
    gx_gpio_set_direction(CONFIG_APP_VC_KWS_LED_PIN_ID, GX_GPIO_DIRECTION_INPUT);

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

