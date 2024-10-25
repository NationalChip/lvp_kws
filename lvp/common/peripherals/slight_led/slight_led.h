#pragma once

#include "../gpio_func.h"
enum SlightledMode
{
    SLIGHT_LED_MODE_OFF = 0,
    SLIGHT_LED_MODE_GEAR1,
    SLIGHT_LED_MODE_GEAR2,
    SLIGHT_LED_MODE_GEAR3,
    SLIGHT_LED_MODE_GEAR4,
    SLIGHT_LED_MODE_GEAR5,
    SLIGHT_LED_MODE_GEAR6,
    SLIGHT_LED_MODE_GEAR7,
    SLIGHT_LED_MODE_GEAR8,
    SLIGHT_LED_MODE_GEAR9,
    SLIGHT_LED_MODE_GEAR10,
    SLIGHT_LED_MODE_GEAR11,
    SLIGHT_LED_MODE_GEAR12,
    SLIGHT_LED_MODE_GEAR13,
    SLIGHT_LED_MODE_GEAR14,
    SLIGHT_LED_MODE_GEAR15,
    SLIGHT_LED_MODE_GEAR16,
    SLIGHT_LED_MODE_GEAR17,
    SLIGHT_LED_MODE_GEAR18,
    SLIGHT_LED_MODE_GEAR19,
    SLIGHT_LED_MODE_GEAR20,
    SLIGHT_LED_MODE_GEAR21,
    SLIGHT_LED_MODE_GEAR22,
    SLIGHT_LED_MODE_GEAR23,
    SLIGHT_LED_MODE_GEAR24,
    SLIGHT_LED_MODE_GEAR25,
    SLIGHT_LED_MODE_GEAR26,
    SLIGHT_LED_MODE_GEAR27,
    SLIGHT_LED_MODE_GEAR28,
    SLIGHT_LED_MODE_GEAR29,
    SLIGHT_LED_MODE_GEAR30,
};

enum SlightWorkMode
{
    SLIGHT_WORKMODE_NORMAL =0,
    SLIGHT_WORKMODE_WARM,
    SLIGHT_WORKMODE_COLD,
    SLIGHT_WORKMODE_NUM
};

int LvpSlightLedInit(unsigned char warm_pin, unsigned char cold_pin);
void LvpSlightLedDeInit(void);
void SetSlightLedMode(enum SlightledMode mode);
enum SlightledMode GetSlightLedMode(void);
void SetSlightWorkmode(enum SlightWorkMode mode);
enum SlightWorkMode GetSlightWorkMode(void);
void SetSlightLedBrightness(int per_thousand);
int GetSlightLedBrightness(void);
