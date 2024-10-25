#ifndef __LVP_BUTTON_H__
#define __LVP_BUTTON_H__

#include "flexible_button.h"

#define MAX_BUTTON_TIME (1000*1000) // 一千秒，可以认为没有用户会持续按那么久的了

typedef struct button_config {
    unsigned char pin_id;                   // GPIO
	const char *btn_name;                // button name
	unsigned char pressed_logic_level;      // 设置按键按下的逻辑电平。1：标识按键按下的时候为高电平；0：标识按键按下的时候未低电平，**重要**
    unsigned short short_press_start_tick;  // 设置短按事件触发的起始 tick 单位毫秒
    unsigned short long_press_start_tick;   // 设置长按事件触发的起始 tick 单位毫秒
    unsigned short long_hold_start_tick;    // 设置长按保持事件触发的起始 tick 单位毫秒
    flex_button_t button;
} GX_BUTTON_CONFIG;

typedef struct button_state {
    int combination_button; // 如果为1，代表是组合按键（目前仅支持两个按键组合）。那么下面的id_1和id_2都有效。如果为0，代表单按键，仅id_1有效
    unsigned char id_1;
    unsigned char id_2;
    unsigned char event; // CLICK, SHORT_START, LONG_START, LONG_HOLD_UP
}GX_BUTTON_STATE;
typedef void (*lvp_common_btn_evt_cb)(void *arg);

void lvp_button_init(GX_BUTTON_CONFIG *btn_table, int size, lvp_common_btn_evt_cb callback);
int lvp_button_check_if_press(GX_BUTTON_CONFIG *btn_table, int size);

#endif /*__LVP_BUTTON_H__  */
