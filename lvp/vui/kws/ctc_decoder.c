/* Grus
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
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
#include <lvp_audio_in.h>
#include <lvp_context.h>
#include <lvp_param.h>
#include "lvp_buffer.h"
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
#include <ctc_model.h>
#include <kws_list.h>
#endif

#include "decoder.h"
#include "kws_strategy.h"

#define LOG_TAG "[LVP_CTC]"
#define BLOCK_SCORE

static int s_ctc_index = 0;
static int s_ctc_decoder_head;
static VUI_KWS_STATE s_state = VUI_KWS_ACTIVE_STATE;
#ifndef CONFIG_LVP_USE_BUFFER_V2
# if (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH < 40)
static float s_ctc_decoder_window[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH][CONFIG_KWS_MODEL_OUTPUT_LENGTH] ALIGNED_ATTR(16) DRAM0_AUDIO_IN_ATTR;
# else
static float s_ctc_decoder_window[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH][CONFIG_KWS_MODEL_OUTPUT_LENGTH] ALIGNED_ATTR(16);
# endif
#else
static float (*s_ctc_decoder_window)[CONFIG_KWS_MODEL_OUTPUT_LENGTH];
#endif

LVP_KWS_PARAM_LIST g_kws_list;

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
#endif
}

#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
static LVP_CTC_MASK s_ctc_threshold_saved[sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM)];
#endif
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
    ResetCtcWinodw();

    for (int i = 0; i < sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM); i++) {
        s_ctc_threshold_saved[i].kws_value = g_kws_param_list[i].kws_value;
        s_ctc_threshold_saved[i].mask_threshold = g_kws_param_list[i].threshold;
    }

    KwsStrategyInit();
#endif
}

void LvpPrintCtcKwsList(void)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION

    printf(LOG_TAG"Kws Version: [%s]\n", LvpCTCModelGetKwsVersion());
# if defined CONFIG_USE_CTC_VERSION_V0DOT0DOT1
    printf(LOG_TAG"Ctc Version: v0.1.1\n");
# elif defined CONFIG_USE_CTC_VERSION_V0DOT0DOT3
    printf(LOG_TAG"Ctc Version: v0.1.3\n");
# else
    printf(LOG_TAG"Ctc Version: error!!!\n");
# endif

    g_kws_list.count = sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM);
    g_kws_list.kws_param_list = g_kws_param_list;
    /*
    printf (LOG_TAG"Demo Kws List [Total:%d]:\n", g_kws_list.count);
    for (int i = 0; i < g_kws_list.count; i++) {
        printf(LOG_TAG"KWS: %s | ", g_kws_list.kws_param_list[i].kws_words);
        printf("KV: %02d | ", g_kws_list.kws_param_list[i].kws_value);
        printf("TRH: %02d | ", g_kws_list.kws_param_list[i].threshold);
# ifdef CONFIG_KWS_TYPE_HYBRID
        printf("XTRH: %02d | ", g_kws_list.kws_param_list[i].xip_threshold);
# endif
        printf("Major: %d | ", g_kws_list.kws_param_list[i].major);
        printf("L: [");
        for (int j = 0; j < g_kws_list.kws_param_list[i].label_length; j++) {
            printf("%d ", g_kws_list.kws_param_list[i].labels[j]);
        }
        printf("]\n");
    }
    */
#endif
}

DRAM0_STAGE2_SRAM_ATTR LVP_KWS_PARAM_LIST *LvpGetKwsParamList(void)
{
    return &g_kws_list;
}

int LvpCtcMask(LVP_CTC_MASK *mask, int mask_num) {
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    for (int i = 0; i < mask_num; i++) {
        for (int j = 0; j < sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM); j++) {
            if (g_kws_param_list[j].kws_value == (mask + i)->kws_value) {
                if ((mask + i)->mask_threshold == 0) {
                    g_kws_param_list[j].threshold = s_ctc_threshold_saved[j].mask_threshold;
                } else if ((mask + i)->mask_threshold > 100 || (mask + i)->mask_threshold < 0) {
                    g_kws_param_list[j].threshold = 100;
                    return -1;
                } else {
                    g_kws_param_list[j].threshold = (mask + i)->mask_threshold;
                }
            }
        }
    }
#endif
    return 0;
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
    memcpy(&s_ctc_decoder_window[idx], rnn_out, CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    s_ctc_index++;
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

static int s_ctc_socre = 0;
int LvpSetCtcScore(int score)
{
    s_ctc_socre = score;

    return 0;
}

int LvpGetCtcScore(void)
{
    return s_ctc_socre;
}

DRAM0_STAGE2_SRAM_ATTR static int _LvpDoGroupScore(LVP_CONTEXT *context, int index, int group_number, int valid_frame_num, int major)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION

    for (int i = index; i < group_number; i++) {
        if (valid_frame_num < sizeof(g_kws_list.kws_param_list[i].label_length)) continue;

        if ((g_kws_list.kws_param_list[i].major&0x1) == major) {
            unsigned char *labels = NULL;
            int label_length = 0;
# ifdef CONFIG_LVP_ENABLE_OPTIMIZE_SHORT_INSTRUCTIONS
            LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
            int runing_time_ms   = context->ctx_index * ctx_header->pcm_frame_num_per_context * ctx_header->frame_length;
            int judgment_time_ms = CONFIG_KWS_MODEL_DECODER_WIN_LENGTH * ctx_header->pcm_frame_num_per_context * ctx_header->frame_length;
            int offset  = CONFIG_LVP_SHORT_INSTRUCTIONS_OFFSET;
            if (runing_time_ms < judgment_time_ms && g_kws_list.kws_param_list[i].kws_value != 100) {
                labels = (unsigned char *)&g_kws_list.kws_param_list[i].labels[offset];
                label_length = g_kws_list.kws_param_list[i].label_length - CONFIG_LVP_SHORT_INSTRUCTIONS_OFFSET;
            } else {
                labels = (unsigned char *)&g_kws_list.kws_param_list[i].labels[0];
                label_length = g_kws_list.kws_param_list[i].label_length;
            }
# else
            labels = (unsigned char *)&g_kws_list.kws_param_list[i].labels[0];
            label_length = g_kws_list.kws_param_list[i].label_length;
# endif

# ifdef CONFIG_LVP_DISABLE_XIP_WHILE_CODE_RUN_AT_SRAM
            LvpXipSuspend();
# endif

# ifndef BLOCK_SCORE
            float ctc_score = LvpFastCtcScore(&s_ctc_decoder_window[0][0]
                            , valid_frame_num
                            , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                            , CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1
                            , labels
                            , label_length);
# else
            int idx = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
            float ctc_score = LvpFastCtcBlockScore(&s_ctc_decoder_window[idx][0]
                            , valid_frame_num - idx// - 1
                            , &s_ctc_decoder_window[0][0]
                            , idx// - 1
                            , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                            , CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1
                            , labels
                            , label_length);

# endif

# ifdef CONFIG_LVP_DISABLE_XIP_WHILE_CODE_RUN_AT_SRAM
            LvpXipResume();
# endif

            float threshold;
#  ifdef CONFIG_KWS_TYPE_HYBRID
            if(LvpModelGetUseXipModelFlag()) {
                threshold = ((float)(g_kws_list.kws_param_list[i].xip_threshold)/10.f);
            } else {
                threshold = ((float)(g_kws_list.kws_param_list[i].threshold)/10.f);
            }
#  else
            threshold = ((float)(g_kws_list.kws_param_list[i].threshold)/10.f);
#  endif
            // 开机时的唤醒需要调整分数
# ifdef CONFIG_LVP_ENABLE_CTC_SCORE_COMPENSATOR
            float score = KwsStrategyFineTuneScore(context, ctc_score);
# else
            float score = ctc_score;
# endif

# if 0
            if (score > threshold - 2) {
                printf (LOG_TAG"ctx_id:%d, Kws:%s[%d], score:%d\n"
                        , context->ctx_index
                        , g_kws_list.kws_param_list[i].kws_words
                        , g_kws_list.kws_param_list[i].kws_value
                        , (int)(10*score));
            }
# endif

# ifdef CONFIG_LVP_ENABLE_CTC_BIONIC
            KwsStrategyRunBionic(context, &g_kws_list.kws_param_list[i], score, threshold, &s_ctc_decoder_window[0][0]);
# endif

            if (score > (threshold + KwsStrategyGetThresholdOffset(context, threshold)) && score <= 120.1f) {
                if ((g_kws_list.kws_param_list[i].major&0x1) == major) {
                    context->kws = g_kws_list.kws_param_list[i].kws_value;
                    LvpSetCtcScore((int)(10*score));
                    printf (LOG_TAG"[CTC] Activation ctx:%d,Kws:%s[%d],th:%d,S:%d,%d\n"
                            , context->ctx_index
                            , g_kws_list.kws_param_list[i].kws_words
                            , g_kws_list.kws_param_list[i].kws_value
                            , (int)(10*threshold)//g_kws_list.kws_param_list[i].threshold
                            , (int)(10*score)
                            , (int)(10*(threshold + KwsStrategyGetThresholdOffset(context, threshold))));
#  ifndef CONFIG_ENABLE_CTC_KWS_AND_BUN_KWS_CASCADE
                    ResetCtcWinodw();
#  endif
                    KwsStrategyReset();
                    KwsStrategyClearThresholdOffset();
                    return 0;
                }
                else {
                    KwsStragegyInsertKwsActivation(i, g_kws_list.kws_param_list[i].kws_value, score, 0, NULL);
# if 1
                    printf (LOG_TAG" FL ctx:%d,Kws:%s[%d],th:%d,S:%d,%d\n"
                            , context->ctx_index
                            , g_kws_list.kws_param_list[i].kws_words
                            , g_kws_list.kws_param_list[i].kws_value
                            , (int)(10*threshold)//g_kws_list.kws_param_list[i].threshold
                            , (int)(10*score)
                            , (int)(10*(threshold + KwsStrategyGetThresholdOffset(context, threshold))));
# endif
                    // KwsStrategyClearThresholdOffset();
                }
            }
        }
    }
    return -1;
#else
    return 0;
#endif
}

int LvpSetVuiKwsStates(VUI_KWS_STATE state)
{
    s_state = state;
    return 0;
}

int LvpGetVuiKwsStates(void)
{
    return s_state;
}

DRAM0_STAGE2_SRAM_ATTR int LvpDoKwsScore(LVP_CONTEXT *context)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
# ifdef CONFIG_MCU_ENABLE_DYNAMICALLY_ADJUSTMENT_CPU_FREQUENCY
    LvpDynamiciallyAdjustCpuFrequency(CONFIG_CPU_MAX_FREQUENCY);
# endif

    context->kws = 0;// 挪到此处的原因是这段代码执行时间比较久,主要是 softmax
    gx_dcache_invalid_range((unsigned int *)context->snpu_buffer, context->ctx_header->snpu_buffer_size);
    float *rnn_out = (float *)LvpCTCModelGetSnpuOutBuffer(context->snpu_buffer);


# ifdef CONFIG_LVP_ADVANCE_HUMAN_VAD_ENABLE
    HumanVadDectectRun(context, rnn_out);
# endif

# ifdef CONFIG_ENABLE_CTC_SOFTMAX_CYCLE_STATISTIC
    int start_softmax_ms = gx_get_time_ms();
# endif
    int valid_frame_num = PrepareData(rnn_out);
# ifdef CONFIG_ENABLE_CTC_SOFTMAX_CYCLE_STATISTIC
    int end_softmax_ms = gx_get_time_ms();
    printf ("softmax:%d ms\n", end_softmax_ms - start_softmax_ms);
# endif
    if (context->ctx_index < LvpGetLogfbankFrameNumPerChannel() / LvpGetPcmFrameNumPerContext() + LvpGetAudioInCtrlStartCtxIndex()) return 0;

    int grout_count  = g_kws_list.count;
    int mod          = context->ctx_index % CONFIG_KWS_MODEL_DECODER_STRIDE_LENGTH;
    int grout_offset = grout_count * mod / CONFIG_KWS_MODEL_DECODER_STRIDE_LENGTH;

    if ((mod + 1) < CONFIG_KWS_MODEL_DECODER_STRIDE_LENGTH) {
        grout_count  = (grout_count * mod + grout_count) / CONFIG_KWS_MODEL_DECODER_STRIDE_LENGTH;
    }

# ifdef CONFIG_ENABLE_CTC_DECODER_CYCLE_STATISTIC
    int start_ms = gx_get_time_ms();
# endif
    // Detect Major Kws
    int ret = _LvpDoGroupScore(context, 0, g_kws_list.count, valid_frame_num, 1);
    if (ret != 0 && s_state == VUI_KWS_ACTIVE_STATE) {
        // Detect Short Kws
        _LvpDoGroupScore(context, grout_offset, grout_count, valid_frame_num, 0);
        // 选择分差大来做最终激活词
        LVP_ACTIVATION_KWS *activation_kws = KwsStragegyRun(context);
        if (NULL != activation_kws) {
            context->kws = activation_kws->kws_value;
            ResetCtcWinodw();
            if (activation_kws->kws_value != 200) { // 200 作为误唤醒词的 kws_value
                KwsStrategyClearThresholdOffset();
                context->kws = activation_kws->kws_value;
            }
            KwsStrategyReset();
        }
    }

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
