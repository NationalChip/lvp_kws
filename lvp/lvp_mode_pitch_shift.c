/* Grus
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mode_pitchshift.c:
 *
 */
#include <autoconf.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <csi_core.h>

#include <driver/gx_audio_in.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_watchdog.h>
#include <driver/gx_delay.h>
#include <driver/gx_clock.h>
#include <driver/gx_irq.h>
#include <driver/gx_snpu.h>
#include <driver/gx_timer.h>
#include <driver/gx_gpio.h>
#include <driver/gx_dcache.h>

#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_board.h>

#include <lvp_queue.h>
#include <lvp_audio_in.h>
#include <uart_message_v2.h>
#include <lvp_uart_record.h>
#include <lvp_attr.h>

#include "lvp_mode.h"
#include "driver/dsp/csky_math.h"
#include "app_core/lvp_app_core.h"

#include <pitchshift.h>
#include <imcra.h>
#ifdef CONFIG_LVP_HAS_VOICE_PLAYER
#include "lvp_voice_player.h"
#endif
#ifdef CONFIG_LVP_HAS_MP3_PLAYER
#include <lvp_mp3_player.h>
#endif
#define LOG_TAG "[LVP_PITCH_SHIFT]"

//=================================================================================================

// #define BEBUG_TIME_PRINTF
// #define CONFIG_LVP_HAS_MODE_UART_RECORD
#define ENABLE_PITCH_SHIFT
#define ENABLE_IMCRA

#define SAMPLE_RATE                     (16000)                     // 采样率16K
#ifdef CONFIG_PCM_FRAME_LENGTH_10MS
#define FRAME_LENGTH                    (10 * SAMPLE_RATE / 1000)
#elif defined CONFIG_PCM_FRAME_LENGTH_16MS
#define FRAME_LENGTH                    (16 * SAMPLE_RATE / 1000)
#endif
#define FRAME_LENGTH_PER_CONTEXT        (CONFIG_LVP_PCM_FRAME_NUM_PER_CONTEXT * FRAME_LENGTH)
#define OUT_WAV_NUM                     (8)
#define OUT_WAV_LENGTH                  FRAME_LENGTH_PER_CONTEXT
#define PITCH_SHIFT_TMP_BUFFER_SIZE     (9 * 1024)                  // 运行 PitchShift 需要的 Memory
#define IMCRA_TMP_BUFFER_SIZE           (42 * 1024)                 // 运行 IMCRA 需要的 Memory  42/58

static LVP_QUEUE s_index_queue;
static uint8_t s_index_queue_buffer[8 * sizeof(LVP_CONTEXT *)]          = {0};
static short wav_buffer[OUT_WAV_LENGTH * OUT_WAV_NUM]                   = {0};

#ifdef ENABLE_PITCH_SHIFT
static unsigned char s_pitch_shift_tmp_buf[PITCH_SHIFT_TMP_BUFFER_SIZE] = {0};
static PitchShiftState *s_pitch_shift_st                                = NULL;
#endif

#ifdef ENABLE_IMCRA
// static unsigned char s_imcra_tmp_buf[IMCRA_TMP_BUFFER_SIZE]             = {0};
static unsigned char *s_imcra_tmp_buf = (void *)(NPU_SRAM_ADDR / 4 * 4);
static ImcraState *s_imcra_st                                           = NULL;
#endif

static volatile char _pitchshift_actived = 1;

void LvpPitchShiftEnable(void)
{
    _pitchshift_actived = 1;
    printf(LOG_TAG"Pitch Shift Enable\n");
}

void LvpPitchShiftDisable(void)
{
    _pitchshift_actived = 0;
    printf(LOG_TAG"Pitch Shift Disable\n");
}

static int _PitchShiftInit(void)
{
#ifdef ENABLE_PITCH_SHIFT
    if (!s_pitch_shift_st) {
        // speed_rate 配置 0.5（升调）或 2（降调）
        //  1. 每次更新输出数据，依赖的样点数（80 / 320）易于context管理
        //  2. 可采用半带滤波器，降低重采样复杂度
#ifdef CONFIG_PITCH_SHIFT_RISING_MODE
        float speed_rate = 0.5;
#elif defined CONFIG_PITCH_SHIFT_DESCENDING_MODE
        float speed_rate = 2;
#endif
        int frame_size  = FRAME_LENGTH_PER_CONTEXT;
        s_pitch_shift_st = PitchShiftStateInit(frame_size, speed_rate, s_pitch_shift_tmp_buf, PITCH_SHIFT_TMP_BUFFER_SIZE);
        if (!s_pitch_shift_st) {
            printf("Pitch Shift init fail!\n");
            while (1) {;}
        }
    }
#endif
#ifdef ENABLE_IMCRA
    s_imcra_st = ImcraStateInit(LVpGetPcmSampleRate(), s_imcra_tmp_buf, IMCRA_TMP_BUFFER_SIZE);
    if (!s_imcra_st) {
        printf("ImcraStateInit Fail!\n");
        while (1) {;}
    }
#endif

    return 0;
}

static int _LvpAudioInRecordCallback(int ctx_index, void *priv)
{
    if (ctx_index > 0) {
        LVP_CONTEXT *context;
        unsigned int ctx_size;
        LvpGetContext(ctx_index - 1, &context, &ctx_size);
        context->ctx_index = ctx_index - 1;
        context->kws        = 0;
        context->vad        = 0;

        if (context->ctx_index%15 == 0) {
            // printf (LOG_TAG"Ctx:%d, Vad:%d\n", context->ctx_index, context->G_vad);
        }

#if(defined CONFIG_LVP_HAS_VOICE_PLAYER) || (defined CONFIG_LVP_HAS_MP3_PLAYER)
        if (
# ifdef CONFIG_LVP_HAS_VOICE_PLAYER
            !(LvpVoicePlayerGetStatus() != PLAYER_STATUS_PLAY && LvpVoicePlayerGetStatus() != PLAYER_STATUS_PREPARE)
# else
            !(LvpMp3PlayerGetStatus() != PLAYER_STATUS_PLAY && LvpMp3PlayerGetStatus() != PLAYER_STATUS_PREPARE)
# endif
        )
#else
        if (0)
#endif
        {
            LvpAudioInUpdateReadIndex(1);
        } else {
            LvpQueuePut(&s_index_queue, (unsigned char *)&(context));
            LvpAudioInUpdateReadIndex(1);
        }
    }

   return 0;
}

static int out_wav_offset = 0;
static void _PitchShiftTick(void)
{
    LVP_CONTEXT *context = NULL;
    if (_pitchshift_actived && LvpQueueGet(&s_index_queue, (unsigned char *)&context)) {
#ifdef BEBUG_TIME_PRINTF
        unsigned long long s_t =  gx_get_time_us();
#endif
        int start = out_wav_offset;
        int frame_length_per_context = FRAME_LENGTH_PER_CONTEXT;
        short *out_buffer = &wav_buffer[start * OUT_WAV_LENGTH];
        short *mic_buffer = LvpGetMicFrame(context, 0 ,0);
        gx_dcache_invalid_range((unsigned int*)mic_buffer, frame_length_per_context * sizeof(short));

#ifdef ENABLE_IMCRA
        for (int j = 0; j < CONFIG_LVP_PCM_FRAME_NUM_PER_CONTEXT; j++) {
            ImcraProcessing(s_imcra_st, &mic_buffer[j * FRAME_LENGTH], &out_buffer[j * FRAME_LENGTH]);
        }
#endif

#ifdef ENABLE_PITCH_SHIFT
        int Ha            = GetPitchShiftHaSize(s_pitch_shift_st);
        int Ha_num        = frame_length_per_context / Ha;
        for (int k = 0; k < Ha_num; k++) {
# ifdef ENABLE_IMCRA
            PitchShiftProcessing(s_pitch_shift_st, &out_buffer[k*Ha], &mic_buffer[k*Ha]);
# else
            PitchShiftProcessing(s_pitch_shift_st, &mic_buffer[k*Ha], &out_buffer[k*Ha]);
# endif
        }
# ifdef ENABLE_IMCRA
        memcpy(out_buffer, mic_buffer, frame_length_per_context * sizeof(short));
# endif
#endif

#ifdef BEBUG_TIME_PRINTF
        unsigned long long e_t =  gx_get_time_us();
        printf(LOG_TAG"use_time: %ld\n", e_t - s_t);
#endif
        ++out_wav_offset;
        if (out_wav_offset >= OUT_WAV_NUM)
            out_wav_offset %= OUT_WAV_NUM;

        APP_EVENT event = {
            .event_id = OUT_WAV_LENGTH * sizeof(short),
            .ctx_index = (unsigned int)&wav_buffer[start * OUT_WAV_LENGTH],
        };
        LvpTriggerAppEvent(&event);

#ifdef CONFIG_LVP_HAS_MODE_UART_RECORD
# if (defined ENABLE_PITCH_SHIFT) && (defined ENABLE_IMCRA)
        gx_dcache_clean_range((uint32_t*)mic_buffer, frame_length_per_context * sizeof(short));
        UartRecordChannelTask((unsigned char *)mic_buffer,
                                    OUT_WAV_LENGTH * sizeof(short), RECORD_CHANNEL_MIC0);
# else
        UartRecordChannelTask((unsigned char *)&wav_buffer[start * OUT_WAV_LENGTH],
                              OUT_WAV_LENGTH * sizeof(short), RECORD_CHANNEL_MIC0);
# endif
#endif
    }
}

//-------------------------------------------------------------------------------------------------

static int _PitchShiftModeInit(LVP_MODE_TYPE mode)
{
    printf("Pitch Shift Mode Init\n");
    LvpQueueInit(&s_index_queue, s_index_queue_buffer,
                sizeof(s_index_queue_buffer), sizeof(unsigned int));

    if (0 != LvpAudioInInit(_LvpAudioInRecordCallback)) {
        printf(LOG_TAG"LvpAudioInInit Failed\n");
        return -1;
    }

    _PitchShiftInit();
#ifdef CONFIG_LVP_HAS_MODE_UART_RECORD
    UartRecordInit(-1, 0);
#endif
    printf("%s  OK\n", __func__);
    return 0;
}

static void _PitchShiftModeTick(void)
{
#if (defined CONFIG_LVP_HAS_VOICE_PLAYER) || (defined CONFIG_LVP_HAS_MP3_PLAYER)
    if (
# ifdef CONFIG_LVP_HAS_VOICE_PLAYER
        LvpVoicePlayerGetStatus() != PLAYER_STATUS_PLAY
        && LvpVoicePlayerGetStatus() != PLAYER_STATUS_PREPARE
# else
        LvpMp3PlayerGetStatus() != PLAYER_STATUS_PLAY
        && LvpMp3PlayerGetStatus() != PLAYER_STATUS_PREPARE
# endif
    ) {
        _PitchShiftTick();
#ifdef CONFIG_LVP_HAS_MODE_UART_RECORD
        UartRecordTick();
#endif
    }
//     else {}
// #else
//     LvpAudioInUpdateReadIndex(1);
#endif

}

static void _PitchShiftModeDone(LVP_MODE_TYPE next_mode)
{
    printf(LOG_TAG"Exit Pitch Shift mode\n");

#ifdef ENABLE_PITCH_SHIFT
    if (s_pitch_shift_st) PitchShiftStateDestroy(s_pitch_shift_st);
#endif
#ifdef ENABLE_IMCRA
    if (s_imcra_st) ImcraStateDestroy(s_imcra_st);
#endif
}

static int _PitchShiftModeBufferInit(void)
{
    return LvpInitBuffer();
}
//-------------------------------------------------------------------------------------------------

const LVP_MODE_INFO lvp_pitchshift_mode_info = {
    .type = LVP_HAS_PITCH_SHIFT_MODE,
    .buffer_init = _PitchShiftModeBufferInit,
    .init = _PitchShiftModeInit,
    .done = _PitchShiftModeDone,
    .tick = _PitchShiftModeTick,
};
