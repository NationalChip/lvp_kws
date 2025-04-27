/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mode_denoise.c:
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

#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_board.h>

#include "common/lvp_queue.h"
#include "common/lvp_audio_in.h"
#include "lvp_mode.h"
#include "app_core/lvp_app_core.h"
#include "csky_const_structs.h"

#ifdef CONFIG_DENOISE_ENABLE_GSC
#include "gsc/beamforming.h"
#endif

#ifdef CONFIG_DENOISE_ENABLE_WEBRTC
#include "webrtc/modules/audio_processing/ns/noise_suppression_x.h"
#endif

#include "lvp_mode_denoise.h"
#include "driver/gx_dcache.h"

#define LOG_TAG "[LVP_MODE_DENOISE]"
#define CONTEXT_TASK_QUEUE_MEMBER_NUM    10

static LVP_QUEUE context_queue;
static unsigned char context_queue_buffer[CONTEXT_TASK_QUEUE_MEMBER_NUM * sizeof(void*)];
static volatile char _denoise_actived = 0;
void LvpDenoiseEnable(void)
{
    _denoise_actived = 1;
}

void LvpDenoiseDisable(void)
{
    _denoise_actived = 0;
}

int LvpDenoiseStatus(void)
{
    return _denoise_actived;
}

void LvpDenoiseGetInputBuffer(void **addr, int *len)
{
    if((addr == NULL) || (len == NULL)) {
        return;
    }

    *addr = LvpGetMicBufferAddr();
    *len = LvpGetMicBufferSize() / LvpGetMicChannelNum();
}

#if 0
static unsigned int input_index = 0;
int LvpDenoisePushPcm(void)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;

    LvpGetContext(input_index, &context, &ctx_size);
    context->ctx_index = input_index++;
    LvpQueuePut(&context_queue, (unsigned char *)&context);
}
#endif

#ifdef CONFIG_DENOISE_ENABLE_WEBRTC
static NsxHandle *g_ns_st;
static int _webrtcDenoiseInit(void)
{
    if((g_ns_st = WebRtcNsx_Create()) == NULL)
        goto _failed;

    if(WebRtcNsx_Init(g_ns_st, 16000) < 0)
        goto _failed;

    if(WebRtcNsx_set_policy(g_ns_st, 0) < 0)
        goto _failed;

    return 0;

_failed:
    if(g_ns_st > 0) {
        WebRtcNsx_Free(g_ns_st);
        g_ns_st = NULL;
    }

    return -1;
}

static int _webrtcDenoiseProc(LVP_CONTEXT *context)
{
    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    short *speech_frame[2];
    short *out_frame[2];

    short *out = (short *)LvpGetOutBuffer(context->ctx_index);
    for(int i = 0; i < ctx_header->pcm_frame_num_per_context; i++) {
        short *cur_mic_buffer = LvpGetMicFrame(context, 0, i);
        speech_frame[0] = cur_mic_buffer;
        out_frame[0] = out;
        WebRtcNsx_Process(g_ns_st, (const short* const*)speech_frame, 1, out_frame);
        out += ctx_header->frame_length * ctx_header->sample_rate / 1000;
    }

    return 0;
}
#endif

#ifdef CONFIG_DENOISE_ENABLE_GSC
static TRI_MIC_BEAMFORMING_STATE *g_bf_st;
static short *g_bf_mictmp   = NULL;
static short *g_bf_temp_buf   = NULL;

static int _beamformingInit(void)
{
    int mic_num                     = 2;//ctx_header->mic_num;
    int sample_rate                 = 16000;//ctx_header->sample_rate;
    float mic_arrays_distance       = 0.03;//VspGetMicDistance();    // DISTANCE between MICS
    float mic_arrays_radius         = 1;//VspGetMicDistance();    // DISTANCE between MICS
    float elevation                 = 0; // Angle between incident angle and horizontal plane
    float gain                      = 1.0f;//VspGetBeamformingGain();
    float sub_ratio                 = 0.6f;//VspGetSpectrumsSubRatio();
    int is_spectrums_sub            = 0;
    float short_time_filter_alpha   = 0.2;
    float  long_time_filter_alpha   = 0.01;
    int is_linear                   = 1;
    int is_circular                 = 0;
    int frame_length                = 400;      // 25ms
    int frame_shift                 = 160;      // 10ms
    int frame_num                   = 3;        // set as 3, DO NOT MODIFY FOT NOW

    //short beam_angle[]             = {120, 130, 140};
    //short beam_angle[]             = {35, 45, 55};
    short beam_angle[]             = {80, 90, 100};
    int beamforming_angle_num       = 3;
    int out_beam_num                = 1;
    int out_beam_idx[]             = {1};

    BEAMFORMING_PORTING_OPS ops;

    ops.mem_ops.bf_malloc = LvpMalloc;
    ops.mem_ops.bf_calloc = LvpCalloc;
    ops.mem_ops.bf_realloc = LvpRealloc;
    ops.mem_ops.bf_free = LvpFree;

    beamforming_porting_init(&ops);

    g_bf_st = beamforming_triMic_init(mic_num, beamforming_angle_num, sample_rate, gain, sub_ratio, is_spectrums_sub,
            is_linear, mic_arrays_distance, is_circular, mic_arrays_radius, elevation,
            short_time_filter_alpha, long_time_filter_alpha, 0, out_beam_num, out_beam_idx);

    if(!g_bf_st) {
        printf (LOG_TAG"Init Beamforming Failed\n");
        return -1;
    }
    set_beam_angle(g_bf_st, beamforming_angle_num, beam_angle);

    g_bf_mictmp = (short *)LvpMalloc(mic_num * frame_length * sizeof(short));
    if (!g_bf_mictmp) {
        printf(LOG_TAG"BF malloc g_bf_mictmp failed\n");
        return -1;
    }

    int bf_size = get_Beamforming_tmpBuffer_size(mic_num, frame_num, frame_length, frame_shift);
    printf("bf_size = %d\n", bf_size);
    g_bf_temp_buf = (short *)LvpMalloc(bf_size);
    if(!g_bf_temp_buf) {
        printf(LOG_TAG"BF malloc g_bf_temp_buf failed\n");
        return -1;
    }
    set_Beamforming_tmpBuffer_size(g_bf_st, g_bf_temp_buf);

    return 0;
}

static int _beamformingProc(LVP_CONTEXT *context)
{
    LVP_CONTEXT_HEADER *ctx_header  = context->ctx_header;
    int frame_num_per_context       = ctx_header->pcm_frame_num_per_context; // set as 3, DO NOT MODIFY FOT NOW
    int beamforming_count           = 0;
    int frame_idx                   = 0;
    int mic_num                     = ctx_header->mic_num;
    int frame_length                = 400;      // 25ms
    int frame_shift                 = 160;      // 10ms
    int frame_num                   = 3;        // set as 3, DO NOT MODIFY FOT NOW
    int out_buffer_idx              = 0;
    int out_buffer_total_idx        = frame_num_per_context / frame_num;

    if (!g_bf_st) {
        printf (LOG_TAG"Need init Beamforming\n");
        return -1;
    }

    for (beamforming_count = 0; beamforming_count < frame_num_per_context; beamforming_count++) {
        for (int i = 0; i < mic_num; i++) {
            short *mic_buffer = LvpGetMicFrame(context, i, beamforming_count);
            csky_copy_q15(mic_buffer, &g_bf_mictmp[i * frame_length + (frame_length - frame_shift)], frame_shift);
        }

        frame_idx = context->ctx_index * ctx_header->pcm_frame_num_per_context + beamforming_count + 1;
        beamforming_triMic_spectrums(g_bf_st, g_bf_mictmp, frame_idx - 1);

        if (frame_idx % frame_num == 0) {
            float sum_change_rate_ratio[2];
            beamforming_triMic_processing(g_bf_st, LvpGetOutBuffer(context->ctx_index), out_buffer_total_idx, out_buffer_idx, sum_change_rate_ratio);
            out_buffer_idx += 1;

            //short *mic_buffer = LvpGetMicFrame(context, 0, 0);
            //csky_copy_q15(mic_buffer, &context->out_buffer[0], frame_shift * frame_num);
        }

        // shift micdata
        for (int i = 0; i < mic_num; i++) {
            csky_copy_q15(g_bf_mictmp + i * frame_length + frame_shift, g_bf_mictmp + i * frame_length, frame_length - frame_shift);
        }
    }

    return 0;
}

#endif

//=================================================================================================
#ifdef CONFIG_DENOISE_AUDIOIN_SRC_HW
static int _LvpAudioInRecordCallback(int ctx_index, void *priv)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;

#if 0
    static int t1 = 0;
    int tn = gx_get_time_ms();
    printf("%d\n",  tn - t1);
    t1 = tn;
#endif

    LvpGetContext(ctx_index, &context, &ctx_size);
    context->ctx_index = ctx_index;

    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    int ctx_sample_num = ctx_header->pcm_frame_num_per_context * ctx_header->frame_length * ctx_header->sample_rate / 1000;

    for(int i = 0; i < ctx_header->mic_num; i++) {
        short *cur_mic_buffer = LvpGetMicFrame(context, i, 0);
        gx_dcache_invalid_range((unsigned int*)cur_mic_buffer, ctx_sample_num * sizeof(short));
    }

    LvpQueuePut(&context_queue, (unsigned char *)&context);
    return 0;
}
#endif

//-------------------------------------------------------------------------------------------------
static int _DenoiseModeInit(LVP_MODE_TYPE mode)
{
    printf(LOG_TAG"Init denoise mode\n");

    LvpQueueInit(&context_queue, context_queue_buffer, CONTEXT_TASK_QUEUE_MEMBER_NUM * sizeof(void*), sizeof(void*));
    LvpInitHeap();

#ifdef CONFIG_DENOISE_ENABLE_GSC
    printf(LOG_TAG"Init gsc Beamforming\n");
    if(0 !=  _beamformingInit()) {
        printf(LOG_TAG"_beamformingInit Failed !!!\n");
        return -1;
    }
#endif

#ifdef CONFIG_DENOISE_ENABLE_WEBRTC
    printf(LOG_TAG"Init webrtc denoise\n");
    if(0 != _webrtcDenoiseInit()) {
        printf(LOG_TAG"_webrtcDenoiseInit Failed !!!\n");
        return -1;
    }
#endif

#ifdef CONFIG_DENOISE_AUDIOIN_SRC_HW
    if(0 != LvpAudioInInit(_LvpAudioInRecordCallback)) {
        printf(LOG_TAG"LvpAudioInInit Failed !!!\n");
        return -1;
    }
#endif

    printf(LOG_TAG"Init OK!!!\n");

    return 0;
}

static void _DenoiseModeTick(void)
{
    LVP_CONTEXT *context;

    if (LvpQueueGet(&context_queue, (unsigned char *)&context))
    {
        //int t1 = 0;
        //int t2 = 0;

        //t1 = gx_get_time_ms();
        if(_denoise_actived) {
#ifdef CONFIG_DENOISE_ENABLE_GSC
            _beamformingProc(context);
#endif

#ifdef CONFIG_DENOISE_ENABLE_WEBRTC
            _webrtcDenoiseProc(context);
#endif
        } else {
#if 1
            LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
            short *cur_mic_buffer = LvpGetMicFrame(context, 0, 0);
            int ctx_sample_num = ctx_header->pcm_frame_num_per_context * ctx_header->frame_length * ctx_header->sample_rate / 1000;
            //printf("addr: 0x%x, num: %d, output buf = 0x%x\n", cur_mic_buffer, ctx_sample_num, (context->out_buffer));
            //gx_dcache_invalid_range((unsigned int*)cur_mic_buffer, ctx_sample_num * sizeof(short));
            //memcpy(context->out_buffer, cur_mic_buffer, ctx_sample_num * sizeof(short));
            csky_copy_q15(cur_mic_buffer, LvpGetOutBuffer(context->ctx_index), ctx_sample_num);
#endif
        }

        //t2 = gx_get_time_ms() - t1;
        //printf("%d\n", t2);
        APP_EVENT app_event = {
            .event_id = LVP_DENOISE_DONE_ID,
            .ctx_index = context->ctx_index
        };
        LvpTriggerAppEvent(&app_event);
        LvpAudioInUpdateReadIndex(1);
    }
}

static void _DenoiseModeDone(LVP_MODE_TYPE next_mode)
{
    printf(LOG_TAG"Exit denoise mode\n");
}

void LvpDenoiseGetAudioInBuffer(void **addr, int *len)
{
    if((addr == NULL) || (len == NULL)) {
        return;
    }

    *addr = LvpGetMicBufferAddr();
    *len = LvpGetMicBufferSize() / LvpGetMicChannelNum();
    printf("addr :0x%x   len=%d\n", *addr, *len);
}

static unsigned int input_index = 0;
int LvpDenoiseUpdateContext(void)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;

    LvpGetContext(input_index, &context, &ctx_size);
    context->ctx_index = input_index++;
    LvpQueuePut(&context_queue, (unsigned char *)&context);
    return 0;
}

static int _DenoiseModeBufferInit(void)
{
#ifndef CONFIG_LVP_USE_BUFFER_V2
    return LvpInitBuffer();
#else
    return LvpInitDenoiseBuffer();
#endif
}
//-------------------------------------------------------------------------------------------------

const LVP_MODE_INFO lvp_denoise_mode_info = {
    .type = LVP_MODE_DENOISE,
    .buffer_init = _DenoiseModeBufferInit,
    .init = _DenoiseModeInit,
    .done = _DenoiseModeDone,
    .tick = _DenoiseModeTick,
};
