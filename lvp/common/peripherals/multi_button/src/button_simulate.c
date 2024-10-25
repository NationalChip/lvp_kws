#include "button_simulate.h"
#include <driver/gx_timer.h>
#include <driver/gx_gpio.h>
#include <lvp_pmu.h>

static SIMULATE_BUTTON* head_handle = NULL;
#define TICKS_INTERVAL    5 //ms
#define SHORT_TICKS       (250 /TICKS_INTERVAL)
#define LONG_TICKS        (1000 /TICKS_INTERVAL)
#define MUTI_CLICK_SPACE_TICKS (200 /TICKS_INTERVAL)

static int LvpSimulateButtonInit(void);
static uint8_t _is_init;

static void SimulateSetPressLevel(uint8_t level, uint8_t gpio)
{
    gx_gpio_set_level(gpio, level);
}

int SimulateButtonInit(SIMULATE_BUTTON* handle, uint8_t gpio, uint8_t active_level, SimulateBtnCallback cb)
{
    if(!_is_init)
    {
        LvpSimulateButtonInit();
        _is_init = 1;
    }
    if(!handle)
        return -1;
    memset(handle, 0, sizeof(SIMULATE_BUTTON));
    handle->active_level = active_level;
    handle->gpio = gpio;
    handle->cb = cb;
    return 0;
}

void SimulateButtonHandler(SIMULATE_BUTTON* handle)
{
    if(handle->state == 12)
        return;
    handle->ticks++;

    if(handle->event == S_SINGLE_CLICK)
    {
        if(handle->state == 0) //PRESS DOWN
        {
            SimulateSetPressLevel(handle->active_level, handle->gpio);
            handle->state = 1;
        }
        else if( handle -> state == 1)
        {
            if(handle->ticks > SHORT_TICKS) //PRESS UP
            {
                SimulateSetPressLevel(!handle->active_level, handle->gpio);
                if(handle->cb)
                    handle->cb(handle->event);
                handle->state = 12;
            }
        }
    }
    else if(handle->event == S_DOUBLE_CLICK)
    {
        if(handle->state == 0)
        {
            SimulateSetPressLevel(handle->active_level, handle->gpio);
            handle->state = 1;
        }
        else if(handle -> state == 1)
        {
            if(handle->ticks > SHORT_TICKS) //PRESS UP
            {
                SimulateSetPressLevel(!handle->active_level, handle->gpio);
                handle->state = 2;
            }
        }
        else if(handle->state == 2)
        {
            if(handle->ticks > SHORT_TICKS + MUTI_CLICK_SPACE_TICKS) //
            {
                SimulateSetPressLevel(handle->active_level, handle->gpio);
                handle->state = 3;
            }
        }
        else if(handle->state == 3)
        {
            if(handle->ticks > SHORT_TICKS + MUTI_CLICK_SPACE_TICKS + SHORT_TICKS) //
            {
                SimulateSetPressLevel(!handle->active_level, handle->gpio);
                if(handle->cb)
                    handle->cb(handle->event);
                handle->state = 12;
            }
        }
    }
    else if(handle->event == S_TRIPLE_CLICK)
    {
        if(handle->state == 0)
        {
            SimulateSetPressLevel(handle->active_level, handle->gpio);
            handle->state = 1;
        }
        else if(handle->state == 1)
        {
            if(handle->ticks > SHORT_TICKS) //PRESS UP
            {
                SimulateSetPressLevel(!handle->active_level, handle->gpio);
                handle->state = 2;
            }
        }
        else if(handle->state == 2)
        {
            if(handle->ticks > SHORT_TICKS + MUTI_CLICK_SPACE_TICKS) //
            {
                SimulateSetPressLevel(handle->active_level, handle->gpio);
                handle->state = 3;
            }
        }
        else if(handle->state == 3)
        {
            if(handle->ticks > SHORT_TICKS + MUTI_CLICK_SPACE_TICKS + SHORT_TICKS) //
            {
                SimulateSetPressLevel(!handle->active_level, handle->gpio);
                handle->state = 4;
            }
        }
        else if(handle->state == 4)
        {
            if(handle->ticks > SHORT_TICKS + MUTI_CLICK_SPACE_TICKS + SHORT_TICKS + MUTI_CLICK_SPACE_TICKS) //
            {
                SimulateSetPressLevel(handle->active_level, handle->gpio);
                handle->state = 5;
            }
        }
        else if(handle->state == 5)
        {
            if(handle->ticks > SHORT_TICKS + MUTI_CLICK_SPACE_TICKS + SHORT_TICKS + MUTI_CLICK_SPACE_TICKS + SHORT_TICKS) //
            {
                SimulateSetPressLevel(!handle->active_level, handle->gpio);
                if(handle->cb)
                    handle->cb(handle->event);
                handle->state = 12;
            }
        }
    }
    if(handle->event == S_LONG_PRESS)
    {
        if(handle->state == 0) //PRESS DOWN
        {
            SimulateSetPressLevel(handle->active_level, handle->gpio);
            handle->state = 1;
        }
        else if( handle->state == 1)
        {
            if(handle->ticks > LONG_TICKS) //PRESS UP
            {
                SimulateSetPressLevel(!handle->active_level, handle->gpio);
                if(handle->cb)
                    handle->cb(handle->event);
                handle->state = 12;
            }
        }
    }
}

static void SimulateButtonTicks(void)
{
    SIMULATE_BUTTON* target;
    for(target=head_handle; target; target=target->next) {
        SimulateButtonHandler(target);
    }
}

int SimulateButtonStart(SIMULATE_BUTTON* handle, SIMULATE_EVENT event)
{
    handle->state = 0;
    handle->ticks = 0;
    handle->event = event;
    SIMULATE_BUTTON* target = head_handle;
    while(target) {
        if(target == handle)return -1; //already exist.
        target = target->next;
    }
    handle->next = head_handle;
    head_handle = handle;
    return 0;
}

void SimulateButtonDeinit(SIMULATE_BUTTON* handle)
{
    SIMULATE_BUTTON** curr;
    for(curr = &head_handle; *curr; ) {
        SIMULATE_BUTTON* entry = *curr;
        if (entry == handle) {
            *curr = entry->next;
//          free(entry);
        } else
            curr = &entry->next;
    }
}

static int SimulateButtonSuspend(void *arg)
{
    _is_init = 0;
    return 0;
}

static int LvpSimulateButtonInit(void)
{
    if(gx_timer_register((int(*)(void*))SimulateButtonTicks, TICKS_INTERVAL, NULL, GX_TIMER_MODE_CONTINUE))
    {
        return -1;
    }
    LVP_SUSPEND_INFO suspend_info = {
        .suspend_callback = SimulateButtonSuspend,
    };
    LvpSuspendInfoRegist(&suspend_info);
    return 0;
}

