/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * ctc.c: The Process For Kws
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
#include "kws_list.h"
#endif


#include "decoder.h"
#include "ctc_model.h"
#include "kws_strategy.h"

#define LOG_TAG "[LVP_MAX_DECODE]"

#define CONFIG_KWS_MAX_DECODER_WORDS_NUMBER 200
#ifndef CONFIG_KWS_MAX_DECODER_WIN_LENGTH
#define CONFIG_KWS_MAX_DECODER_WIN_LENGTH 10
#endif
#ifndef CONFIG_MODEL_ACTIVATION_NUMBERS
#define CONFIG_MODEL_ACTIVATION_NUMBERS 1
#endif

#ifdef CONFIG_ENABLE_CTC_KWS_AND_BUN_KWS_CASCADE
#define MODEL_OUTPUT_LENGTH CONFIG_BUN_KWS_MODEL_OUTPUT_LENGTH
#else
#define MODEL_OUTPUT_LENGTH CONFIG_KWS_MODEL_OUTPUT_LENGTH
#endif

static int s_max_decoder_window[CONFIG_KWS_MAX_DECODER_WIN_LENGTH][MODEL_OUTPUT_LENGTH] = {0};
static int s_max_score = 0;
static int s_max_index = 0;
static LVP_KWS_PARAM_LIST g_kws_list;
static VUI_KWS_STATE s_state = VUI_KWS_ACTIVE_STATE;
static uint8_t *activation_flag[CONFIG_KWS_MAX_DECODER_WORDS_NUMBER] = {0};

void ResetMaxWindow(void)
{
    memset(s_max_decoder_window, 0, sizeof(s_max_decoder_window));
    s_max_score = 0;
}

DRAM0_STAGE2_SRAM_ATTR LVP_KWS_PARAM_LIST *LvpGetKwsParamList(void)
{
    return &g_kws_list;
}

void LvpPrintMaxKwsList(void)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    g_kws_list.count = sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM);
    g_kws_list.kws_param_list = g_kws_param_list;
    printf (LOG_TAG"Demo Kws List [Total:%d]:\n", g_kws_list.count);
    for (int i = 0; i < g_kws_list.count; i++) {
        printf(LOG_TAG"KWS: %s | ", g_kws_list.kws_param_list[i].kws_words);
        printf("KV: %02d | ", g_kws_list.kws_param_list[i].kws_value);
        printf("TRH: %02d | ", g_kws_list.kws_param_list[i].threshold);
#ifdef CONFIG_ENABLE_CTC_KWS_AND_BUN_KWS_CASCADE
        printf("BUNKWS TRH: %02d | ", g_kws_list.kws_param_list[i].bk_threshold);
#endif
        printf("\n");
    }
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

static int _LvpDoMaxScore(LVP_CONTEXT *context, int major)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    memset(activation_flag, 0, g_kws_list.count * sizeof(uint8_t));
    gx_dcache_invalid_range((unsigned int *)context->snpu_buffer, context->ctx_header->snpu_buffer_size);
    float *output = (float *)LvpCTCModelGetSnpuOutBuffer(context->snpu_buffer);
    int idx = s_max_index % CONFIG_KWS_MAX_DECODER_WIN_LENGTH;

    s_max_index++;
    context->kws = 0;
    for (int i = 0; i < g_kws_list.count; i++) {
        int score = (int)(output[i] * 1000);

#ifdef CONFIG_ENABLE_CTC_KWS_AND_BUN_KWS_CASCADE
        int threshold = g_kws_list.kws_param_list[i].bk_threshold + KwsStrategyGetBunkwsThresholdOffset(context, g_kws_list.kws_param_list[i].bk_threshold);
#elif defined CONFIG_LVP_ENABLE_BUNKWS_BIONIC
        KwsStrategyRunBionic(context, &g_kws_list.kws_param_list[i], score, g_kws_list.kws_param_list[i].threshold, NULL);
        int threshold = 0;
        if (g_kws_list.kws_param_list[i].major)
            threshold = g_kws_list.kws_param_list[i].threshold + KwsStrategyGetBunkwsThresholdOffset(context, g_kws_list.kws_param_list[i].threshold);
        else
            threshold = g_kws_list.kws_param_list[i].threshold;
#else
        int threshold = g_kws_list.kws_param_list[i].threshold + KwsStrategyGetBunkwsThresholdOffset(context, g_kws_list.kws_param_list[i].threshold);
#endif

        if ((threshold > 100) && (score > threshold - 100) && (score < threshold)) {
            printf (LOG_TAG"Similar ctx:%d,Kws:%s[%d],th:%d,S:%d,D:%d\n"
                , context->ctx_index
                , g_kws_list.kws_param_list[i].kws_words
                , g_kws_list.kws_param_list[i].kws_value
                , threshold
                , score
                , score - threshold);
        }

        if ((score > threshold) && ((g_kws_list.kws_param_list[i].major & 0x1) == major)) {
            printf (LOG_TAG"Greater than the score threshold! th:%d,S:%d,D:%d\n", threshold, score, score - threshold);
            if (s_max_score < score && activation_flag[i] <= CONFIG_MODEL_ACTIVATION_NUMBERS)
                s_max_score = score;
            s_max_decoder_window[idx][i] = 1;
            for (int j = 0; j < CONFIG_KWS_MAX_DECODER_WIN_LENGTH; j ++) {
                activation_flag[i] += s_max_decoder_window[j][i];
            }
            if (activation_flag[i] == CONFIG_MODEL_ACTIVATION_NUMBERS) {
                KwsStragegyInsertKwsActivation(i, g_kws_list.kws_param_list[i].kws_value, s_max_score/10.f, 0, NULL);
                printf (LOG_TAG"awaken ctx:%d,Kws:%s[%d],th:%d,S:%d,%d\n"
                    , context->ctx_index
                    , g_kws_list.kws_param_list[i].kws_words
                    , g_kws_list.kws_param_list[i].kws_value
                    , threshold
                    , s_max_score
                    , s_max_score - threshold);
                s_max_score = 0;
                if (g_kws_list.kws_param_list[i].major)
                    KwsStrategyClearBunkwsThresholdOffset();
#ifdef CONFIG_ENABLE_CTC_KWS_AND_BUN_KWS_CASCADE
                ResetCtcWinodw();
                return g_kws_list.kws_param_list[i].kws_value;
#endif
            }
        }
        else {
            s_max_decoder_window[idx][i] = 0;
        }
    }
    return -1;
#else
    return 0;
#endif
}

int LvpDoMaxDecoder(LVP_CONTEXT *context)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    int ret = _LvpDoMaxScore(context, 1);
    if (ret != 0 && s_state == VUI_KWS_ACTIVE_STATE) {
        _LvpDoMaxScore(context, 0);
    }
    // 选择分差大来做最终激活词
    LVP_ACTIVATION_KWS *activation_kws = KwsStragegyRun(context);
    if (NULL != activation_kws) {
        context->kws = activation_kws->kws_value;
        KwsStrategyClearThresholdOffset();
        KwsStrategyReset();
    }
#endif
    return 0;
}

void LvpInitMaxKws(void)
{
    if (CONFIG_KWS_MAX_DECODER_WORDS_NUMBER < g_kws_list.count)
        printf(LOG_TAG "==ERROR== The activation_flag space is too small\n");

    KwsStrategyInit();
}
