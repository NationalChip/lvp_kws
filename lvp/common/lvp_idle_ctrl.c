/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_idle_ctrl.c:
 *
 */

#include <autoconf.h>
#include <stdio.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_audio_in.h>
#include <driver/gx_snpu.h>
#include <driver/gx_irq.h>

#include <lvp_audio_in.h>
#include <lvp_app_core.h>
#include <lvp_pmu.h>
#include <lvp_queue.h>

#include <driver/gx_rtc.h>
#include <driver/gx_rtc.h>
#include <driver/gx_timer.h>
#include <driver/misc.h>
#include <driver/gx_delay.h>

#define LOG_TAG "[IDLE_CTRL]"

static struct {
    unsigned int priv_tick_time;
} s_lic_handle;

int LvpIdleCtrl(void)
{
    unsigned int cur_tick_time = gx_get_time_us();

#if 0
    unsigned int average_tick_cycle;
    average_tick_cycle = average_tick_cycle * 8 / 10 + (cur_tick_time - s_lic_handle.priv_tick_time) * 2 / 10;
    printf(LOG_TAG"t %dus\n", s_lic_handle.average_tick_cycle);
#endif

    unsigned int tick_continued_time = cur_tick_time - s_lic_handle.priv_tick_time;

    if (tick_continued_time < CONFIG_LVP_IDLE_CTRL_THRESHOLD) {
        for (int i = 0; i < 18; i ++) gx_mcu_set_wakeup_source(i);

//        printf("****Enter IDLE\n");
        gx_mcu_idle();
//        printf("****Exit IDLE\n");

        for (int i = 0; i < 18; i ++) gx_mcu_clr_wakeup_source(i);
    }

    s_lic_handle.priv_tick_time = gx_get_time_us();
    return 0;
}


