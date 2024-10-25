#include "../gpio_func.h"
#include "pwm_led.h"
#include <stdio.h>
#define FAN_LED_PERIOD_NS 1000000
#define FAN_LED_DUTY_OFF_NS 0
#define FAN_LED_DUTY_BREATH_NS 1000000
#define FAN_LED_DUTY_WHITE_NS 900000

static char led_pin = -1;
int LvpPWMLedInit(unsigned char pin)
{
    if(pin < 0)
        return -1;
    led_pin = pin;
    LvpPWMLedModeControl(LED_MODE_OFF);
    return 0;
}

void LvpPWMLedDeInit(void)
{
    if(led_pin >= 0)
    {
        gx_gpio_set_direction(led_pin, GX_GPIO_DIRECTION_OUTPUT);
        gx_gpio_set_level(led_pin, GX_GPIO_LEVEL_LOW);
    }
}

void LvpPWMLedModeControl(enum FanledMode mode)
{
    switch(mode)
    {
        case LED_MODE_OFF:
            gx_gpio_enable_pwm(led_pin, FAN_LED_PERIOD_NS, FAN_LED_DUTY_OFF_NS);
            break;
        case LED_MODE_BREATH:
            gx_gpio_enable_pwm(led_pin, FAN_LED_PERIOD_NS, FAN_LED_DUTY_BREATH_NS);
            break;
        case LED_MODE_WHITE:
            gx_gpio_enable_pwm(led_pin, FAN_LED_PERIOD_NS, FAN_LED_DUTY_WHITE_NS);
            break;
        default:
            break;
    }
}