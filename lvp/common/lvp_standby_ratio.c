#include <autoconf.h>
#include <string.h>
#include <stdio.h>
#include <types.h>

#include <driver/gx_rtc.h>
#include <driver/gx_pmu_ctrl.h>

#include <lvp_context.h>
#include <lvp_attr.h>
#include <lvp_buffer.h>
#include <lvp_param.h>
#include <lvp_board.h>
#include <lvp_pmu.h>
#include "lvp_standby_ratio.h"

#define LOG_TAG "[RATIO]"

typedef struct {
    unsigned char system_status[CONFIG_CACULATE_STANDBY_RATIO_WINDOW_LENGTH]; // 0 -->> standby; 1 -->> wake
    unsigned int  system_status_index;
    unsigned long start_time;

    unsigned long running_time;
    unsigned long standby_time;
} STANDBY_RATIO_HANDLE;
static STANDBY_RATIO_HANDLE s_standby_ratio;

static void _RecordSystemStatus(unsigned long start, unsigned long end, unsigned char status)
{
    unsigned long druation = end - start;

    if (status) {
        s_standby_ratio.running_time += druation;
    } else {
        s_standby_ratio.standby_time += druation;
    }

    // printf (LOG_TAG"## idx:%d t:%d d: %d s:%d; ", system_status_index, end, druation, status);
    if (druation >= CONFIG_CACULATE_STANDBY_RATIO_WINDOW_LENGTH) {
        for (int i = 0; i < CONFIG_CACULATE_STANDBY_RATIO_WINDOW_LENGTH; i++) {
            s_standby_ratio.system_status[i] = status;
        }
        s_standby_ratio.system_status_index = 0;
    } else {
        if (s_standby_ratio.system_status_index + druation < CONFIG_CACULATE_STANDBY_RATIO_WINDOW_LENGTH) {
            for (int i = s_standby_ratio.system_status_index; i < s_standby_ratio.system_status_index + druation; i++) {
                s_standby_ratio.system_status[i] = status;
            }
        } else {
            int remain = s_standby_ratio.system_status_index + druation - CONFIG_CACULATE_STANDBY_RATIO_WINDOW_LENGTH;

            for (int i = s_standby_ratio.system_status_index; i < CONFIG_CACULATE_STANDBY_RATIO_WINDOW_LENGTH; i++) {
                s_standby_ratio.system_status[i] = status;
            }
            for (int i = 0; i < remain; i++) {
                s_standby_ratio.system_status[i] = status;
            }
        }

        s_standby_ratio.system_status_index = (s_standby_ratio.system_status_index+druation)%CONFIG_CACULATE_STANDBY_RATIO_WINDOW_LENGTH;
    }
}

int LvpStandbyRatioSuspend(void *priv)
{
    gx_rtc_get_tick(&s_standby_ratio.start_time);
    // printf (LOG_TAG"start_time: %d\n", start_time);

    return 0;
}


int LvpStandbyRatioResume(void *priv)
{
    unsigned long time;
    gx_rtc_get_tick(&time);
    _RecordSystemStatus(s_standby_ratio.start_time, time, 0);
    s_standby_ratio.start_time = time;

    printf (LOG_TAG"standby_time: %d, total_time: %d, average:%d%%\n", s_standby_ratio.standby_time, s_standby_ratio.standby_time + s_standby_ratio.running_time, LvpCountAverageStandbyRatio());

    return 0;
}

void LvpStandbyRatioInit(void)
{
    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    if (start_mode == GX_WAKEUP_SOURCE_COLD || start_mode == GX_WAKEUP_SOURCE_WDT) {
        memset(&s_standby_ratio, 0, sizeof(STANDBY_RATIO_HANDLE));

        LVP_SUSPEND_INFO suspend_info = {
            .suspend_callback = LvpStandbyRatioSuspend,
            .priv = NULL
        };

        LVP_RESUME_INFO resume_info = {
            .resume_callback = LvpStandbyRatioResume,
            .priv = NULL
        };
        LvpSuspendInfoRegist(&suspend_info);
        LvpResumeInfoRegist(&resume_info);
    }
}

int LvpCountRealTimeStandbyRatio(void)
{
    unsigned long time;
    gx_rtc_get_tick(&time);
    _RecordSystemStatus(s_standby_ratio.start_time, time, 1);
    s_standby_ratio.start_time = time;

    int system_standby_count = 0;
    for (int i = 0; i < CONFIG_CACULATE_STANDBY_RATIO_WINDOW_LENGTH; i++) {
        if (0 == s_standby_ratio.system_status[i]) system_standby_count ++;
    }

    // printf ("s: %d\n", system_standby_count);
    int standby_ratio = system_standby_count*1000/CONFIG_CACULATE_STANDBY_RATIO_WINDOW_LENGTH;

    return standby_ratio;
}

int LvpCountAverageStandbyRatio(void)
{
    float ratio = (float)s_standby_ratio.standby_time/(float)(s_standby_ratio.standby_time + s_standby_ratio.running_time);
    return (int)(100*ratio);
}
