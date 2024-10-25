/* Grus
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mode.c: lvp mode
 *
 */

#include <autoconf.h>
#include <types.h>
#include <stdio.h>

#include "lvp_mode.h"

extern const LVP_MODE_INFO lvp_idle_mode_info;
extern const LVP_MODE_INFO lvp_tws_mode_info;
extern const LVP_MODE_INFO lvp_feed_mode_info;
extern const LVP_MODE_INFO lvp_record_mode_info;
extern const LVP_MODE_INFO lvp_denoise_mode_info;
extern const LVP_MODE_INFO lvp_nn_denoise_mode_info;
extern const LVP_MODE_INFO lvp_fft_recover_mode_info;
extern const LVP_MODE_INFO lvp_sed_mode_info;

static const LVP_MODE_INFO *s_lvp_mode_list[] = {
    &lvp_idle_mode_info,
#ifdef CONFIG_LVP_HAS_TWS_MODE
    &lvp_tws_mode_info,
#endif
#ifdef CONFIG_LVP_HAS_FEED_MODE
    &lvp_feed_mode_info,
#endif
#ifdef CONFIG_LVP_HAS_RECORD_MODE
    &lvp_record_mode_info,
#endif
#ifdef CONFIG_LVP_HAS_DENOISE_MODE
    &lvp_denoise_mode_info,
#endif
#ifdef CONFIG_LVP_HAS_NN_DENOISE_MODE
    &lvp_nn_denoise_mode_info,
#endif
#ifdef CONFIG_LVP_HAS_FFT_RECOVER_MODE
    &lvp_fft_recover_mode_info,
#endif
#ifdef CONFIG_LVP_HAS_SED_MODE
    &lvp_sed_mode_info,
#endif
};

static int s_current_index;
static int s_lvp_loop;

//=================================================================================================

LVP_MODE_TYPE LvpInitMode(LVP_MODE_TYPE type)
{
    s_lvp_loop = 1;
    s_current_index = 0;    // 0 Always is IDLE mode
    for (int i = 0; i < sizeof(s_lvp_mode_list) / sizeof(LVP_MODE_INFO *); i++) {
        if (s_lvp_mode_list[i]->type == type) {
            s_current_index = i;
            break;
        }
    }
    s_lvp_mode_list[s_current_index]->buffer_init();
    s_lvp_mode_list[s_current_index]->init(LVP_MODE_INIT_FLAG);
    return s_lvp_mode_list[s_current_index]->type;
}

LVP_MODE_TYPE LvpSwitchMode(LVP_MODE_TYPE type)
{
    if (s_lvp_mode_list[s_current_index]->type == type)
        return type;

    int required_index = -1;
    for (int i = 0; i < sizeof(s_lvp_mode_list) / sizeof(LVP_MODE_INFO *); i++) {
        if (s_lvp_mode_list[i]->type == type) {
            required_index = i;
            break;
        }
    }

    if (required_index != -1) {
        // Exit current mode
        s_lvp_mode_list[s_current_index]->done(type);

        // Init the required mode
        if (s_lvp_mode_list[required_index]->buffer_init() == 0 && s_lvp_mode_list[required_index]->init(s_lvp_mode_list[s_current_index]->type) == 0) {
            s_current_index = required_index;
        }
        else {  // Force to IDLE mode
            s_lvp_mode_list[s_current_index]->buffer_init();
            s_lvp_mode_list[s_current_index]->init(s_lvp_mode_list[s_current_index]->type);
            s_current_index = 0;
        }
    }

    return s_lvp_mode_list[s_current_index]->type;
}

LVP_MODE_TYPE LvpGetCurrentMode(void)
{
    return s_lvp_mode_list[s_current_index]->type;
}

//=================================================================================================

int LvpModeTick(void)
{
    if (s_lvp_mode_list[s_current_index]->tick)
        s_lvp_mode_list[s_current_index]->tick();

    return s_lvp_loop;
}

void LvpExit(void)
{
    s_lvp_mode_list[s_current_index]->done(LVP_MODE_IDLE);
    s_lvp_loop = 0;
}
