#pragma once
#include "../gpio_func.h"
enum FanMotorMode {
    MOTOR_GEAR_1 = 0,
    MOTOR_GEAR_2,
    MOTOR_GEAR_3,
    MOTOR_OFF,
};

void LvpPWMMotorModeControl(enum FanMotorMode mode);
int LvpPWMMotorInit(unsigned char switch_pin, unsigned char speed_pin);
void LvpPWMMotorDeInit(void);