/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app_event.c:
 */

#include <autoconf.h>

#include <stdio.h>
#include <string.h>

#include <lvp_system_init.h>
#include <lvp_buffer.h>
#include <board_config.h>
#include <driver/gx_timer.h>

#if (defined CONFIG_LVP_ENABLE_G_SENSOR_RECORD || defined CONFIG_LVP_ENABLE_G_SENSOR_VAD)
#include <lvp_G_vad.h>
#endif

#ifdef CONFIG_LVP_HAS_VOICE_PLAYER
#include <lvp_voice_player.h>
#endif

#ifdef CONFIG_LVP_HAS_MP3_PLAYER
#include <lvp_mp3_player.h>
#endif

#ifdef CONFIG_LVP_HAS_WATCHDOG
#include <driver/gx_watchdog.h>
#endif

#include <gsensor.h>
#include "lvp_uart_record.h"
#include "lvp_pmu.h"
#include "lvp_queue.h"
#include "lvp_app_core.h"
#include "lvp_app.h"
#include "uart_message_v2.h"

#define LOG_TAG "[APP_EVENT]"

__attribute__((weak)) LVP_APP *app_core_ops = NULL;
//=================================================================================================

#define LVP_APP_MISC_QUEUE_LEN 8
static unsigned char s_app_misc_event_queue_buffer[LVP_APP_MISC_QUEUE_LEN * sizeof(APP_EVENT)] = {0};
LVP_QUEUE s_app_misc_event_queue;

#if ((defined CONFIG_LVP_ENABLE_G_SENSOR_VAD) || (defined CONFIG_UART_RECORD_ENABLE))
#define LVP_APP_ENABLE_AIN_CB_EVENT
#endif
#ifdef LVP_APP_ENABLE_AIN_CB_EVENT
#define LVP_APP_AIN_CB_QUEUE_LEN 3
static unsigned char s_app_ain_cb_event_queue_buffer[LVP_APP_AIN_CB_QUEUE_LEN * sizeof(APP_EVENT)] = {0};
LVP_QUEUE s_app_ain_cb_event_queue;
#endif

//=================================================================================================
#if (defined CONFIG_LVP_ENABLE_G_SENSOR_RECORD || defined CONFIG_LVP_ENABLE_G_SENSOR_VAD)
#define G_VAD_STRIDE_SIZE (CONFIG_LVP_G_SENSOR_VAD_FRAME_SHIFT * 6)
static unsigned char G_sensor_buffer_stride[G_VAD_STRIDE_SIZE]  ALIGNED_ATTR(16)= {0};
int LvpAppUpdataGvad(APP_EVENT *app_event)
{
    if (app_event->ctx_index < 2) {
        return 0;
    }

    unsigned int G_vad = 0;

//            unsigned int ttt = gx_get_time_us();
//        LvpDynamiciallyAdjustCpuFrequency(CPU_FREQUENCE_24M);
//    printf("zzz %d\n", GsensorGetDataSize());
    while (GsensorGetDataSize() >= G_VAD_STRIDE_SIZE) {
        if (GsensorGetConsequentDataSize() >= G_VAD_STRIDE_SIZE) {
            unsigned int G_sensor_len = GsensorGetConsequentDataSize() - (GsensorGetConsequentDataSize() % G_VAD_STRIDE_SIZE);
            unsigned char *G_sensor_buffer = GsensorGetDataPointer(G_sensor_len);
#ifdef CONFIG_LVP_ENABLE_G_SENSOR_VAD
            G_vad = LvpGsensorVad((short *)G_sensor_buffer, G_sensor_len, 3);
#endif
#ifdef CONFIG_LVP_ENABLE_G_SENSOR_RECORD
            UartRecordChannelTask(G_sensor_buffer, G_sensor_len, RECORD_CHANNEL_G_SENSOR);
#endif
        } else if (GsensorGetDataSize() >= G_VAD_STRIDE_SIZE) {
            unsigned int G_sensor_len = GsensorGetDataSize();
//            printf("G %d\n", G_sensor_len / G_VAD_STRIDE_SIZE);
            GsensorGetData(G_sensor_buffer_stride, G_VAD_STRIDE_SIZE);
#ifdef CONFIG_LVP_ENABLE_G_SENSOR_VAD
            G_vad = LvpGsensorVad((short *)G_sensor_buffer_stride, G_sensor_len, 3);
#endif
#ifdef CONFIG_LVP_ENABLE_G_SENSOR_RECORD
            UartRecordChannelTask(G_sensor_buffer_stride, G_VAD_STRIDE_SIZE, RECORD_CHANNEL_G_SENSOR);
#endif
        }
    }
//        LvpDynamiciallyAdjustCpuFrequency(CPU_FREQUENCE_12M);
//            printf("g %d\n", gx_get_time_us() - ttt);

    return G_vad;
}
#endif

//=================================================================================================
int LvpTriggerAppEvent(APP_EVENT *app_event)
{
#ifdef CONFIG_LVP_ENABLE_G_SENSOR_VAD
    unsigned int kws_id = app_event->event_id;
    if ((kws_id >= 100 && kws_id <= 255) && !LvpGetGvad()) return 0;  // Gvad未激活时,过滤掉kws事件
#endif

    LvpQueuePut(&s_app_misc_event_queue, (const unsigned char *)app_event);

#ifdef LVP_APP_ENABLE_AIN_CB_EVENT
    if (app_event->event_id == AUDIO_IN_RECORD_DONE_EVENT_ID) {
        LvpQueuePut(&s_app_ain_cb_event_queue, (const unsigned char *)app_event);
    }
#endif

    return 0;
}

static int _LvpAppSuspend(void *priv)
{
#ifdef CONFIG_LVP_HAS_WATCHDOG
    gx_watchdog_stop();
#endif
    if ((app_core_ops) && (app_core_ops->AppSuspend)) {
        app_core_ops->AppSuspend(app_core_ops->suspend_priv);
    }
    return 0;
}

static int _LvpAppResume(void *priv)
{
    if((app_core_ops) && (app_core_ops->AppResume)) {
        app_core_ops->AppResume(app_core_ops->resume_priv);
    }
    return 0;
}

#ifdef CONFIG_LVP_HAS_WATCHDOG
static void _WatchdogCallback(int irq, void *pdata)
{
        gx_reboot();
}
#endif

#if (defined CONFIG_LVP_ENABLE_G_SENSOR_RECORD || defined CONFIG_LVP_ENABLE_G_SENSOR_VAD)
static unsigned char record_buffer[6 * 16 * 10] ALIGNED_ATTR(16);
#endif
int LvpInitializeAppEvent(void)
{
#if (defined CONFIG_LVP_ENABLE_G_SENSOR_RECORD || defined CONFIG_LVP_ENABLE_G_SENSOR_VAD)
    GsensorInit(record_buffer, sizeof(record_buffer));

# ifdef CONFIG_LVP_ENABLE_G_SENSOR_VAD
#  ifdef CONFIG_LVP_G_SENSOR_VAD_V1
    G_VAD_CONFIG config = {
        .useful_dim_mask                    = G_VAD_DIM_1 | G_VAD_DIM_2 | G_VAD_DIM_3,
        .fft_frame_shift                    = CONFIG_LVP_G_SENSOR_VAD_FRAME_SHIFT,
        .G_vad_threshold                    = CONFIG_LVP_G_SENSOR_VAD_ENERGY_THRESHOLD * 0.01f,
        .G_vad_hold_context_counter         = CONFIG_LVP_G_SENSOR_VAD_HOLD_CONTEXT_NUM,
        .G_vad_voice_count_window           = CONFIG_LVP_G_SENSOR_VAD_VOICE_COUNT_WINDOW,
        .G_vad_voice_count_threshold        = CONFIG_LVP_G_SENSOR_VAD_VOICE_COUNT_THRESHOLD,
        .G_vad_fft_result_count_threshold   = CONFIG_LVP_G_SENSOR_FFT_RESULT_COUNT_THRESHOLD
    };

    LvpGsensorVadInit(&config);
#  endif
#  ifdef CONFIG_LVP_G_SENSOR_VAD_V2
    G_VAD_CONFIG config = {
        .useful_dim_mask                    = G_VAD_DIM_1 | G_VAD_DIM_2 | G_VAD_DIM_3,
        .fft_frame_shift                    = CONFIG_LVP_G_SENSOR_VAD_FRAME_SHIFT,
        .G_vad_threshold                    = CONFIG_LVP_G_SENSOR_VAD_ENERGY_THRESHOLD * 0.001f,
        .G_vad_voice_count_window           = CONFIG_LVP_G_SENSOR_VAD_VOICE_COUNT_WINDOW,
        .G_vad_voice_count_threshold        = CONFIG_LVP_G_SENSOR_VAD_VOICE_COUNT_THRESHOLD,
        .G_vad_fft_result_count_threshold   = CONFIG_LVP_G_SENSOR_FFT_RESULT_COUNT_THRESHOLD
    };

    LvpGsensorVadInit(&config);
#  endif
#  ifdef CONFIG_LVP_G_SENSOR_VAD_V3
    G_VAD_CONFIG config = {
        .useful_dim_mask                    = G_VAD_DIM_1,
        .fft_frame_shift                    = CONFIG_LVP_G_SENSOR_VAD_FRAME_SHIFT,
        .G_vad_threshold_1                  = CONFIG_LVP_G_SENSOR_VAD_ENERGY_THRESHOLD_1 * 0.001f,
        .G_vad_window_1                     = CONFIG_LVP_G_SENSOR_VAD_ENERGY_WINDOWS_1,
        .G_vad_threshold_2                  = CONFIG_LVP_G_SENSOR_VAD_ENERGY_THRESHOLD_2,
        .G_vad_window_2                     = CONFIG_LVP_G_SENSOR_VAD_ENERGY_WINDOWS_2,
        .G_vad_voice_count_window           = CONFIG_LVP_G_SENSOR_VAD_ENERGY_WINDOWS_1 + CONFIG_LVP_G_SENSOR_VAD_ENERGY_WINDOWS_2,
        .G_vad_voice_count_threshold        = CONFIG_LVP_G_SENSOR_VAD_VOICE_COUNT_THRESHOLD,
    };

    LvpGsensorVadInit(&config);
#  endif
# endif
#endif

#ifdef CONFIG_UART_RECORD_ENABLE
    UartRecordInit(-1, 0);
#endif

    LvpQueueInit(&s_app_misc_event_queue, s_app_misc_event_queue_buffer, LVP_APP_MISC_QUEUE_LEN * sizeof(APP_EVENT), sizeof(APP_EVENT));
#ifdef LVP_APP_ENABLE_AIN_CB_EVENT
    LvpQueueInit(&s_app_ain_cb_event_queue, s_app_ain_cb_event_queue_buffer, LVP_APP_AIN_CB_QUEUE_LEN * sizeof(APP_EVENT), sizeof(APP_EVENT));
#endif
    if((app_core_ops) && (app_core_ops->AppInit))
        app_core_ops->AppInit();

    LVP_SUSPEND_INFO suspend_info = {
        .suspend_callback = _LvpAppSuspend,
        .priv = "_LvpAppSuspend"
    };

    LVP_RESUME_INFO resume_info = {
        .resume_callback = _LvpAppResume,
        .priv = "_LvpAppResume"
    };
    LvpSuspendInfoRegist(&suspend_info);
    LvpResumeInfoRegist(&resume_info);

#ifdef CONFIG_LVP_HAS_WATCHDOG
        gx_watchdog_init(CONFIG_LVP_HAS_WATCHDOG_RESET_MS ,\
                         CONFIG_LVP_HAS_WATCHDOG_TIMEOUT_MS,\
                         _WatchdogCallback, NULL);
#endif
    return 0;
}

int LvpAppEventTick(void)
{
#ifdef CONFIG_UART_RECORD_ENABLE
    UartRecordTick();
#endif

    APP_EVENT app_event = {.event_id = 0};

    if (LvpQueueGet(&s_app_misc_event_queue, (unsigned char *)&app_event)) {
        if((app_core_ops) && (app_core_ops->AppEventResponse))
            app_core_ops->AppEventResponse(&app_event);
    }

#ifdef LVP_APP_ENABLE_AIN_CB_EVENT
    if (LvpQueueGet(&s_app_ain_cb_event_queue, (unsigned char *)&app_event)) {
#if (defined CONFIG_LVP_ENABLE_G_SENSOR_RECORD || defined CONFIG_LVP_ENABLE_G_SENSOR_VAD)
        LvpAppUpdataGvad(&app_event);
# endif
# ifdef CONFIG_UART_RECORD_ENABLE
        LVP_CONTEXT *context;
        unsigned int ctx_size;
        LvpGetContext(app_event.ctx_index, &context, &ctx_size);
        UartRecordTask(context);
# endif
    }
#endif

    if((app_core_ops) && (app_core_ops->AppTaskLoop)) {
        app_core_ops->AppTaskLoop();
    }
#ifdef CONFIG_LVP_HAS_VOICE_PLAYER
    LvpVoicePlayerTask(NULL);
#endif
#ifdef CONFIG_LVP_HAS_MP3_PLAYER
     LvpMp3PlayerTask(NULL);
#endif

#ifdef CONFIG_LVP_HAS_UART_MESSAGE_2_0
    UartMessageAsyncTick();
#endif

#ifdef CONFIG_LVP_HAS_WATCHDOG_TICK_BLOCK
    gx_watchdog_ping();
#endif
    return 0;
}


