/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 *vc_led.h
 *
 */

#ifndef __VC_LED_H__
#define __VC_LED_H__

void PickupLedOn(void);
void PickupLedOff(void);
void KwsLedFlicker(unsigned short duration_ms, unsigned short interval_ms, unsigned char count);

void EnableGpioLed(void);
void DisableGpioLed(void);

#endif /* __VC_LED_H__ */
