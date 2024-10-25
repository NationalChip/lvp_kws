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

#include <lvp_context.h>
#include <lvp_param.h>
#include <lvp_buffer.h>
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
#include <ctc_model.h>
#endif

#ifdef CONFIG_ENABLE_KWS_LEVEL_DECODER
# include "kws_level_decoder.h"
#endif
#include "decoder.h"
#include "kws_strategy.h"

#define LOG_TAG "[ST]"

#define _GRUP_ACTIVATION_KWS_CONUNT_ 8
typedef struct {
    int total;
    LVP_ACTIVATION_KWS activation_kws[_GRUP_ACTIVATION_KWS_CONUNT_];
} LVP_ACTIVATION_KWS_LIST;
static LVP_ACTIVATION_KWS_LIST s_activation_kws_list;

//=================================================================================================

static LVP_KWS_PARAM *_GetKwsParamByKwsIndex(int kws_index, char *kws_words)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    LVP_KWS_PARAM_LIST *kws_list = LvpGetKwsParamList();
    if (NULL == kws_list) return NULL;

    return &kws_list->kws_param_list[kws_index];
#endif

    return NULL;
}
static BIONIC s_bionic = { 0 };
void KwsStrategyInit(void)
{
    memset ((void *)&s_activation_kws_list, 0, sizeof(LVP_ACTIVATION_KWS_LIST));
}

void KwsStrategyReset(void)
{
    memset ((void *)&s_activation_kws_list, 0, sizeof(LVP_ACTIVATION_KWS_LIST));
}

void KwsStrategyClearBunkwsThresholdOffset(void)
{
#ifdef CONFIG_LVP_ENABLE_BUNKWS_BIONIC
    s_bionic.bunkws_context_counter = 0;
    s_bionic.bunkws_threshold_offset = 0.f;
#endif
}

float KwsStrategyGetBunkwsThresholdOffset(LVP_CONTEXT *context, float threshold)
{
    return s_bionic.bunkws_threshold_offset;
}

void KwsStragegyInsertKwsActivation(int kws_index, int kws_value, float kws_score, int score_index, void *priv)
{
    for (int i = 0; i < s_activation_kws_list.total; i++) {
        LVP_ACTIVATION_KWS *activation_kws = &s_activation_kws_list.activation_kws[i];
        if ((activation_kws->kws_value == kws_value) && (activation_kws->kws_index = kws_index)) {
            if (kws_score > activation_kws->kws_score) {
                activation_kws->kws_score = kws_score;
            }
            return;
        }
    }
    int index = s_activation_kws_list.total;
    LVP_ACTIVATION_KWS *activation_kws = &s_activation_kws_list.activation_kws[index];
    activation_kws->kws_index = kws_index;
    activation_kws->kws_value = kws_value;
    activation_kws->kws_score = kws_score;
    activation_kws->score_index = score_index;
    activation_kws->reverse   = priv;
    s_activation_kws_list.total ++;
}

LVP_ACTIVATION_KWS *KwsStragegyRun(LVP_CONTEXT *context)
{
    if (s_activation_kws_list.total == 0) return NULL;

#ifdef CONFIG_VPA_ENABLE_DELAY_TO_DECODE
    extern int KwsCheckDelay2Decode(LVP_CONTEXT *context);
    int ret = KwsCheckDelay2Decode(context);
    if(ret) {
        printf (LOG_TAG" #### Need to delay. ret:%d\n", ret);
        return NULL;
    }
#endif

    LVP_ACTIVATION_KWS *activation_kws   = &s_activation_kws_list.activation_kws[0];
    float score_difference = 0.f;
    float difference = 0.f;
    for (int i = 0; i < s_activation_kws_list.total; i++) {
        int kws_index   = s_activation_kws_list.activation_kws[i].kws_index;
        float kws_score = s_activation_kws_list.activation_kws[i].kws_score;
        LVP_KWS_PARAM *kws_param = _GetKwsParamByKwsIndex(kws_index, NULL);
#ifdef CONFIG_ENABLE_KWS_LEVEL_DECODER
            // static char* last_top_kws;
            int kws_level_type = kws_param->major & KWS_LEVEL_MASK;
            int last_kws_decoder_type = GetKwsDecoderLevel();
            // printf (LOG_TAG"kws_level_type:%x, last_kws_decoder_type:%x\n", kws_level_type, last_kws_decoder_type);
            // printf (LOG_TAG"%d, %d\n", kws_level_type & KWS_LEVEL_TOP_MASK
                                // , (kws_level_type & (~KWS_LEVEL_TOP_MASK)));
            if (0 == kws_level_type
            || (((kws_level_type&last_kws_decoder_type) == last_kws_decoder_type)
            && ((kws_level_type&(last_kws_decoder_type|KWS_LEVEL_END)) == (last_kws_decoder_type|KWS_LEVEL_END)))) {
            } else if (kws_level_type == KWS_LEVEL_MID) {
            } else if (((kws_level_type & KWS_LEVEL_TOP_MASK) != 0) && ((kws_level_type & (~KWS_LEVEL_TOP_MASK)) == 0) ) {
            } else {
                printf (LOG_TAG" IGNORE Hit! Kws:[%s,%d],KV:%d,S:%d,ctx:%d\n"
                        , kws_param->kws_words
                        , kws_param->threshold
                        , kws_param->kws_value
                        , (int)(10*kws_score)
                        , context->ctx_index);
                continue;
            }
#endif

        float threshold;
#ifdef CONFIG_KWS_TYPE_HYBRID
        if(LvpModelGetUseXipModelFlag()) {
            threshold = ((float)(kws_param->xip_threshold)/10.f);
        } else {
            threshold = ((float)(kws_param->threshold)/10.f);
        }
#else
        threshold = ((float)(kws_param->threshold)/10.f);
#endif

        if (NULL != kws_param) {
            if(kws_score < threshold) {
                difference = kws_score - (threshold - 10.f);
                difference = difference / 2;
                if (difference < 0) difference = 0.1f;
            }
            else {
                difference = kws_score - threshold;
            }
            if (difference > score_difference) {
                score_difference = difference;
                activation_kws   = &s_activation_kws_list.activation_kws[i];
            }
            printf (LOG_TAG" Kws:%s[%d],th:%d,S:%d,D:%d\n"
                    , kws_param->kws_words
                    , kws_param->kws_value
                    , (int)(10*threshold)
                    , (int)(10*kws_score)
                    , (int)(10*difference));
        }
    }

    LVP_KWS_PARAM *kws_param = _GetKwsParamByKwsIndex(activation_kws->kws_index, NULL);
    float threshold;
#ifdef CONFIG_KWS_TYPE_HYBRID
    if(LvpModelGetUseXipModelFlag()) {
        threshold = ((float)(kws_param->xip_threshold)/10.f);
    } else {
        threshold = ((float)(kws_param->threshold)/10.f);
    }
#else
    threshold = ((float)(kws_param->threshold)/10.f);
#endif

    if (kws_param && activation_kws) {
#ifdef CONFIG_ENABLE_KWS_LEVEL_DECODER
            static char* last_top_kws;
            static float last_top_sore = 0.f;
            int kws_level_type = kws_param->major & KWS_LEVEL_MASK;
            int last_kws_decoder_type = GetKwsDecoderLevel();
            printf (LOG_TAG"kws_level_type:%x, last_kws_decoder_type:%x\n", kws_level_type, last_kws_decoder_type);
            // printf (LOG_TAG"%d, %d\n", ((kws_level_type&KWS_LEVEL_END) == KWS_LEVEL_END)
            // , ((KWS_LEVEL_TOP1 == last_kws_decoder_type) || (KWS_LEVEL_TOP2 == last_kws_decoder_type)));
            if (0 == kws_level_type
            || (((kws_level_type&last_kws_decoder_type) == last_kws_decoder_type)
            && ((kws_level_type&(last_kws_decoder_type|KWS_LEVEL_END)) == (last_kws_decoder_type|KWS_LEVEL_END)))) {
                if (kws_level_type == 0) last_top_kws = "";

                float j_score = 0.f;
                LVP_KWS_PARAM_LIST *kws_list = LvpGetKwsParamList();
                if (10*activation_kws->kws_score < kws_list->kws_param_list[activation_kws->kws_index].threshold) {
                    j_score = last_top_sore*(activation_kws->kws_score+10.f);
                } else {
                    j_score = last_top_sore*activation_kws->kws_score;
                }
                // float j_score = last_top_sore*activation_kws->kws_score;
                // float j_th = 8500.f;
                // if (KWS_LEVEL_TOP4 == last_kws_decoder_type)
                
                int jc_threshold = kws_list->kws_param_list[activation_kws->kws_index].js_threshold;
                if (j_score < (float)jc_threshold) {
                    printf (LOG_TAG" JS_Act! JS_Act_Kws:[%s %s,%d],KV:%d,S:%d,J_S:%f[%d],ctx:%d\n"
                            , last_top_kws, kws_param->kws_words
                            , (int)(10*threshold)
                            , kws_param->kws_value
                            , (int)(10*activation_kws->kws_score)
                            , last_top_sore*activation_kws->kws_score
                            , jc_threshold
                            , context->ctx_index);
                    ResetCtcWinodw();
                    KwsStrategyReset();
                    return NULL;
                }
                printf (LOG_TAG" Activation! Kws:%s %s[%d],th:%d,S:%d,J_S:%f[%d],ctx:%d\n"
                        , last_top_kws, kws_param->kws_words
                        , kws_param->kws_value
                        , (int)(10*threshold)
                        , (int)(10*activation_kws->kws_score)
                        , last_top_sore*activation_kws->kws_score
                        , jc_threshold
                        , context->ctx_index);
                last_top_kws = NULL;
                last_top_sore = 0.f;
            } else if (kws_level_type == KWS_LEVEL_MID) {
                last_top_sore = activation_kws->kws_score;
                printf (LOG_TAG" MID Hit! Kws:[%s,%d],th:%d,S:%d,ctx:%d\n"
                        , kws_param->kws_words
                        , kws_param->kws_value
                        , (int)(10*threshold)
                        , (int)(10*activation_kws->kws_score)
                        , context->ctx_index);
            } else if (((kws_level_type & KWS_LEVEL_TOP_MASK) != 0) && ((kws_level_type & (~KWS_LEVEL_TOP_MASK)) == 0) ) {
                last_top_kws = kws_param->kws_words;
                last_top_sore = activation_kws->kws_score;
                printf (LOG_TAG" TOP Hit! Kws:[%s,%d],th:%d,S:%d,ctx:%d\n"
                        , kws_param->kws_words
                        , kws_param->kws_value
                        , (int)(10*threshold)
                        , (int)(10*activation_kws->kws_score)
                        , context->ctx_index);
            } else {
                printf (LOG_TAG" IGNORE Hit! Kws:[%s,%d],th:%d,S:%d,ctx:%d\n"
                        , kws_param->kws_words
                        , kws_param->kws_value
                        , (int)(10*threshold)
                        , (int)(10*activation_kws->kws_score)
                        , context->ctx_index);
                KwsStrategyReset();
                last_top_sore = 0.f;
                return NULL;
            }
#else
        printf (LOG_TAG" Activation ctx:%d,Kws:%s[%d],th:%d,S:%d\n"
                , context->ctx_index
                , kws_param->kws_words
                , kws_param->kws_value
                , (int)(10*threshold)
                , (int)(10*activation_kws->kws_score));
#endif
    } else {
        KwsStrategyReset();
    }

    return activation_kws;
}

//=================================================================================================

static float threshold_offset = 0.f;
float KwsStrategyGetThresholdOffset(LVP_CONTEXT *context, float threshold)
{
    LVP_CONTEXT *prev_context;
    unsigned int ctx_size;
    LvpGetContext(context->ctx_index - 1, &prev_context, &ctx_size);

    int ns_threshold_offset = 0;
    if (prev_context->env_noise == ENV_HIGH_NOISE) {
        // 问题号：#267286 主要是为了解决高噪下的误激活，因为耳机在地铁报站的时候，前期的测试会出现误激活的问题
        if (threshold > 90) ns_threshold_offset = 1;
        else ns_threshold_offset = 2;
    }
    return threshold_offset + ns_threshold_offset;
}

#ifdef CONFIG_LVP_ENABLE_BIONIC
static int context_counter = 0;
#endif
void KwsStrategyClearThresholdOffset(void)
{
#ifdef CONFIG_LVP_ENABLE_BIONIC
    context_counter  = 0;
    threshold_offset =  0.f;
    // printf(LOG_TAG" reset th_offset: %d\n", (int)(threshold_offset*10));
#endif
}

__attribute__((unused)) int KwsStrategyRunBionic(LVP_CONTEXT *context, LVP_KWS_PARAM *kws, float score, float threshold, float *decoder_window)
{
#ifdef CONFIG_LVP_ENABLE_BIONIC
    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    int ctx_ms = ctx_header->frame_length * ctx_header->pcm_frame_num_per_context;
    context_counter ++;

    // CONFIG_LVP_KWS_THRESHOLD_ADJUST_TIME s
    LVP_KWS_PARAM_LIST *kws_list = LvpGetKwsParamList();
    int time_out = CONFIG_LVP_KWS_THRESHOLD_ADJUST_TIME * 1000 * kws_list->count / CONFIG_KWS_MODEL_DECODER_STRIDE_LENGTH;

    if (context_counter * ctx_ms > time_out && threshold_offset < CONFIG_LVP_KWS_MAX_THRESHOLD_ADJUSTMENT_VALUE/10.f) {
        context_counter   = 0;
        threshold_offset += CONFIG_LVP_KWS_THRESHOLD_STEP/10.f;
        printf(LOG_TAG"threshold_offset: %d\n",  (int)(threshold_offset*10));
    }
#endif

    return 0;
}

//=================================================================================================

__attribute__((unused)) float KwsStrategyFineTuneScore(LVP_CONTEXT *context, float score)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    int runing_time_ms   = context->ctx_index * ctx_header->pcm_frame_num_per_context * ctx_header->frame_length;
    int judgment_time_ms = CONFIG_KWS_MODEL_DECODER_WIN_LENGTH * ctx_header->pcm_frame_num_per_context * ctx_header->frame_length;

    if (runing_time_ms < judgment_time_ms) {
# ifdef CONFIG_MCU_ENABLE_DYNAMICALLY_ADJUSTMENT_CPU_FREQUENCY
        score += 1.f;
# else
#  if defined CONFIG_MCU_FREQUENCY_8M
        score += 3.f;
#  elif defined CONFIG_MCU_FREQUENCY_12M
        score += 2.f;
#  elif defined CONFIG_MCU_FREQUENCY_24M
        score += 1.f;
#  endif
# endif
    }
#endif
    return score;
}
