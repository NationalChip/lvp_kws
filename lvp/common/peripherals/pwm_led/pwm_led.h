#pragma once

#include "../gpio_func.h"
enum FanledMode
{
    LED_MODE_OFF = 0,
    LED_MODE_BREATH,
    LED_MODE_WHITE,
};
int LvpPWMLedInit(unsigned char pin);
void LvpPWMLedDeInit(void);
void LvpPWMLedModeControl(enum FanledMode mode);
