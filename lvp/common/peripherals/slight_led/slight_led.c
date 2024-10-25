#include "../gpio_func.h"
#include "slight_led.h"
#include <stdio.h>
#include <board_misc_config.h>

#if 0
#define SLIGHT_LED_PERIOD_NS 1000000
#define SLIGHT_LED_DUTY_OFF_NS 0

#define _5V_1A_POWER 5
#define _5V_2A_POWER 10
#define LED_MAX_POWER 12
#define MAX_LED_DUTY_NS (SLIGHT_LED_PERIOD_NS * (_5V_1A_POWER + _5V_2A_POWER)/(LED_MAX_POWER*2) )
// #define MAX_LED_DUTY_NS 625000

#define MIN_LED_DUTY_NS (MAX_LED_DUTY_NS/SLIGHT_LED_MODE_GEAR30)
// #define MIN_LED_DUTY_NS 62500


#define MAX_CHANGED_DUTY_NS MAX_LED_DUTY_NS
// #define MAX_LED_DUTY_NS 1000000
#else

#define SLIGHT_LED_PERIOD_NS 1000000
#define SLIGHT_LED_DUTY_OFF_NS 0
#define MIN_LED_DUTY_NS 10000
#define MAX_LED_DUTY_NS 900000
#define MAX_CHANGED_DUTY_NS MAX_LED_DUTY_NS
#endif


static char warm_led_pin = -1;
static char cold_led_pin = -1;
static enum SlightledMode led_mode = SLIGHT_LED_MODE_OFF;
static enum SlightWorkMode work_mode = SLIGHT_WORKMODE_NORMAL;
static int brightness_perthousand = 0;

int LvpSlightLedInit(unsigned char warm_pin, unsigned char cold_pin)
{
    if(warm_pin < 0 || cold_pin < 0)
        return -1;

    warm_led_pin = warm_pin;
    cold_led_pin = cold_pin;

    work_mode = SLIGHT_WORKMODE_WARM;
    led_mode = SLIGHT_LED_MODE_OFF;
    SetSlightLedMode(SLIGHT_LED_MODE_OFF);
    // printf("MAX_CHANGED_DUTY_NS =%d MAX_LED_DUTY_NS2=%d\n", MAX_CHANGED_DUTY_NS, MAX_LED_DUTY_NS2);
    return 0;
}

void LvpSlightLedDeInit(void)
{
    if(warm_led_pin >= 0)
    {
        warm_led_pin = -1;
    }
    if(cold_led_pin >= 0)
    {
        cold_led_pin = -1;
    }
}

void SetSlightLedMode(enum SlightledMode mode)
{
    unsigned char is_warm_disable = 0, is_cold_disable = 0;
    int duty_ns;
    if(work_mode == SLIGHT_WORKMODE_WARM)
    {
        is_cold_disable = 1;
        gx_gpio_enable_pwm(cold_led_pin, SLIGHT_LED_PERIOD_NS, SLIGHT_LED_DUTY_OFF_NS);
        // gx_gpio_set_direction(cold_led_pin, GX_GPIO_DIRECTION_OUTPUT);
        // gx_gpio_set_level(cold_led_pin, GX_GPIO_LEVEL_LOW);
    }
    else if(work_mode == SLIGHT_WORKMODE_COLD)
    {
        is_warm_disable = 1;
        gx_gpio_enable_pwm(warm_led_pin, SLIGHT_LED_PERIOD_NS, SLIGHT_LED_DUTY_OFF_NS);
    }
    brightness_perthousand = 1000*mode/SLIGHT_LED_MODE_GEAR30;
    if(mode)
    {
        duty_ns = mode *MAX_CHANGED_DUTY_NS / SLIGHT_LED_MODE_GEAR30;
        duty_ns = (duty_ns > MIN_LED_DUTY_NS)?duty_ns:MIN_LED_DUTY_NS;
        if(mode == SLIGHT_LED_MODE_GEAR30)
            duty_ns = MAX_LED_DUTY_NS;
    }
    else
    {
        led_mode = SLIGHT_LED_MODE_OFF;
        duty_ns = 0;
    }
    printf("%s %d duty_ns =%d mode=%d brightness_perthousand=%d\n", __func__,__LINE__, duty_ns, mode, brightness_perthousand);
    if(!is_warm_disable)
    {
        gx_gpio_enable_pwm(warm_led_pin, SLIGHT_LED_PERIOD_NS, duty_ns);
    }
    if(!is_cold_disable)
    {
        gx_gpio_enable_pwm(cold_led_pin, SLIGHT_LED_PERIOD_NS, duty_ns);
    }
    led_mode = mode;
}

void SetSlightLedBrightness(int per_thousand)//1~1000
{
    int brightness;
    int per_real = per_thousand;
    if(led_mode == SLIGHT_LED_MODE_OFF)
    {
        return;
    }
    if(per_thousand <= 1000)
    {
        brightness = per_thousand * MAX_CHANGED_DUTY_NS /1000;
        brightness = (brightness >= MIN_LED_DUTY_NS)? brightness:MIN_LED_DUTY_NS;
    }
    else if(per_thousand >=1000)
    {
        per_real = 1000;
        brightness = MAX_CHANGED_DUTY_NS;
    }
    // printf("brightness ========%d\n", brightness);
    if(work_mode == SLIGHT_WORKMODE_WARM)
    {
        gx_gpio_enable_pwm(cold_led_pin, SLIGHT_LED_PERIOD_NS, SLIGHT_LED_DUTY_OFF_NS);
        gx_gpio_enable_pwm(warm_led_pin, SLIGHT_LED_PERIOD_NS, brightness);
    }
    else if(work_mode == SLIGHT_WORKMODE_COLD)
    {
        gx_gpio_enable_pwm(cold_led_pin, SLIGHT_LED_PERIOD_NS, brightness);
        gx_gpio_enable_pwm(warm_led_pin, SLIGHT_LED_PERIOD_NS, SLIGHT_LED_DUTY_OFF_NS);
    }
    else if(work_mode == SLIGHT_WORKMODE_NORMAL)
    {
        gx_gpio_enable_pwm(cold_led_pin, SLIGHT_LED_PERIOD_NS, brightness);
        gx_gpio_enable_pwm(warm_led_pin, SLIGHT_LED_PERIOD_NS, brightness);
    }
    // printf("=====brightness = %d per_real=%d\n", brightness, per_real);
    led_mode = per_thousand/33;
    led_mode = (led_mode > SLIGHT_LED_MODE_GEAR1)? led_mode:SLIGHT_LED_MODE_GEAR1;
    led_mode = (led_mode > SLIGHT_LED_MODE_GEAR30)? SLIGHT_LED_MODE_GEAR30:led_mode;
    // printf("led_mode =%d brightness =%d\n", led_mode, brightness);
    brightness_perthousand = per_real;
}

int GetSlightLedBrightness()
{
    return brightness_perthousand;
}

enum SlightledMode GetSlightLedMode(void)
{
    return led_mode;
}

void SetSlightWorkmode(enum SlightWorkMode mode)
{
    work_mode = mode;
    SetSlightLedMode(led_mode);
}

enum SlightWorkMode GetSlightWorkMode(void)
{
    return work_mode;
}
