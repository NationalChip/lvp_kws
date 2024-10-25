/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mic_test.c:
 *
 */

#include <stdio.h>
#include <string.h>
#include <types.h>
#include <autoconf.h>
#include <lvp_buffer.h>
#include <board_misc_config.h>
#include "lvp_mic_test.h"
#include "csky_const_structs.h"

#define CONTEXT_LENGTH      (PCM_FRAME_SIZE * CONFIG_LVP_PCM_FRAME_NUM_PER_CONTEXT)
#define CHANNEL_LENGTH      (PCM_FRAME_SIZE * CONFIG_LVP_PCM_FRAME_NUM_PER_CHANNEL)
#define CALC_LENGTH         (PCM_FRAME_SIZE * 2)

typedef struct {
    uint32_t index;
    float64_t amplitude;
    float64_t level;
    float64_t frequency;
} MIC_TEST_PARA;


static MIC_TEST_PARA s_test_mic_para[2]; // 8002最多就支持2个麦克风
static MIC_TEST_PARA s_test_ref_para[1]; // 8002最多就支持一路回采
static char mic_test_str[128]; // 保持测试结果的字符串, 例如：mic0 fre:1000 level:-24, mic1 fre:0 level:0, ref fre:0 level:0

static int _MicCalcAmplitudeAndFrequency(int16_t *pcm_mic, uint32_t pcm_size, uint32_t sample_rate
        , float64_t *amplitude, float64_t *level, float64_t *frequency)
{
    #define FFT_SIZE 256
    float32_t realf_data[FFT_SIZE] = {0};
    float32_t cmplx_data[FFT_SIZE*2] = {0};

    csky_rfft_fast_instance_f32 S;
    csky_rfft_fast_init_f32(&S, FFT_SIZE);

    uint32_t sample_num = pcm_size < S.fftLenRFFT ? pcm_size : S.fftLenRFFT;

    for (int i = 0; i < sample_num; i++) {
        realf_data[i] = pcm_mic[i] / 32768.f;
    }

    csky_rfft_fast_f32(&S, realf_data, cmplx_data, 0);
    csky_cmplx_mag_f32(cmplx_data, realf_data, S.fftLenRFFT);

    float32_t max_result = 0;
    uint32_t max_index = 0;
    csky_max_f32(realf_data, S.fftLenRFFT, &max_result, &max_index);

    *amplitude = max_result * 2 / sample_num;
    // *level = 20 * csky_log10_f64(*amplitude);
    *level = 20 * log10(*amplitude);
    *frequency = max_index * sample_rate / S.fftLenRFFT;

    return 0;
}

void LvpMicTestProcess(LVP_CONTEXT *context)
{
    LVP_CONTEXT_HEADER *ctx_headler = context->ctx_header;
    if(ctx_headler->mic_num > 2) return;
    if(ctx_headler->ref_num > 1) return;

#ifdef CONFIG_LVP_BUFFER_HAS_MIC
    for (int i = 0; i < ctx_headler->mic_num; i++) {
        for (int j = 0; j <= CONTEXT_LENGTH - CALC_LENGTH; j += CALC_LENGTH) {
            float64_t amplitude;
            float64_t level;
            float64_t frequency;

            _MicCalcAmplitudeAndFrequency(&ctx_headler->mic_buffer[CHANNEL_LENGTH*i+j], CALC_LENGTH, ctx_headler->sample_rate
                    , &amplitude, &level, &frequency);

            s_test_mic_para[i].index++;
            s_test_mic_para[i].amplitude += (amplitude - s_test_mic_para[i].amplitude) / s_test_mic_para[i].index;
            s_test_mic_para[i].level += (level - s_test_mic_para[i].level) / s_test_mic_para[i].index;
            s_test_mic_para[i].frequency += (frequency - s_test_mic_para[i].frequency) / s_test_mic_para[i].index;
        }
    }
#endif
#ifdef CONFIG_LVP_BUFFER_HAS_REF
    for (int i = 0; i < ctx_headler->ref_num; i++) {
        for (int j = 0; j <= CONTEXT_LENGTH - CALC_LENGTH; j += CALC_LENGTH) {
            float64_t amplitude;
            float64_t level;
            float64_t frequency;

            _MicCalcAmplitudeAndFrequency(&ctx_headler->ref_buffer[CHANNEL_LENGTH*i+j], CALC_LENGTH, ctx_headler->sample_rate
                    , &amplitude, &level, &frequency);

            s_test_ref_para[i].index++;
            s_test_ref_para[i].amplitude += (amplitude - s_test_ref_para[i].amplitude) / s_test_ref_para[i].index;
            s_test_ref_para[i].level += (level - s_test_ref_para[i].level) / s_test_ref_para[i].index;
            s_test_ref_para[i].frequency += (frequency - s_test_ref_para[i].frequency) / s_test_ref_para[i].index;
        }
    }
#endif
}

char* LvpMicTestGetResult(void)
{
    // snprintf需要加上 utility_objs += utility/libc/memfile.o
    snprintf(mic_test_str, sizeof(mic_test_str), "mic0 fre:%d level:%d, mic1 fre:%d level:%d, ref fre:%d level:%d\n", 
        (int)s_test_mic_para[0].frequency, (int)s_test_mic_para[0].level, (int)s_test_mic_para[1].frequency, (int)s_test_mic_para[1].level, (int)s_test_ref_para[0].frequency, (int)s_test_ref_para[0].level);

    return mic_test_str;
}

