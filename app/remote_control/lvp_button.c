#include <autoconf.h>
#include <lvp_app.h>
#include <lvp_buffer.h>
#include <lvp_board.h>
#include <driver/gx_gpio.h>
#include <driver/gx_timer.h>
#include "flexible_button.h"
#include "lvp_button.h"

//#define GX_BUTTON_DEBUG

#ifdef GX_BUTTON_DEBUG
#define btn_debug(_fmt,args...) printf(_fmt,##args)
#else
#define btn_debug(_fmt,args...)
#endif
extern int button_up;
#if 0 // example
static GX_BUTTON_CONFIG btn_table[] = {
/*  按键对应的GPIO  按键名称  按下有效电平 持续长按  持续更长按  持续最长按*/
    {7,             "UP",    1,           1000,    1500,       3000},
};
#endif

#define MAX_BUTTON_NUM 13 // 8002最多13个GPIO口可用

static int interrupt_port = -1;
static lvp_common_btn_evt_cb btn_callback;
static int combination_button = 0; // 1 表示发现了有两个按键同时按下，组合键生效，目前仅支持两个按键的组合键
static int button_flag[MAX_BUTTON_NUM] = {-1}; // 做组合按键检测用

static void clear_combination_state(void)
{
    int i = 0;
    combination_button = 0;
    for (i = 0; i < MAX_BUTTON_NUM; i++)
    {
        button_flag[i] = -1;
    }
}

static void check_button(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;
    int i = 0;
    GX_BUTTON_STATE state;
    memset(&state, 0, sizeof(GX_BUTTON_STATE));

    if (btn->event == FLEX_BTN_PRESS_DOWN) // 按下事件发送
    {
        state.combination_button = 0;
        state.id_1 = btn->id;
        state.id_2 = 0;
        state.event = btn->event;
        btn_callback((void*)&state);
    }

    if (combination_button == 0)
    {
        if (btn->event == FLEX_BTN_PRESS_DOWN)
        {
            button_flag[btn->id] = FLEX_BTN_PRESS_DOWN; // 按键按下，该按键状态做个记录状态，如果在按键事件中，发现有两个按键都有记录状态，说明是组合按键

            int flag = 0;
            for (i = 0; i < MAX_BUTTON_NUM; i++)
            {
                if (button_flag[i] != -1)
                {
                    flag++;
                }
                if (flag == 2)
                {
                    // 一但有两个按键同时按下，说明是组合按键了，除非该组合按键结束，否则后续其它按键不在做组合按键的判断了
                    combination_button = 1;
                    {
                        if (button_flag[btn->id] != -1) // 该按键属于组合按键里的
                        {
                            button_flag[btn->id] = btn->event;

                            for (i = 0; i < MAX_BUTTON_NUM; i++)
                            {
                                if ((i != btn->id) && (button_flag[i] == button_flag[btn->id])) // 组合的两个按键，处于同一事件下
                                {
                                    state.combination_button = 1;
                                    state.id_1 = btn->id;
                                    state.id_2 = i;
                                    state.event = btn->event;
                                    btn_callback((void*)&state);
                                    return;
                                 }
                            }
                        }
                    }
                    return;
                }
            }
        }
    }

    if (combination_button == 0) // 单按键
    {
        if (btn->event != FLEX_BTN_PRESS_DOWN)
        {
            state.combination_button = 0;
            state.id_1 = btn->id;
            state.id_2 = 0;
            state.event = btn->event;
            btn_callback((void*)&state);
        }

        if ((btn->event == FLEX_BTN_PRESS_CLICK) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) || (btn->event == FLEX_BTN_PRESS_SHORT_UP) || (btn->event == FLEX_BTN_PRESS_LONG_UP) || (btn->event == FLEX_BTN_PRESS_LONG_HOLD_UP) || (btn->event == FLEX_BTN_PRESS_REPEAT_CLICK))
        {
            button_flag[btn->id] = -1; // 按键弹起，恢复判断combination的初始状态
        }
        return;
    }
    else // 组合按键
    {
        if (btn->event == FLEX_BTN_PRESS_DOWN) return;

        if (button_flag[btn->id] == -1) // 该按键不属于组合按键里的。比如三个或者以上的按键同时按下，其中两个作为组合，其它的作为独立按键判断
        {
            state.combination_button = 0;
            state.id_1 = btn->id;
            state.id_2 = 0;
            state.event = btn->event;
            btn_callback((void*)&state);
            return;
        }

        if (button_flag[btn->id] != -1) // 该按键属于组合按键里的
        {
            button_flag[btn->id] = btn->event;

            for (i = 0; i < MAX_BUTTON_NUM; i++)
            {
                if ((i != btn->id) && (button_flag[i] == button_flag[btn->id])) // 组合的两个按键，处于同一事件下
                {
                    state.combination_button = 1;
                    state.id_1 = btn->id;
                    state.id_2 = i;
                    state.event = btn->event;
                    btn_callback((void*)&state);
                    break;
                }
            }
        }

        if ((btn->event == FLEX_BTN_PRESS_CLICK) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) || (btn->event == FLEX_BTN_PRESS_SHORT_UP) || (btn->event == FLEX_BTN_PRESS_LONG_UP) || (btn->event == FLEX_BTN_PRESS_LONG_HOLD_UP) || (btn->event == FLEX_BTN_PRESS_REPEAT_CLICK))
        {
            // button_up = 1;
            /* 组合按键事件上报 */
            if((btn->event == FLEX_BTN_PRESS_CLICK) || (btn->event == FLEX_BTN_PRESS_DOUBLE_CLICK) || (btn->event == FLEX_BTN_PRESS_REPEAT_CLICK)) {
                state.event = FLEX_BTN_PRESS_SHORT_UP;
            } else {
                 state.event = btn->event;
            }
            state.combination_button = 1;
            state.id_1 = btn->id;
            state.id_2 = 0;

            btn_callback((void*)&state);

        /*
            printf("\n");
            for (i = 0; i < USER_BUTTON_NUM; i++)
            {
                printf("%d = %d, ", i, button_flag[i]);
            }
            printf("\n");*/

            button_flag[btn->id] = -1; // 按键弹起，恢复判断combination的初始状态

            int flag = 0;
            for (i = 0; i < MAX_BUTTON_NUM; i++)
            {
                if (button_flag[i] != -1)
                {
                    flag = 1;
                }
            }

            if (flag == 0) // 全部事件都是-1了，就去掉组合按键的标识
            {
                combination_button = 0;
            }

        }
    }
}

static void common_btn_evt_cb(void *arg)
{
#if 0
    flex_button_t *btn = (flex_button_t *)arg;

    btn_debug("id: [%d - ]  event: [%d - %30s]  repeat: %d\n",
        btn->id,
        btn->event, enum_event_string[btn->event],
        btn->click_cnt);
#endif
    check_button(arg);
}

static uint8_t common_btn_read(void *arg)
{
    uint8_t value = 0;

    flex_button_t *btn = (flex_button_t *)arg;

    if (interrupt_port != -1)
    {
        if (btn->id == interrupt_port)
        {
            value = btn->pressed_logic_level; // 低功耗下，按键中断恢复后，可能在程序运行的时候，按键已经松开了，因此要强制模拟次按下的动作
            interrupt_port = -1;
        }
        else
        {
            value = gx_gpio_get_level(btn->id);
        }
    }
    else
    {
        value = gx_gpio_get_level(btn->id);
    }

    return value;
}

static int gpioInterruptCallBack(int port,void *arg)
{
    btn_debug("Interrupt! port = %d\n", port);

    // 测试发现，无论如何多快按下，都可以保留中断触发，在8002待机恢复后进入到中断里。因此中断里重新进行按键检测，可以避免丢失按键消息
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    interrupt_port = port;
    flex_button_scan();
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return 0;
}

static int timerInterruptCallback(void *arg)
{
    flex_button_scan();
    return 0;
}

static int btn_init = 0;

void lvp_button_init(GX_BUTTON_CONFIG *btn_table, int size, lvp_common_btn_evt_cb callback)
{
    if (btn_init == 0)
    {
        int table_size = size;
        btn_debug("table_size = %d\n", table_size);

        clear_combination_state();

        int i = 0;
        for (i = 0; i < table_size; i++)
        {
            //复用管脚为GPIO
            if (btn_table[i].pin_id == 2)
            {
                padmux_set(btn_table[i].pin_id, 0);
            }
            else
            {
                padmux_set(btn_table[i].pin_id, 1);
            }

            //设置端口方向为输入
            gx_gpio_set_direction(btn_table[i].pin_id, GX_GPIO_DIRECTION_INPUT);

            btn_table[i].button.id = btn_table[i].pin_id;
            btn_table[i].button.short_press_start_tick = FLEX_MS_TO_SCAN_CNT(btn_table[i].short_press_start_tick);
            btn_table[i].button.long_press_start_tick = FLEX_MS_TO_SCAN_CNT(btn_table[i].long_press_start_tick);
            btn_table[i].button.long_hold_start_tick = FLEX_MS_TO_SCAN_CNT(btn_table[i].long_hold_start_tick);
            btn_table[i].button.pressed_logic_level = btn_table[i].pressed_logic_level;
            btn_table[i].button.usr_button_read = common_btn_read;
            btn_table[i].button.cb = common_btn_evt_cb;

            flex_button_register(&btn_table[i].button);
        }

        //初始化中断回调函数，中断只是作为触发信号
        for(i = 0; i < table_size; i++)
        {
            if (btn_table[i].pressed_logic_level == 1)
            {
                gx_gpio_enable_trigger(btn_table[i].pin_id, GX_GPIO_TRIGGER_EDGE_RISING, gpioInterruptCallBack, NULL);
            }
            else
            {
                gx_gpio_enable_trigger(btn_table[i].pin_id, GX_GPIO_TRIGGER_EDGE_FALLING, gpioInterruptCallBack, NULL);
            }
        }

        btn_callback = callback;
        btn_init = 1;
    }

    if(gx_timer_register((int(*)(void*))timerInterruptCallback, 20, NULL, GX_TIMER_MODE_CONTINUE))
    {
        btn_debug("gx_timer_register fail\n");
    }
}

int lvp_button_check_if_press(GX_BUTTON_CONFIG *btn_table, int size)
{
    int i = 0;
    for (i = 0; i < size; i++)
    {
        uint8_t value = 0;
        value = gx_gpio_get_level(btn_table[i].pin_id);
        if (btn_table[i].pressed_logic_level == value)
        {
            return 1; // 代表有某个按键被按下中
        }
    }

    return 0; // 无按键被按下
}





