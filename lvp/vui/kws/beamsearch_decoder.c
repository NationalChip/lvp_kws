/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * ctc_decoder.c: The Process For Kws
 *
 */

#include <autoconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <driver/gx_timer.h>
#include <driver/gx_cache.h>

#include <lvp_system_init.h>
#include <lvp_context.h>
#include <lvp_param.h>
#include <ctc_model.h>
#include <kws_list.h>
#include <lm_model.h>
#include <lst.h>
#include "decoder.h"
#include "kws_strategy.h"
#include "lm_decoder.h"
#include "lvp_buffer.h"

#define LOG_TAG "[LVP_BEAMSEARCH]"
#define BLOCK_SCORE

static int s_ctc_index = 0;
static int s_ctc_decoder_head;

#ifndef CONFIG_LVP_USE_BUFFER_V2
# if (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH < 40)
static float s_ctc_decoder_window[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH][CONFIG_KWS_MODEL_OUTPUT_LENGTH] ALIGNED_ATTR(16) DRAM0_AUDIO_IN_ATTR;
# else
static float s_ctc_decoder_window[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH][CONFIG_KWS_MODEL_OUTPUT_LENGTH] ALIGNED_ATTR(16);
# endif
#else
static float (*s_ctc_decoder_window)[CONFIG_KWS_MODEL_OUTPUT_LENGTH];
#endif

LANGUAGE_MODEL_PARAM g_demo_lm_param;
LANGUAGE_MODEL_PARAM *g_lm_param = NULL;

static PREFIX_LIST prefix_list[CONFIG_BEAM_SIZE];
static float s_decode_frame[65];

LVP_KWS_PARAM_LIST g_kws_list;

static int blank_count = 0;

void LvpInitCtcDemoLanguageModel(void)
{
    g_demo_lm_param.lm       = &lm_bin[0];
    g_demo_lm_param.lm_size  = lm_bin_len;
    g_demo_lm_param.lst      = &lst_bin[0];
    g_demo_lm_param.lst_size = lst_bin_len;

    g_lm_param = &g_demo_lm_param;
    KwsCtcBeamSearchLmInit(g_lm_param->lm, g_lm_param->lst);
}

DRAM0_STAGE2_SRAM_ATTR void ResetWinodw(void)
{
    for (int n = 0; n < CONFIG_BEAM_SIZE; n++) {
        memset(prefix_list[n].prefix_set_prev, CONFIG_KWS_MODEL_OUTPUT_LENGTH, CONFIG_PREFIX_CHAR_LENGTH);
        prefix_list[n].probs_nb_prev = 0.0;
        prefix_list[n].probs_b_prev = 1.0;
        prefix_list[n].probs_set_prev = 0.0;
    }
    blank_count = 0;
}

DRAM0_STAGE2_SRAM_ATTR void ResetCtcWinodw(void)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    for (int i = 0; i < CONFIG_KWS_MODEL_DECODER_WIN_LENGTH; i++) {
        for (int j = 0; j < CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1; j++) {
            s_ctc_decoder_window[i][j] = (1 - 0.9999f) / (CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1);
        }
        s_ctc_decoder_window[i][CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1] = 0.9999f;
    }
    s_ctc_decoder_head = CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
    s_ctc_index = 0;

    for (int n = 0; n < CONFIG_BEAM_SIZE; n++) {
        memset(prefix_list[n].prefix_set_prev, CONFIG_KWS_MODEL_OUTPUT_LENGTH, CONFIG_PREFIX_CHAR_LENGTH);
        prefix_list[n].probs_nb_prev = 0.0;
        prefix_list[n].probs_b_prev = 1.0;
        prefix_list[n].probs_set_prev = 0.0;
    }
#endif
}

void LvpInitCtcKws(void)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
# ifdef CONFIG_LVP_USE_BUFFER_V2
    s_ctc_decoder_window = LvpGetCtcDecoderWindowAddr();
    if(!s_ctc_decoder_window)
    {
        printf("GetCtcDecoderWindowAddr failed\n");
        return;
    }
# endif
    g_kws_list.count = sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM);
    g_kws_list.kws_param_list = g_kws_param_list;
    LvpInitSegmentation(g_segmentation_list_len, g_segmentation_list_offset, g_active_index_stash, g_delay_string_stash, SEGMENTATION_DEEPTH);
    ResetWinodw();
    LvpInitCtcDemoLanguageModel();
#endif
}

DRAM0_STAGE2_SRAM_ATTR static int _LvpDoGroupScore(LVP_CONTEXT *context, int index, int group_number, int valid_frame_num, int major)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION

# ifdef CONFIG_LVP_DISABLE_XIP_WHILE_CODE_RUN_AT_SRAM
            LvpXipSuspend();
# endif


# ifdef CONFIG_LVP_DISABLE_XIP_WHILE_CODE_RUN_AT_SRAM
            LvpXipResume();
# endif

# ifdef CONFIG_LVP_ENABLE_BIONIC
            KwsStrategyRunBionic(context, &g_kws_list.kws_param_list[i], score, threshold, &s_ctc_decoder_window[0][0]);
# endif

    int ret = -1;
    int deepth = LvpGetSegmentationDeepth();
    float ctc_score = 0.f;
    unsigned char *labels = NULL;
    int label_length = 0;

    for(int i = 0; i < g_segmentation_list_len[deepth]; i++) {
        LVP_KWS_SEGMENTATION_PARAM temp = g_segmentation_list[i + g_segmentation_list_offset[deepth]];
        if (temp.ctc_enable) {
            labels = (unsigned char *)&temp.labels[0];
            label_length =temp.label_length;

            int idx = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;

            // printf("%x, %d, %x, %d, %d, %d, %x, %d \n",
            //       &s_ctc_decoder_window[idx][0]
            //             , valid_frame_num - idx// - 1
            //             , &s_ctc_decoder_window[0][0]
            //             , idx// - 1
            //             , CONFIG_KWS_MODEL_OUTPUT_LENGTH
            //             , CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1
            //             , labels
            //             , label_length);

            ctc_score = LvpFastCtcBlockScore(&s_ctc_decoder_window[idx][0]
                        , valid_frame_num - idx// - 1
                        , &s_ctc_decoder_window[0][0]
                        , idx// - 1
                        , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                        , CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1
                        , labels
                        , label_length);
            int fix_score = ctc_score * 10;

            if (fix_score >= temp.threshold) {
                printf("ctc[%d]\n", temp.id);
                LvpSegmentationCtcMatched(i, temp.next);
                ResetWinodw();
                ResetCtcWinodw();
                return 0;
            }
        }
        else continue;
    }

    ret = LvpDoBeamSearch(context
                        , g_segmentation_list
                        , prefix_list
                        , s_decode_frame
                        , valid_frame_num
                        , 0
                        , 0
                        , 0);

    if (ret == 0) {
        ResetWinodw();
        //printf("beamsearch == ctc\n");
        //KwsStragegyInsertKwsActivation(i, g_kws_list.kws_param_list[i].kws_value, 1000, NULL);

    }
    else if (ret == 1) {
        ResetCtcWinodw();
        ResetWinodw();
    }
    //KwsStrategyClearThresholdOffset();
    return -1;
#else
    return 0;
#endif
}


DRAM0_STAGE2_SRAM_ATTR int PrepareData(float *rnn_out)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
# ifdef CONFIG_KWS_MODEL_SUPPORT_SOFTMAX
#  ifndef BLOCK_SCORE
    // shift the s_ctc_decoder_window and push the new nonblank frame into end of s_ctc_decoder_window
    memmove(&s_ctc_decoder_window[0][0], &s_ctc_decoder_window[1][0], (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - 1) * CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    memcpy(&s_ctc_decoder_window[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - 1], rnn_out, CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
#  else
    int idx = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
    memcpy(s_decode_frame, rnn_out, CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    memcpy(&s_ctc_decoder_window[idx], rnn_out, CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    ++s_ctc_index;

    if (s_decode_frame[64] > 0.9f)
        blank_count++;
    else
        blank_count = 0;
    if (blank_count >= 3) {
        ResetWinodw();
        cleanDeepth();
    }
#  endif
# else
    float tmp_buffer[CONFIG_KWS_MODEL_OUTPUT_LENGTH];

    // softmax
    float total_prob = 0.f;
    for (int i = 0; i < CONFIG_KWS_MODEL_OUTPUT_LENGTH; i++) {
        tmp_buffer[i] = expf(rnn_out[i]);
        total_prob += tmp_buffer[i];
    }
#  ifndef BLOCK_SCORE
    // shift the s_ctc_decoder_window and push the new nonblank frame into end of s_ctc_decoder_window
    memmove(&s_ctc_decoder_window[0][0], &s_ctc_decoder_window[1][0], (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - 1) * CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    for (int i = 0; i < CONFIG_KWS_MODEL_OUTPUT_LENGTH; i++) {
        s_ctc_decoder_window[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - 1][i] = tmp_buffer[i] / total_prob;
    }
#  else
    int idx = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
    for (int i = 0; i < CONFIG_KWS_MODEL_OUTPUT_LENGTH; i++) {
        s_ctc_decoder_window[idx][i] = tmp_buffer[i] / total_prob;
    }
    s_ctc_index++;
#  endif
# endif
    return CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
#else
    return 0;
#endif
}

DRAM0_STAGE2_SRAM_ATTR LVP_KWS_PARAM_LIST *LvpGetKwsParamList(void)
{
    return &g_kws_list;
}

DRAM0_STAGE2_SRAM_ATTR int LvpDoKwsScore(LVP_CONTEXT *context)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
# ifdef CONFIG_MCU_ENABLE_DYNAMICALLY_ADJUSTMENT_CPU_FREQUENCY
    LvpDynamiciallyAdjustCpuFrequency(CONFIG_CPU_MAX_FREQUENCY);
# endif
    context->kws = 0;// 挪到此处的原因是这段代码执行时间比较久,主要是 softmax
    gx_dcache_invalid_range((uint32_t *)context->snpu_buffer, context->ctx_header->snpu_buffer_size);
    float *rnn_out = (float *)LvpCTCModelGetSnpuOutBuffer(context->snpu_buffer);

# ifdef CONFIG_ENABLE_CTC_SOFTMAX_CYCLE_STATISTIC
    int start_softmax_ms = gx_get_time_ms();
# endif

    int valid_frame_num = PrepareData(rnn_out);
# ifdef CONFIG_ENABLE_CTC_SOFTMAX_CYCLE_STATISTIC
    int end_softmax_ms = gx_get_time_ms();
    printf ("softmax:%d ms\n", end_softmax_ms - start_softmax_ms);
# endif

# ifdef CONFIG_ENABLE_CTC_DECODER_CYCLE_STATISTIC
    int start_ms = gx_get_time_ms();
# endif
    // Detect Major Kws
    _LvpDoGroupScore(context, 0, 1, valid_frame_num, 1);

# ifdef CONFIG_ENABLE_CTC_DECODER_CYCLE_STATISTIC
    int end_ms = gx_get_time_ms();
    printf ("decoder:%d ms\n", end_ms - start_ms);
# endif

# ifdef CONFIG_MCU_ENABLE_DYNAMICALLY_ADJUSTMENT_CPU_FREQUENCY
    LvpDynamiciallyAdjustCpuFrequency(CONFIG_CPU_MINI_FREQUENCY);
# endif

#endif
    return 0;
}
