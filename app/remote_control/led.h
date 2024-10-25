#ifndef LED_H_
#define LED_H_

#include "lvp_app_common.h"
#include <driver/gx_gpio.h>
#include <driver/gx_delay.h>

void ledOn(void);
void ledOff(void);
void ledBright(const int delay_ms);

#endif