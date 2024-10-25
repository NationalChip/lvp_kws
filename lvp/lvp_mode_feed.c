/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * grus_loop.c:
 *
 */
#include <autoconf.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <csi_core.h>
#include <soc_config.h>

#include <driver/gx_audio_in.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_watchdog.h>
#include <driver/gx_delay.h>
#include <driver/gx_clock.h>
#include <driver/gx_irq.h>
#include <driver/gx_snpu.h>
#include <driver/gx_timer.h>
#include <driver/gx_gpio.h>
#include <gsensor.h>

#include <lvp_context.h>
#include <lvp_buffer.h>

#include "common/lvp_pmu.h"
#include "common/lvp_queue.h"
#include "common/lvp_audio_in.h"
#include "common/lvp_uart_record.h"
#include "common/lvp_system_init.h"

#include "app_core/lvp_app_core.h"
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
#include "vui/kws/decoder.h"
#endif

#include "uart_message_v2.h"

#include "lvp_mode_tws.h"
#include "lvp_mode.h"
#include "lvp_kws.h"

#define LOG_TAG "[LVP_FEED]"

#define _UART_PORT_ CONFIG_LVP_FEED_UART_PORT

//=================================================================================================
#define LVP_STATE_WAITING_COUNT_DOWN    (18)
#define LVP_STATE_AVAD_COUNT_DOWN       (18)
#define LVP_STATE_FVAD_COUNT_DOWN       (18)
#define LVP_STATE_WAKEUP_COUNT_DOWN     (18)
typedef enum {
    LVP_STANDBY_STATE_WAITING,
    LVP_STANDBY_STATE_AVAD,
    LVP_STANDBY_STATE_FVAD,
    LVP_STANDBY_STATE_WAKEUP,
    LVP_STANDBY_STATE_STANDBY,
}LVP_STANDBY_STATE;

static struct {
    LVP_STANDBY_STATE     state;
    int                 count_down;
    int                 host_sleeped;
} s_standby_state;

DRAM0_STAGE2_SRAM_ATTR void _LvpSetStandbyState(LVP_STANDBY_STATE state)
{
    s_standby_state.state = state;
    switch (state) {
        case LVP_STANDBY_STATE_WAITING:
            s_standby_state.count_down = LVP_STATE_WAITING_COUNT_DOWN;
            break;
        case LVP_STANDBY_STATE_AVAD:
            s_standby_state.count_down = LVP_STATE_AVAD_COUNT_DOWN;
            break;
        case LVP_STANDBY_STATE_FVAD:
            s_standby_state.count_down = LVP_STATE_FVAD_COUNT_DOWN;
            break;
        case LVP_STANDBY_STATE_WAKEUP:
            s_standby_state.count_down = LVP_STATE_WAKEUP_COUNT_DOWN;
            break;
        default:
            break;
    }
}

DRAM0_STAGE2_SRAM_ATTR void _LvpStandbyStateLoop(void){
    if (s_standby_state.state == LVP_STANDBY_STATE_FVAD) {
        s_standby_state.count_down--;
        if (!s_standby_state.count_down) {
            _LvpSetStandbyState(LVP_STANDBY_STATE_STANDBY);
        }
    }
}

//=================================================================================================

typedef struct {
    unsigned int module_id;
    unsigned int priv;
} MODULE_INFO;

#define KWS_TASK_QUEUE_BUFFER_SIZE 10

static unsigned char s_kws_task_queue_buffer[KWS_TASK_QUEUE_BUFFER_SIZE * sizeof(MODULE_INFO)];
static LVP_QUEUE s_kws_task_queue;

//=================================================================================================
#define INVALID_VAD_TIME_MS (900)
static int invalid_vad_cnt = 0;
DRAM0_STAGE2_SRAM_ATTR static int _LvpAudioInRecordCallback(int ctx_index, void *priv)
{
    if (ctx_index > 0) {
        LVP_CONTEXT *context;
        unsigned int ctx_size;
        LvpGetContext(ctx_index - 1, &context, &ctx_size);
        context->ctx_index = ctx_index - 1;
        //context->kws         = 0; // 挪到 ctc.c 中清0
        context->vad         = 0;
        context->G_vad         = 0;

        // Get FFTVad
        int vad = LvpAudioInQueryFFTVad((unsigned *)priv);

#ifdef CONFIG_ENABLE_NOISE_JUDGEMENT
        LvpAuidoInQueryEnvNoise(context);
#endif

        if (context->ctx_header->fft_vad_en) {
            if (vad || context->ctx_index <= LvpGetLogfbankFrameNumPerChannel() / LvpGetPcmFrameNumPerContext()) {
                context->fft_vad = 1;
                invalid_vad_cnt  = 0;
            } else {
                unsigned int frame_length = context->ctx_header->frame_length;
                unsigned int frame_num_per_ctx = context->ctx_header->pcm_frame_num_per_context;
                invalid_vad_cnt++;
                if (invalid_vad_cnt * frame_length * frame_num_per_ctx >= INVALID_VAD_TIME_MS) {
                    context->fft_vad = 0;
                }
                else {
                    context->fft_vad = 1;
                }
            }
        } else {
            context->fft_vad = 1;
        }

#ifdef CONFIG_LVP_STANDBY_ENABLE
        _LvpStandbyStateLoop();
#endif

        if (context->fft_vad) {
            _LvpSetStandbyState(LVP_STANDBY_STATE_FVAD);
        }

        LvpKwsRun(context);

        if (context->ctx_index%15 == 0) {
            printf (LOG_TAG"Ctx:%d, Vad:%d\n", context->ctx_index, context->fft_vad);
        }

        APP_EVENT plc_event = {
            .event_id = AUDIO_IN_RECORD_DONE_EVENT_ID,
            .ctx_index = context->ctx_index
        };
        LvpTriggerAppEvent(&plc_event);
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------

DRAM0_STAGE2_SRAM_ATTR static int _LvpActiveSnpuCallback(int module_id, GX_SNPU_STATE state, void *priv)
{
    MODULE_INFO module_info = {
        .module_id = module_id,
        .priv = (unsigned int)priv
    };
    LvpQueuePut(&s_kws_task_queue, (unsigned char *)&module_info);

    return 0;
}

//-------------------------------------------------------------------------------------------------
typedef struct {
    unsigned int frame_length;       // ms
    unsigned int sample_rate;        // default : 16000
    unsigned int frame_num_per_pack; // default : 4
    unsigned int version;            // 0x1
} UART_FEATURE_CONFIG;

static UART_FEATURE_CONFIG s_feature_config = {
    .frame_length = 10,
    .sample_rate = 16000,
    .frame_num_per_pack = 4,
    .version = 0x1
};

static int _StartGetLogfbank(void);
static int _StopGetLogfbank(void);

static unsigned int s_index = 0;
static int _uartRecvCallback(MSG_PACK * pack, void *priv) {
    if (pack->msg_header.cmd == MSG_NTF_SLEEP) {
        s_standby_state.host_sleeped = 1;
        _StopGetLogfbank();
        while (gx_snpu_get_state() == GX_SNPU_BUSY);
        LvpAudioInResume();
    }

    if (pack->msg_header.cmd == MSG_NTF_WAKEUP) {
        _StartGetLogfbank();
    }

    if (pack->msg_header.cmd == MSG_NTF_FEATURE_READY) {
        if ( pack->len == sizeof(UART_FEATURE_CONFIG)) {
            UART_FEATURE_CONFIG *config = (UART_FEATURE_CONFIG *)pack->body_addr;
            if (config->version != s_feature_config.version) {
                _StopGetLogfbank();
                printf("start get logfbank error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            } else {
                s_standby_state.host_sleeped = 0;
                LvpAudioInSuspend();
                printf("start get logfbank\n");
            }
        } else {
            s_standby_state.host_sleeped = 0;
            LvpAudioInSuspend();
            printf("start get logfbank\n");
        }
    }

    if (pack->msg_header.cmd == MSG_NTF_FEATURE_DATA) {
        _LvpAudioInRecordCallback(s_index, (void *)pack->body_addr);
        s_index++;
    }

    return 0;
}

static int _StartGetLogfbank(void)
{
    UART_MSG_REGIST regist_info_1 = {
        .msg_id = MSG_NTF_FEATURE_READY,
        .port = _UART_PORT_,
        .msg_buffer = (unsigned char*)LvpGetLogfbankBufferAddr(),
        .msg_buffer_length = (unsigned int)LvpGetLogfbankBufferSize(),
        .msg_pack_callback = _uartRecvCallback,
    };
    UartMessageAsyncRegist(&regist_info_1);

    UART_MSG_REGIST regist_info_2 = {
        .msg_id = MSG_NTF_FEATURE_DATA,
        .port = _UART_PORT_,
        .msg_buffer = (unsigned char*)LvpGetLogfbankBufferAddr(),
        .msg_buffer_length = (unsigned int)LvpGetLogfbankBufferSize(),
        .msg_pack_callback = _uartRecvCallback,
    };
    UartMessageAsyncRegist(&regist_info_2);

    MSG_PACK send_pack_start = {
        .port = _UART_PORT_,
        .msg_header.magic = MSG_HOST_MAGIC,
        .msg_header.cmd = MSG_NTF_FEATURE_START,
        .body_addr = (unsigned char*)&s_feature_config,
        .len = sizeof(s_feature_config)
    };
    UartMessageAsyncSend(&send_pack_start);


    return 0;
}

static int _StopGetLogfbank(void)
{
    printf("StopGetLogfbank\n");
    MSG_PACK send_pack_stop = {
        .port = _UART_PORT_,
        .msg_header.magic = MSG_HOST_MAGIC,
        .msg_header.cmd = MSG_NTF_FEATURE_DONE,
    };
    UartMessageAsyncSend(&send_pack_stop);

    UART_MSG_REGIST regist_info_1 = {
        .msg_id = MSG_NTF_FEATURE_READY,
        .port = _UART_PORT_,
        .msg_buffer = (unsigned char*)LvpGetLogfbankBufferAddr(),
        .msg_buffer_length = (unsigned int)LvpGetLogfbankBufferSize(),
        .msg_pack_callback = _uartRecvCallback,
    };
    UartMessageAsyncLogout(&regist_info_1);

    UART_MSG_REGIST regist_info_2 = {
        .msg_id = MSG_NTF_FEATURE_DATA,
        .port = _UART_PORT_,
        .msg_buffer = (unsigned char*)LvpGetLogfbankBufferAddr(),
        .msg_buffer_length = (unsigned int)LvpGetLogfbankBufferSize(),
        .msg_pack_callback = _uartRecvCallback,
    };
    UartMessageAsyncLogout(&regist_info_2);

    return 0;
}

static int _FeedModeInit(LVP_MODE_TYPE mode)
{
    LvpQueueInit(&s_kws_task_queue, s_kws_task_queue_buffer, KWS_TASK_QUEUE_BUFFER_SIZE * sizeof(MODULE_INFO), sizeof(MODULE_INFO));
#if (defined CONFIG_LVP_ENABLE_CTC_DECODER) || (defined CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER || (defined CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER))
    LvpInitCtcKws();
#endif

    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    LvpKwsInit(_LvpActiveSnpuCallback, start_mode);
    if (start_mode == GX_WAKEUP_SOURCE_COLD || start_mode == GX_WAKEUP_SOURCE_WDT) {
        if (0 != LvpAudioInInit(_LvpAudioInRecordCallback)) {
            printf(LOG_TAG"LvpAudioInInit Failed\n");
            return -1;
        }
    } else {
        LvpAudioInStandbyToStartup();
    }

    s_standby_state.host_sleeped = 1;
    UART_MSG_INIT_CONFIG init_config = {
        .port = _UART_PORT_,
        .baudrate = 500000,
        .reinit_flag = 1
    };

    UartMessageAsyncDone();
    if (UartMessageAsyncInit(&init_config))
        return -1;

    //    _StartGetLogfbank();
    UART_MSG_REGIST regist_info_1 = {
        .msg_id = MSG_NTF_SLEEP,
        .port = _UART_PORT_,
        .msg_pack_callback = _uartRecvCallback,
    };
    UartMessageAsyncRegist(&regist_info_1);

    UART_MSG_REGIST regist_info_2 = {
        .msg_id = MSG_NTF_WAKEUP,
        .port = _UART_PORT_,
        .msg_pack_callback = _uartRecvCallback,
    };
    UartMessageAsyncRegist(&regist_info_2);

    _LvpSetStandbyState(LVP_STANDBY_STATE_FVAD);

    return 0;
}

DRAM0_STAGE2_SRAM_ATTR static void _FeedModeTick(void)
{
#ifdef CONFIG_UART_RECORD_ENABLE
    UartRecordTick();
#endif

    MODULE_INFO module_info = {0};
    if (LvpQueueGet(&s_kws_task_queue, (unsigned char *)&module_info)) {
        if (module_info.module_id == 0x100) {
#ifdef CONFIG_LVP_HAS_VOICE_PLAYER
            if (LvpVoicePlayerGetStatus() != PLAYER_STATUS_PLAY
                && LvpVoicePlayerGetStatus() != PLAYER_STATUS_PREPARE) {
# ifdef CONFIG_LVP_ENABLE_CTC_DECODER
                LvpDoKWSScore((LVP_CONTEXT *)module_info.priv);
# else
                LvpDoUserDecoder((LVP_CONTEXT *)module_info.priv);
# endif
            }
#else
# ifdef CONFIG_LVP_ENABLE_CTC_DECODER
            LvpDoKwsScore((LVP_CONTEXT *)module_info.priv);
# else
            LvpDoUserDecoder((LVP_CONTEXT *)module_info.priv);
# endif
#endif
            LvpAudioInUpdateReadIndex(1);
        }

#ifdef CONFIG_LVP_ENABLE_G_SENSOR_VAD
        if (((LVP_CONTEXT *)module_info.priv)->kws && LvpGetGvad()) {
#else
            if (((LVP_CONTEXT *)module_info.priv)->kws) {
#endif
                if (s_standby_state.host_sleeped == 1) {
                    MSG_PACK send_pack = {
                        .port = _UART_PORT_,
                        .msg_header.magic = MSG_HOST_MAGIC,
                        .msg_header.cmd = MSG_NTF_SET_WAKEUP,
                    };
                    UartMessageAsyncSend(&send_pack);
                }

                APP_EVENT plc_event = {
                    .event_id = ((LVP_CONTEXT *)module_info.priv)->kws,
                    .ctx_index = ((LVP_CONTEXT *)module_info.priv)->ctx_index
                };
                LvpTriggerAppEvent(&plc_event);
            }
        }

        LvpAppEventTick();

        if ((s_standby_state.state == LVP_STANDBY_STATE_STANDBY) && !LvpPmuSuspendIsLocked()) {
            LvpPmuSuspend(LRT_GPIO | LRT_AUDIO_IN | LRT_I2C);
        }
}

static void _FeedModeDone(LVP_MODE_TYPE next_mode)
{
    printf(LOG_TAG"Exit FEED mode\n");
}

static int _FeedModeBufferInit(void)
{
    return LvpInitBuffer();
}
//-------------------------------------------------------------------------------------------------

const LVP_MODE_INFO lvp_feed_mode_info = {
    .type = LVP_MODE_FEED,
    .buffer_init = _FeedModeBufferInit,
    .init = _FeedModeInit,
    .done = _FeedModeDone,
    .tick = _FeedModeTick,
};
