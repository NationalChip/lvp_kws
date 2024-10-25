/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * gpio_func.h:
 *
 */

#ifndef __GPIO_FUNC_H__
#define __GPIO_FUNC_H__

#include <driver/gx_gpio.h>
#include <driver/gx_padmux.h>

typedef struct {
    unsigned char pin_id;
    unsigned char gpio_sel;
    unsigned char default_sel;
}GPIO_FUNC_PIN;

static inline void EnablePinGpioFunc(GPIO_FUNC_PIN *gpio_pin) {
    padmux_set(gpio_pin->pin_id, gpio_pin->gpio_sel);
}

static inline void DisablePinGpioFunc(GPIO_FUNC_PIN *gpio_pin) {
    padmux_set(gpio_pin->pin_id, gpio_pin->default_sel);
}

#endif /* __GPIO_FUNC_H__ */
