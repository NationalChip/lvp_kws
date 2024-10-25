#pragma once

#include "stdint.h"
#include "string.h"
#include <stdio.h>

typedef enum {
    S_SINGLE_CLICK = 0,
    S_DOUBLE_CLICK,
    S_TRIPLE_CLICK,
    S_LONG_PRESS,
}SIMULATE_EVENT;

typedef void (*SimulateBtnCallback)(SIMULATE_EVENT event);

typedef struct SIMULATE_BUTTON {
    uint16_t ticks;
    uint8_t state:4;
    uint8_t active_level:1;
    SIMULATE_EVENT event:3;
    uint8_t gpio;
    SimulateBtnCallback cb;
    struct SIMULATE_BUTTON *next;
}SIMULATE_BUTTON;

int SimulateButtonInit(SIMULATE_BUTTON* handle, uint8_t gpio, uint8_t active_level, SimulateBtnCallback cb);
int SimulateButtonStart(SIMULATE_BUTTON* handle, SIMULATE_EVENT event);
void SimulateButtonDeinit(SIMULATE_BUTTON* handle);

