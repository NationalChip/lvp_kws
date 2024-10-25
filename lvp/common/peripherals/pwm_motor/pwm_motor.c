#include "pwm_motor.h"
#include "../gpio_func.h"
#include <stdio.h>
#define FAN_MOTOR_PERIOD_NS 3000
#define FAN_MOTOR_DUTY_STEP1_NS 2500
#define FAN_MOTOR_DUTY_STEP2_NS 1500
#define FAN_MOTOR_DUTY_STEP3_NS 0
static char speed_pin_num = -1;
static char switch_pin_num = -1;
static void PWMMotorSwitchOn(void)
{
    if(switch_pin_num >= 0)
    {
        gx_gpio_set_direction(switch_pin_num, GX_GPIO_DIRECTION_OUTPUT);
        gx_gpio_set_level(switch_pin_num, GX_GPIO_LEVEL_HIGH);
    }
}

static void PWMMotorSwitchOff(void)
{
    if(switch_pin_num >= 0)
    {
        gx_gpio_set_direction(switch_pin_num, GX_GPIO_DIRECTION_OUTPUT);
        gx_gpio_set_level(switch_pin_num, GX_GPIO_LEVEL_LOW);
    }
}

void LvpPWMMotorModeControl(enum FanMotorMode mode)
{
    switch(mode)
    {
        case MOTOR_GEAR_1:
            PWMMotorSwitchOn();
            gx_gpio_enable_pwm(speed_pin_num, FAN_MOTOR_PERIOD_NS, FAN_MOTOR_DUTY_STEP1_NS);
            break;
        case MOTOR_GEAR_2:
            PWMMotorSwitchOn();
            gx_gpio_enable_pwm(speed_pin_num, FAN_MOTOR_PERIOD_NS, FAN_MOTOR_DUTY_STEP2_NS);
            break;
        case MOTOR_GEAR_3:
            PWMMotorSwitchOn();
            gx_gpio_enable_pwm(speed_pin_num, FAN_MOTOR_PERIOD_NS, FAN_MOTOR_DUTY_STEP3_NS);
            break;
        case MOTOR_OFF:
            PWMMotorSwitchOff();
            break;
        default:
            break;
    }
}

int LvpPWMMotorInit(unsigned char switch_pin, unsigned char speed_pin)
{
    if(switch_pin < 0 || speed_pin < 0)
        return -1;
    speed_pin_num = speed_pin;
    switch_pin_num = switch_pin;
    PWMMotorSwitchOff();
    return 0;
}

void LvpPWMMotorDeInit(void)
{
    if(speed_pin_num && switch_pin_num)
    {
        gx_gpio_disable_pwm(speed_pin_num);
        gx_gpio_disable_pwm(switch_pin_num);
        speed_pin_num = -1;
        switch_pin_num = -1;
    }
}
