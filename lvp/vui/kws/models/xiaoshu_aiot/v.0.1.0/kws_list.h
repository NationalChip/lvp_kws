/* Grus
 * Copyright (C) 1991-2024 Nationalchip Co., Ltd
 *
 * kws_list.h: a Circular Queue using array
 *
 */

#ifndef __KWS_LIST_H__
#define __KWS_LIST_H__

#include <lvp_param.h>

// 注： 阈值已经在此处固定，更改阈值请手动更改此文件

LVP_KWS_PARAM g_kws_param_list[] = {
    {"Hello_Tina",      {0}, 1, 374, 100, 1},
    {"Press_OK",        {0}, 1, 249, 100, 1},
    {"Press_exit",      {0}, 1, 127, 100, 1},
    {"channel_down",    {0}, 1, 455, 100, 1},
    {"channel_up",      {0}, 1, 703, 100, 1},
    {"downward",        {0}, 1, 463, 100, 1},
    {"enter_EPG",       {0}, 1, 192, 100, 1},
    {"enter_menu",      {0}, 1, 288, 100, 1},
    {"enter_standby",   {0}, 1, 118, 100, 1},
    {"power_on",        {0}, 1, 438, 100, 1},
    {"set_mute",        {0}, 1, 289, 100, 1},
    {"show_favorite",   {0}, 1, 70, 100, 1},
    {"show_inform",     {0}, 1, 503, 100, 1},
    {"switch_TV",       {0}, 1, 28, 100, 1},
    {"switch_radio",    {0}, 1, 92, 100, 1},
    {"turn_left",       {0}, 1, 354, 100, 1},
    {"turn_right",      {0}, 1, 236, 100, 1},
    {"upward",          {0}, 1, 173, 100, 1},
    {"volume_down",     {0}, 1, 561, 100, 1},
    {"volume_up",       {0}, 1, 722, 100, 1},
};

#endif /* __KWS_LIST_H__ */
