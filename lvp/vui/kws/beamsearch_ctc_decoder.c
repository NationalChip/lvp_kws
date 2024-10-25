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
#include <lvp_buffer.h>
#include <lvp_param.h>
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
#include <ctc_model.h>
#include <kws_list.h>
#endif

#ifdef CONFIG_ENABLE_LANGUAGE_MODEL
#include <lm_model.h>
#include <lst.h>
#endif

#ifdef CONFIG_ENABLE_KWS_LEVEL_DECODER
# include "kws_level_decoder.h"
#endif
#include "decoder.h"
#include "kws_strategy.h"
#include "lm_decoder.h"
#include "lvp_buffer.h"

#define LOG_TAG "[LVP_CTC]"
#define BLOCK_SCORE

#ifdef CONFIG_VPA_ENABLE_DELAY_TO_DECODE
#define _KWS_DELAY_2_DECODE_NUM_    CONFIG_VPA_DELAY_TO_DECODE_CNT
static KWS_DELAY2DECODE s_kws_delay2decode[sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM)];
int s_max_kws_delay2decode_cnt = 0;
int s_top_delay_cnt = 0;
#endif

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

LANGUAGE_MODEL_PARAM g_demo_lm_param;
LANGUAGE_MODEL_PARAM *g_lm_param = NULL;

static PREFIX_LIST prefix_list[CONFIG_BEAM_SIZE];

LVP_KWS_PARAM_LIST g_kws_list;

void LvpInitCtcDemoLanguageModel(void)
{
#ifdef CONFIG_ENABLE_LANGUAGE_MODEL
    g_demo_lm_param.lm       = &lm_bin[0];
    g_demo_lm_param.lm_size  = lm_bin_len;
    g_demo_lm_param.lst      = &lst_bin[0];
    g_demo_lm_param.lst_size = lst_bin_len;

    g_lm_param = &g_demo_lm_param;
    KwsCtcBeamSearchLmInit(g_lm_param->lm, g_lm_param->lst);
#endif
}

void _ResetDelay2Decode(void)
{
#ifdef CONFIG_VPA_ENABLE_DELAY_TO_DECODE
    s_max_kws_delay2decode_cnt = 0;
    s_top_delay_cnt = 0;
    memset (s_kws_delay2decode, 0, sizeof(s_kws_delay2decode));
    printf (LOG_TAG" ######### clear delay2decode\n");
#endif
}


void ResetCtcWinodw(void)
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


void ResetCtcWinodwUser(int start, int len)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION

    int head_idx = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH + start;

    if (len <= CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - head_idx) {
        for (int i = head_idx; i < len; i++) {
            for (int j = 0; j < CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1; j++) {
                s_ctc_decoder_window[i][j] = (1 - 0.9999f) / (CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1);
            }
            s_ctc_decoder_window[i][CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1] = 0.9999f;
        }
    } else {
        for (int i = head_idx; i < CONFIG_KWS_MODEL_DECODER_WIN_LENGTH; i++) {
            for (int j = 0; j < CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1; j++) {
                s_ctc_decoder_window[i][j] = (1 - 0.9999f) / (CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1);
            }
            s_ctc_decoder_window[i][CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1] = 0.9999f;
        }
        for (int i = 0; i <= len - (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - head_idx); i++) {
            for (int j = 0; j < CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1; j++) {
                s_ctc_decoder_window[i][j] = (1 - 0.9999f) / (CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1);
            }
            s_ctc_decoder_window[i][CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1] = 0.9999f;
        }
    }

    // s_ctc_decoder_head = CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
    // s_ctc_index = 0;

    for (int n = 0; n < CONFIG_BEAM_SIZE; n++) {
        memset(prefix_list[n].prefix_set_prev, CONFIG_KWS_MODEL_OUTPUT_LENGTH, CONFIG_PREFIX_CHAR_LENGTH);
        prefix_list[n].probs_nb_prev = 0.0;
        prefix_list[n].probs_b_prev = 1.0;
        prefix_list[n].probs_set_prev = 0.0;
    }
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
    LvpInitCtcDemoLanguageModel();
    KwsLevelDecoderInit();
# ifdef CONFIG_VPA_ENABLE_DELAY_TO_DECODE
    s_max_kws_delay2decode_cnt = 0;
    s_top_delay_cnt = 0;
    memset (s_kws_delay2decode, 0, sizeof(s_kws_delay2decode));
# endif

#endif
}

void LvpPrintCtcKwsList(void)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    g_kws_list.count = sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM);
    g_kws_list.kws_param_list = g_kws_param_list;
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
# ifdef CONFIG_ENABLE_LANGUAGE_MODEL
    printf (LOG_TAG"Alpha:%d\n", CONFIG_KWS_MODEL_BEAMSEARCH_AND_LM_ALPHA);
    printf (LOG_TAG"Beta:%d\n", CONFIG_KWS_MODEL_BEAMSEARCH_AND_LM_BETA);
# endif

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

DRAM0_STAGE2_SRAM_ATTR static int _LvpDoGroupScore(LVP_CONTEXT *context, int index, int group_number, int valid_frame_num, int major)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION

    for (int i = index; i < group_number; i++) {
        if (valid_frame_num < sizeof(g_kws_list.kws_param_list[i].label_length)) continue;

# ifdef CONFIG_VPA_ENABLE_ONLY_PRINTF_SCORE_WITHOUT_ACTIVATE
# else
#  ifdef CONFIG_ENABLE_KWS_LEVEL_DECODER
        // static int delay_cnt = 0;
        static int last_decoder_level = 0;

        int kws_level_type = g_kws_list.kws_param_list[i].major & KWS_LEVEL_MASK;//(0x1ff<<7);
        int kws_decoder_level = GetKwsDecoderLevel();

        // TOP命中后，统一延迟4个ctx
        if (last_decoder_level != kws_decoder_level && (kws_decoder_level != 0)) {
            last_decoder_level = kws_decoder_level;
            s_top_delay_cnt = 0;
            printf(LOG_TAG"======= Hit top, need delay\n");
        }
        if ((kws_decoder_level != 0) && (major == 0)) {
            static int last_ctx_index = 0;
            if (last_ctx_index != context->ctx_index) {
                s_top_delay_cnt++;
                last_ctx_index = context->ctx_index;
                // printf(LOG_TAG"======= ctx: %d, delay_cnt: %d\n", context->ctx_index, s_top_delay_cnt);
            }
            if (s_top_delay_cnt <= 4) {
                continue;
            }
        }

        // 先保证top激活
        if ( (((kws_decoder_level & KWS_LEVEL_TOP_MASK) == 0)) 
            && (KWS_LEVEL_END == (kws_level_type & KWS_LEVEL_END))
            && (major == 0) ) {
            if (kws_decoder_level != 0) printf(LOG_TAG"kws_decoder_level: %x, kws_level_type: %x\n", kws_decoder_level, kws_level_type);
            // printf(LOG_TAG"###$$$### %s\n", g_kws_list.kws_param_list[i].kws_words);
            continue;
        }

        // top 激活后，只判断该top下的kws
        if (((kws_level_type & kws_decoder_level) != kws_decoder_level) && (kws_decoder_level != 0) && (major == 0) && (kws_level_type != 0)) {
            // printf(LOG_TAG"## %s, %x, %x\n", g_kws_list.kws_param_list[i].kws_words, kws_level_type & KWS_LEVEL_TOP_MASK, kws_decoder_level);
            continue;
        }

        // top 激活后，该 top 不再参与判断
        if ((kws_level_type == kws_decoder_level) && (kws_decoder_level != 0) && (major == 0) ) {
            continue;
        }
        // if(kws_decoder_level) printf(LOG_TAG"#$$# %s, %x, %x\n", g_kws_list.kws_param_list[i].kws_words, kws_level_type & KWS_LEVEL_TOP_MASK, kws_decoder_level);

#  endif
# endif
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

            int idx = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
            int len1 = valid_frame_num - idx;
            int len2 = idx;

# ifdef CONFIG_ENABLE_KWS_LEVEL_DECODER
            if (label_length < 6) {
                int drop_len = CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - label_length*4;// - label_length/2;
                idx = (s_ctc_index + drop_len) % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
                if (idx < drop_len) {
                    len1 = valid_frame_num - drop_len;
                    len2 = 0;
                } else {
                    len1 = valid_frame_num - idx;
                    len2 = idx - drop_len;
                }
            }
# endif

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
            int score_index = 0;
            float ctc_score = LvpFastCtcBlockScorePlus(&s_ctc_decoder_window[idx][0]
                            , len1// - 1
                            , &s_ctc_decoder_window[0][0]
                            , len2// - 1
                            , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                            , CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1
                            , labels
                            , label_length
                            , &score_index);

# endif

# ifdef CONFIG_LVP_DISABLE_XIP_WHILE_CODE_RUN_AT_SRAM
            LvpXipResume();     // 这个接口会影响大约2ms左右的npu算力，详细原因请见问题号：319541
# endif

            float threshold;
# ifdef CONFIG_LVP_KWS_HUAWEI_WATCH_V0DOT1DOT0_2020_1109
            threshold = ((float)(g_kws_list.kws_param_list[i].threshold)/10.f) + 3;
            if (threshold < 80) threshold = 81;
# else
#  ifdef CONFIG_KWS_TYPE_HYBRID
            if(LvpModelGetUseXipModelFlag()) {
                threshold = ((float)(g_kws_list.kws_param_list[i].xip_threshold)/10.f);
            } else {
                threshold = ((float)(g_kws_list.kws_param_list[i].threshold)/10.f);
            }
#  else
            threshold = ((float)(g_kws_list.kws_param_list[i].threshold)/10.f);
#  endif
# endif
            // 开机时的唤醒需要调整分数
# ifdef CONFIG_LVP_ENABLE_CTC_SCORE_COMPENSATOR
            float score = KwsStrategyFineTuneScore(context, ctc_score);
# else
            float score = ctc_score;
# endif

# ifdef CONFIG_VPA_ENABLE_ONLY_PRINTF_SCORE_WITHOUT_ACTIVATE
            printf (LOG_TAG"ctx_id:%d, Kws:%s[%d], score:%d\n"
                    , context->ctx_index
                    , g_kws_list.kws_param_list[i].kws_words
                    , g_kws_list.kws_param_list[i].kws_value
                    , (int)(10*score));
            continue;
# endif
# if 1
            if (score > threshold - 3) {
                printf (LOG_TAG"ctx_id:%d, Kws:%s[%d], score:%d, score_index:%d\n"
                        , context->ctx_index
                        , g_kws_list.kws_param_list[i].kws_words
                        , g_kws_list.kws_param_list[i].kws_value
                        , (int)(10*score)
                        , score_index);
            }
# endif

# ifdef CONFIG_LVP_ENABLE_BIONIC
            KwsStrategyRunBionic(context, &g_kws_list.kws_param_list[i], score, threshold, &s_ctc_decoder_window[0][0]);
# endif

            if (score > (threshold + KwsStrategyGetThresholdOffset(context, threshold)) && score < 120.f) {
                if ((g_kws_list.kws_param_list[i].major&0xf) == 1) {
# ifdef CONFIG_ENABLE_CTC_JUDGE_TO_REDUCE_FALSE_ACTIVATION  // 误激活判断
                    int wrong_flag = LvpFastCtcJudge(&s_ctc_decoder_window[idx][0]
                                , len1// - 1
                                , &s_ctc_decoder_window[0][0]
                                , len2// - 1
                                , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                                , CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1
                                , labels
                                , label_length
                                , threshold);

                    if (wrong_flag == 1) {
                        printf ("False ctx:%d,Kws:%s[%d],th:%d,S:%d,%d\n"
                                , context->ctx_index
                                , g_kws_list.kws_param_list[i].kws_words
                                , g_kws_list.kws_param_list[i].kws_value
                                , g_kws_list.kws_param_list[i].threshold
                                , (int)(10*score)
                                , (int)(10*(threshold + KwsStrategyGetThresholdOffset(context, threshold))));

                        ResetCtcWinodw();
                        KwsStrategyReset();
                        return 0;
                    }
# endif
                    context->kws = g_kws_list.kws_param_list[i].kws_value;
                    printf (LOG_TAG"[CTC] Activation ctx:%d,Kws:%s[%d],th:%d,S:%d,%d\n"
                            , context->ctx_index
                            , g_kws_list.kws_param_list[i].kws_words
                            , g_kws_list.kws_param_list[i].kws_value
                            , (int)(10*threshold)//g_kws_list.kws_param_list[i].threshold
                            , (int)(10*score)
                            , (int)(10*(threshold + KwsStrategyGetThresholdOffset(context, threshold))));
                    ResetCtcWinodw();
                    KwsStrategyReset();
                    KwsStrategyClearThresholdOffset();
                    return 0;
                }
                else {
# if 0
                    printf (LOG_TAG" FL ctx:%d,Kws:%s[%d],th:%d,S:%d,%d\n"
                            , context->ctx_index
                            , g_kws_list.kws_param_list[i].kws_words
                            , g_kws_list.kws_param_list[i].kws_value
                            , (int)(10*threshold)//g_kws_list.kws_param_list[i].threshold
                            , (int)(10*score)
                            , (int)(10*(threshold + KwsStrategyGetThresholdOffset(context, threshold))));
# endif

# ifdef CONFIG_VPA_ENABLE_DELAY_TO_DECODE
                    int delay_2_decode_num = (g_kws_list.kws_param_list[i].major & KWS_TYPE_DEALY2DECODE_MASK) >> 16;
                    if (delay_2_decode_num == 0) delay_2_decode_num = _KWS_DELAY_2_DECODE_NUM_;// 保持原有方案不受影响

                    if (s_max_kws_delay2decode_cnt < delay_2_decode_num) {
                        s_max_kws_delay2decode_cnt = delay_2_decode_num;
                    }

                    if (((g_kws_list.kws_param_list[i].major&KWS_TYPE_DEALY2DECODE) == KWS_TYPE_DEALY2DECODE)
                         && (s_kws_delay2decode[i].cnt < delay_2_decode_num)) {
                    // if (((g_kws_list.kws_param_list[i].major&0x40) == 0x40) && (s_kws_delay2decode[i].cnt < _KWS_DELAY_2_DECODE_NUM_)) {
                        s_kws_delay2decode[i].cnt++;
                        s_kws_delay2decode[i].ctx_index = context->ctx_index;
                        // if (s_max_kws_delay2decode_cnt < s_kws_delay2decode[i].cnt) {
                        //     s_max_kws_delay2decode_cnt = s_kws_delay2decode[i].cnt;
                        // }
                        printf (LOG_TAG" == %d need delay[%d], %d, max:%d\n", i, s_kws_delay2decode[i].cnt, delay_2_decode_num, s_max_kws_delay2decode_cnt);
                        continue;
                    }
# endif
                    int ret = -1;
                    float bm_activation_score = 0;
# ifdef CONFIG_ENABLE_BM_CYCLE_STATISTIC
                    int start_ms = gx_get_time_ms();
# endif
                    ret = LvpDoBeamSearch(context
                                        , g_kws_list
                                        , prefix_list
                                        , &s_ctc_decoder_window[0][0]
                                        , valid_frame_num
                                        , score
                                        , i
                                        , s_ctc_index
                                        , &bm_activation_score);
# ifdef CONFIG_ENABLE_BM_CYCLE_STATISTIC
                    int end_ms = gx_get_time_ms();
                    printf ("BM decoder:%d ms\n", end_ms - start_ms);
# endif
                    if (ret == 0) {
                        // printf("beamsearch == ctc\n");
                        KwsStragegyInsertKwsActivation(i, g_kws_list.kws_param_list[i].kws_value, score, score_index, NULL);
                    } else if (ret > 0) {
                        printf (LOG_TAG"ret: %d,slect kws:%s\n", ret, g_kws_list.kws_param_list[ret].kws_words);
                            KwsStragegyInsertKwsActivation(ret, g_kws_list.kws_param_list[ret].kws_value, bm_activation_score, score_index, NULL);
                    } else {
                        printf(LOG_TAG"beamsearch != ctc [%f]\n", score - threshold);
                        if (score - threshold >= (CONFIG_FILTER_LOW_CTC_SCORES_TH_WHEN_BM_IS_FALSE / 10.f)){
                            KwsStragegyInsertKwsActivation(i, g_kws_list.kws_param_list[i].kws_value, score - 10, score_index, NULL);
                        }
                    }
# ifdef CONFIG_VPA_ENABLE_DELAY_TO_DECODE
                    s_kws_delay2decode[i].cnt++;
                    s_kws_delay2decode[i].ctx_index = context->ctx_index;
# endif
                    // KwsStrategyClearThresholdOffset();
                }
            } else {
# ifdef CONFIG_VPA_ENABLE_DELAY_TO_DECODE
                if ((s_kws_delay2decode[i].cnt > 0) && (s_kws_delay2decode[i].ctx_index != context->ctx_index)) {
                    printf(LOG_TAG"######### clear: %d, cnt:%d, ctx:%d, cur_ctx:%d, score:%f\n", i, s_kws_delay2decode[i].cnt, s_kws_delay2decode[i].ctx_index, context->ctx_index, score);
                    s_kws_delay2decode[i].cnt = 0;
                    s_kws_delay2decode[i].ctx_index = 0;
                }
# endif
            }
        }
    }
    return -1;
#else
    return 0;
#endif
}

int KwsCheckDelay2Decode(LVP_CONTEXT *context)
{
#ifdef CONFIG_VPA_ENABLE_DELAY_TO_DECODE
    int ret = 0;
    for (int n = 0; n < g_kws_list.count; n++) {
        if ((s_kws_delay2decode[n].cnt <= s_max_kws_delay2decode_cnt) && (s_kws_delay2decode[n].cnt !=0)) {

            if (s_kws_delay2decode[n].cnt < s_max_kws_delay2decode_cnt - 1) s_kws_delay2decode[n].cnt = s_max_kws_delay2decode_cnt - 1;
            else s_kws_delay2decode[n].cnt+=1;

            ret = 1;
            printf(LOG_TAG"kws:%s, cnt:%d, max:%d\n"
                        , g_kws_list.kws_param_list[n].kws_words
                        , s_kws_delay2decode[n].cnt
                        , s_max_kws_delay2decode_cnt);
        }
    }

    return ret;
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

# ifdef CONFIG_ENABLE_CTC_SOFTMAX_CYCLE_STATISTIC
    int start_softmax_ms = gx_get_time_ms();
# endif
    int valid_frame_num = PrepareData(rnn_out);
# ifdef CONFIG_ENABLE_CTC_SOFTMAX_CYCLE_STATISTIC
    int end_softmax_ms = gx_get_time_ms();
    printf ("softmax:%d ms\n", end_softmax_ms - start_softmax_ms);
# endif

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
            // context->kws = activation_kws->kws_value;

            int ret = KwsLevelDecoderRun(activation_kws);
            if (0 == ret) {
                context->kws = activation_kws->kws_value;
                printf ("Hit!!!!\n");
                ResetCtcWinodw();
                if (activation_kws->kws_value!=200) { // 200 作为误唤醒词的 kws_value
                    KwsStrategyClearThresholdOffset();
                    context->kws = activation_kws->kws_value;
                }
                // KwsStrategyReset();
            }
            _ResetDelay2Decode();
            KwsStrategyReset();
        }
        KwsLevelDecoderTick(context);
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
