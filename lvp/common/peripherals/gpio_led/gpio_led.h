/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app.h
 *
 */

#ifndef __GPIO_LED_H__
#define __GPIO_LED_H__

void GpioLedOn(unsigned char led_gpio);
void GpioLedOff(unsigned char led_gpio);
void GpioLedFlicker(unsigned char led_gpio, unsigned short duration_ms, unsigned short interval_ms, unsigned char count);

#endif /* __GPIO_LED_H__ */

