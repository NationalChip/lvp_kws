/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * gpio_led.c
 *
 */

#include <driver/gx_timer.h>
#include <driver/gx_padmux.h>
#include <driver/gx_gpio.h>
#include <driver/gx_delay.h>

#include "gpio_led.h"

void GpioLedOn(unsigned char led_gpio)
{
    gx_gpio_set_level(led_gpio, GX_GPIO_LEVEL_LOW);
}

void GpioLedOff(unsigned char led_gpio)
{
    gx_gpio_set_level(led_gpio, GX_GPIO_LEVEL_HIGH);
}

void GpioLedFlicker(unsigned char led_gpio, unsigned short duration_ms, unsigned short interval_ms, unsigned char count)
{
    unsigned char i;

    for(i = 0; i < count; i++) {
        GpioLedOn(led_gpio);
        gx_mdelay(duration_ms);
        GpioLedOff(led_gpio);

        if(i == count - 1)
            break;

        gx_mdelay(interval_ms);
    }
}

