/* Grus
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mode_fft_recover.c:
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

#include "lvp_mode.h"
#include "driver/dsp/csky_math.h"
#include "driver/dsp/csky_const_structs.h"
#include "app_core/lvp_app_core.h"


#define LOG_TAG "[LVP_FFT_RECOVER]"

#define OUT_WAV_NUM 8
#define OUT_WAV_LENGTH 160
float ifft_input[514];
float ifft_output[512];
short wav_buffer[OUT_WAV_LENGTH * OUT_WAV_NUM] = {0};
static unsigned char s_index_queue_buffer[8 * sizeof(unsigned int)];
static LVP_QUEUE s_index_queue;

//=================================================================================================

float synthesis_window [160] = {
1.003311,1.001017,0.998729,0.996451,0.994188,0.991942,0.989718,0.987520,0.985351,0.983215,
0.981116,0.979055,0.977038,0.975066,0.973142,0.971269,0.969450,0.967687,0.965983,0.964339,
0.962758,0.961242,0.959793,0.958413,0.957103,0.955864,0.954699,0.953608,0.952593,0.951656,
0.950796,0.950015,0.949315,0.948694,0.948156,0.947699,0.947324,0.947033,0.946824,0.946699,
0.946657,0.946699,0.946824,0.947033,0.947324,0.947699,0.948156,0.948694,0.949315,0.950015,
0.950796,0.951656,0.952593,0.953608,0.954699,0.955864,0.957103,0.958413,0.959793,0.961242,
0.962758,0.964339,0.965983,0.967687,0.969450,0.971269,0.973142,0.975066,0.977038,0.979055,
0.981116,0.983215,0.985351,0.987520,0.989718,0.991942,0.994188,0.996451,0.998729,1.001017,
1.009795,1.012129,1.014479,1.016840,1.019208,1.021580,1.023949,1.026312,1.028665,1.031003,
1.033322,1.035618,1.037885,1.040120,1.042318,1.044475,1.046586,1.048649,1.050658,1.052609,
1.054499,1.056323,1.058079,1.059762,1.061368,1.062896,1.064340,1.065699,1.066969,1.068148,
1.069233,1.070222,1.071113,1.071903,1.072592,1.073177,1.073658,1.074033,1.074301,1.074463,
1.074516,1.074463,1.074301,1.074033,1.073658,1.073177,1.072592,1.071903,1.071113,1.070222,
1.069233,1.068148,1.066969,1.065699,1.064340,1.062896,1.061368,1.059762,1.058079,1.056323,
1.054499,1.052609,1.050658,1.048649,1.046586,1.044475,1.042318,1.040120,1.037885,1.035618,
1.033322,1.031003,1.028665,1.026312,1.023949,1.021580,1.019208,1.016840,1.014479,1.012129,
};
float hann_400[400] = {0.080000, 0.080057, 0.080227, 0.080511, 0.080908, 0.081418, 0.082042, 0.082778, 0.083627, 0.084589, 0.085663, 0.086850, 0.088148, 0.089557, 0.091078, 0.092710, 0.094452, 0.096304, 0.098265, 0.100335, 0.102514, 0.104801, 0.107195, 0.109696, 0.112303, 0.115015, 0.117833, 0.120755, 0.123780, 0.126907, 0.130137, 0.133468, 0.136899, 0.140430, 0.144059, 0.147786, 0.151609, 0.155529, 0.159543, 0.163651, 0.167852, 0.172145, 0.176529, 0.181002, 0.185564, 0.190213, 0.194949, 0.199770, 0.204674, 0.209662, 0.214731, 0.219880, 0.225108, 0.230414, 0.235797, 0.241254, 0.246785, 0.252388, 0.258063, 0.263807, 0.269619, 0.275498, 0.281442, 0.287450, 0.293520, 0.299651, 0.305841, 0.312089, 0.318393, 0.324752, 0.331164, 0.337628, 0.344142, 0.350703, 0.357312, 0.363966, 0.370663, 0.377402, 0.384181, 0.390998, 0.397852, 0.404741, 0.411664, 0.418618, 0.425603, 0.432615, 0.439654, 0.446718, 0.453805, 0.460913, 0.468040, 0.475186, 0.482347, 0.489522, 0.496710, 0.503909, 0.511116, 0.518331, 0.525551, 0.532775, 0.540000, 0.547225, 0.554449, 0.561669, 0.568884, 0.576091, 0.583290, 0.590478, 0.597653, 0.604815, 0.611960, 0.619087, 0.626195, 0.633282, 0.640346, 0.647385, 0.654397, 0.661382, 0.668336, 0.675259, 0.682148, 0.689002, 0.695819, 0.702598, 0.709337, 0.716034, 0.722688, 0.729297, 0.735859, 0.742372, 0.748836, 0.755248, 0.761607, 0.767911, 0.774159, 0.780349, 0.786480, 0.792551, 0.798558, 0.804502, 0.810381, 0.816193, 0.821937, 0.827612, 0.833215, 0.838746, 0.844204, 0.849586, 0.854892, 0.860120, 0.865269, 0.870338, 0.875326, 0.880230, 0.885051, 0.889787, 0.894436, 0.898998, 0.903471, 0.907855, 0.912148, 0.916349, 0.920457, 0.924471, 0.928391, 0.932214, 0.935941, 0.939571, 0.943101, 0.946532, 0.949863, 0.953093, 0.956221, 0.959246, 0.962167, 0.964985, 0.967697, 0.970304, 0.972805, 0.975199, 0.977486, 0.979665, 0.981735, 0.983696, 0.985548, 0.987290, 0.988922, 0.990443, 0.991852, 0.993150, 0.994337, 0.995411, 0.996373, 0.997222, 0.997959, 0.998582, 0.999092, 0.999489, 0.999773, 0.999943, 1.000000, 0.999943, 0.999773, 0.999489, 0.999092, 0.998582, 0.997959, 0.997222, 0.996373, 0.995411, 0.994337, 0.993150, 0.991852, 0.990443, 0.988922, 0.987290, 0.985548, 0.983696, 0.981735, 0.979665, 0.977486, 0.975199, 0.972805, 0.970304, 0.967697, 0.964985, 0.962167, 0.959246, 0.956220, 0.953093, 0.949863, 0.946532, 0.943101, 0.939570, 0.935941, 0.932214, 0.928391, 0.924471, 0.920457, 0.916349, 0.912148, 0.907855, 0.903471, 0.898998, 0.894436, 0.889787, 0.885051, 0.880230, 0.875326, 0.870338, 0.865269, 0.860120, 0.854892, 0.849586, 0.844203, 0.838746, 0.833215, 0.827612, 0.821937, 0.816193, 0.810381, 0.804502, 0.798558, 0.792550, 0.786480, 0.780349, 0.774159, 0.767911, 0.761607, 0.755248, 0.748836, 0.742372, 0.735858, 0.729297, 0.722688, 0.716034, 0.709337, 0.702598, 0.695819, 0.689002, 0.682148, 0.675258, 0.668336, 0.661381, 0.654397, 0.647385, 0.640346, 0.633282, 0.626195, 0.619087, 0.611960, 0.604814, 0.597653, 0.590478, 0.583290, 0.576091, 0.568883, 0.561669, 0.554449, 0.547225, 0.540000, 0.532775, 0.525551, 0.518331, 0.511116, 0.503909, 0.496710, 0.489522, 0.482347, 0.475185, 0.468040, 0.460913, 0.453805, 0.446718, 0.439654, 0.432615, 0.425603, 0.418618, 0.411664, 0.404741, 0.397852, 0.390998, 0.384181, 0.377402, 0.370663, 0.363966, 0.357312, 0.350703, 0.344141, 0.337628, 0.331164, 0.324752, 0.318393, 0.312089, 0.305841, 0.299651, 0.293520, 0.287449, 0.281442, 0.275497, 0.269619, 0.263807, 0.258063, 0.252388, 0.246785, 0.241254, 0.235796, 0.230414, 0.225108, 0.219880, 0.214731, 0.209662, 0.204674, 0.199770, 0.194949, 0.190213, 0.185564, 0.181002, 0.176529, 0.172145, 0.167852, 0.163651, 0.159543, 0.155529, 0.151609, 0.147785, 0.144059, 0.140429, 0.136899, 0.133468, 0.130137, 0.126907, 0.123779, 0.120754, 0.117833, 0.115015, 0.112303, 0.109696, 0.107195, 0.104801, 0.102514, 0.100335, 0.098265, 0.096304, 0.094452, 0.092710, 0.091078, 0.089557, 0.088148, 0.086850, 0.085663, 0.084589, 0.083627, 0.082778, 0.082042, 0.081418, 0.080908, 0.080511, 0.080227, 0.080057, };

static float last[240];
#define PARAM1 400
#define PARAM2 160
#define PARAM3 240
#define PARAM4 80
#define RECOVER_FFT_POINTS 257
#define RECOVER_WAV_POINTS 160


static float32_t s_last_y = 0;
static int _Lfilter(float32_t *in, float32_t *out, int len)
{
    for(int i = 0; i < len; i++) {
        out[i] = in[i] + 0.97f * s_last_y;
        s_last_y = out[i];
    }

    return 0;
}

static int _DeHanmingWindow(float32_t *in, float32_t *out)
{
    for(int i = 0; i < PARAM1; i++) {

        out[i] = in[i] * hann_400[i];
    }
    for(int i = 0; i < PARAM2; i++) {
        out[i] = (out[i] + last[i]) * synthesis_window[i];
    }

    for(int i = PARAM2;i < PARAM3; i++) {
        last[i-PARAM2] = last[i];
    }

    for(int i = 0; i < PARAM4; i++) {
        last[i] += out[PARAM2+i];
    }

    for(int i = PARAM4; i < PARAM3; i++) {
        last[i] = out[PARAM2+i];
    }

    return 0;
}

int _RecoverWav(float *amp, float *phase, float *ifft_tmp_buffer, float *ifft_out_buffer, short *wav)
{

    csky_rfft_fast_f32(&csky_rfft_sR_f32_len512, ifft_tmp_buffer, ifft_out_buffer, 1); // 10ms,  512 + 2 个点 ，最后赋值两个0

    _DeHanmingWindow(ifft_out_buffer, ifft_out_buffer);
    _Lfilter(ifft_out_buffer, ifft_out_buffer, RECOVER_WAV_POINTS);
    for(int i = 0; i < RECOVER_WAV_POINTS; i++) {
        wav[i] = (short)(ifft_out_buffer[i]);
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------


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
          //printf (LOG_TAG"Ctx:%d, Vad:%d\n", context->ctx_index, context->G_vad);
      }
      LvpQueuePut(&s_index_queue, (unsigned char *)&(context->ctx_index));
      LvpAudioInUpdateReadIndex(1);
   }

   return 0;
}

static int out_wav_offset = 0;
static void _RecoverTick(void)
{
    int index;
    if (LvpQueueGet(&s_index_queue, (unsigned char *)&index)) {
        LVP_CONTEXT *context;
        unsigned int ctx_size;
        LvpGetContext(index, &context, &ctx_size);
        LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
        unsigned int send_len = ctx_header->fft_buffer_size / ctx_header->fft_frame_num_per_channel * ctx_header->pcm_frame_num_per_context;

        unsigned char *send_buffer = (unsigned char *)ctx_header->fft_buffer + \
                                     send_len * (context->ctx_index % (ctx_header->fft_frame_num_per_channel / ctx_header->pcm_frame_num_per_context));
        gx_dcache_invalid_range((unsigned int*)send_buffer, send_len);


        for(int i = 0; i < 512; i++) {
            *(ifft_input + i) = (*((int*)send_buffer + i)) * 1.f;
        }
        *(ifft_input + 512) = 0.f;
        *(ifft_input + 513) = 0.f;

        int start = out_wav_offset;

        _RecoverWav(NULL, NULL, ifft_input, ifft_output, \
                   &wav_buffer[out_wav_offset * OUT_WAV_LENGTH]);

        ++out_wav_offset;
        if (out_wav_offset >= OUT_WAV_NUM)
            out_wav_offset %= OUT_WAV_NUM;

        APP_EVENT event = {
            .event_id = OUT_WAV_LENGTH,
            .ctx_index = (unsigned int)&wav_buffer[start * OUT_WAV_LENGTH],
        };
        LvpTriggerAppEvent(&event);

        UartRecordChannelTask((unsigned char *)&wav_buffer[start * OUT_WAV_LENGTH], \
                              OUT_WAV_LENGTH * sizeof(short), RECORD_CHANNEL_MIC0);
    }
}

static int _FftRecoverModeInit(LVP_MODE_TYPE mode)
{
    printf("FFT recover Mode Init\n");
    LvpQueueInit(&s_index_queue, s_index_queue_buffer,
                sizeof(s_index_queue_buffer), sizeof(unsigned int));

    if (0 != LvpAudioInInit(_LvpAudioInRecordCallback)) {
        printf(LOG_TAG"LvpAudioInInit Failed\n");
        return -1;
    }
    UartRecordInit(-1, 0);

    return 0;
}

static void _FftRecoverModeTick(void)
{
    _RecoverTick();
    UartRecordTick();
}

static void _FftRecoverModeDone(LVP_MODE_TYPE next_mode)
{
    printf(LOG_TAG"Exit FFT recover mode\n");
}

static int _FftRecoverModeBufferInit(void)
{
    return LvpInitBuffer();
}
//-------------------------------------------------------------------------------------------------

const LVP_MODE_INFO lvp_fft_recover_mode_info = {
    .type = LVP_MODE_FFT_RECOVER,
    .buffer_init = _FftRecoverModeBufferInit,
    .init = _FftRecoverModeInit,
    .done = _FftRecoverModeDone,
    .tick = _FftRecoverModeTick,
};
