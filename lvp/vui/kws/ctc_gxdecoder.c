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
#include <driver/gx_irq.h>

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
#include <unistd.h>
#include <gxdecoder.h>
#include <lvp_pmu.h>

#define LOG_TAG "[CTC_GXDECODER]"
#define XDECODE_INPUT_LENGTH (CONFIG_KWS_MODEL_OUTPUT_LENGTH-1)

static int s_ctc_index = 0;
static int s_ctc_decoder_head;
static VUI_KWS_STATE s_state = VUI_KWS_ACTIVE_STATE;
#ifdef CONFIG_ENABLE_SWITCH_NPU_MODEL_RUN_IN_FLASH_OR_SRAM
static float s_ctc_decoder_window[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH][CONFIG_KWS_MODEL_OUTPUT_LENGTH] ALIGNED_ATTR(16) DRAM0_AUDIO_IN_ATTR;
static void *s_gxdecoder_tmp_buffer = NULL;
#else
static float s_ctc_decoder_window[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH][CONFIG_KWS_MODEL_OUTPUT_LENGTH] ALIGNED_ATTR(16);
static unsigned char *s_gxdecoder_tmp_buffer;//[50*1024] ALIGNED_ATTR(16) ;
#endif
static char *s_gxdecoder_words = NULL;

#ifdef CONFIG_ENABLE_DO_NOT_SLEEP_WHEN_AT_WAKE_UP
static int lock = 0;
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

void LvpInitCtcKws(void)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    g_kws_list.count = sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM);
    g_kws_list.kws_param_list = g_kws_param_list;
    ResetCtcWinodw();

    LvpGXDocoderInit();
# ifdef CONFIG_ENABLE_DO_NOT_SLEEP_WHEN_AT_WAKE_UP
    LvpPmuSuspendLockCreate(&lock);
# endif
#endif
}

void LvpPrintCtcKwsList(void)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
# ifdef CONFIG_ENABLE_USED_FOR_ALL_GRUS_FAMILY
    printf("G:%d\n", 1);
# endif
    printf(LOG_TAG"Kws Version: [%s]\n", LvpCTCModelGetKwsVersion());

    g_kws_list.count = sizeof(g_kws_param_list) / sizeof(LVP_KWS_PARAM);
    g_kws_list.kws_param_list = g_kws_param_list;
    printf (LOG_TAG"Demo Kws List [Total:%d]:\n", g_kws_list.count);
    for (int i = 0; i < g_kws_list.count; i++) {
        printf(LOG_TAG"KWS: %s | ", g_kws_list.kws_param_list[i].kws_words);
        printf("KV: %02d | ", g_kws_list.kws_param_list[i].kws_value);
        printf("TRH: %02d | ", g_kws_list.kws_param_list[i].threshold);
        printf("Major: %d | ", g_kws_list.kws_param_list[i].major);
        printf("L: [");
        for (int j = 0; j < g_kws_list.kws_param_list[i].label_length; j++) {
            printf("%d ", g_kws_list.kws_param_list[i].labels[j]);
        }
        printf("]\n");
    }
#endif
}

DRAM0_STAGE2_SRAM_ATTR LVP_KWS_PARAM_LIST *LvpGetKwsParamList(void)
{
    return &g_kws_list;
}

DRAM0_STAGE2_SRAM_ATTR int PrepareData(float *rnn_out)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION

    int idx = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
    memcpy(&s_ctc_decoder_window[idx], rnn_out, CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    s_ctc_index++;

    return CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
#else
    return 0;
#endif
}


DRAM0_STAGE2_SRAM_ATTR static int _LvpDoGroupScore(LVP_CONTEXT *context, int index, int group_number, int valid_frame_num, int major)
{

#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    static int skip_frame_flag = 0;
    if (skip_frame_flag) {
        skip_frame_flag = 0;
        return 0;
    }
    for (int i = index; i < group_number; i++) {
        if (valid_frame_num < sizeof(g_kws_list.kws_param_list[i].label_length)) continue;
        if (s_gxdecoder_words!=NULL) {
            char *ctc_decoder_words = LvpGetCtcWords(g_kws_list.kws_param_list[i].kws_words);
            if (strstr(s_gxdecoder_words , ctc_decoder_words)) {
                ;
            } else {
                continue;
            }
        }
        if (g_kws_list.kws_param_list[i].major == major) {
            unsigned short *labels = NULL;
            int label_length = 0;
            labels = (unsigned short *)&g_kws_list.kws_param_list[i].labels[0];
            label_length = g_kws_list.kws_param_list[i].label_length;

# ifdef CONFIG_LVP_DISABLE_XIP_WHILE_CODE_RUN_AT_SRAM
            LvpXipSuspend();
# endif

            int idx = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
            int start_index = 0;
            float ctc_score = LvpFastCtcBlockScorePlus(&s_ctc_decoder_window[idx][0]
                            , valid_frame_num - idx// - 1
                            , &s_ctc_decoder_window[0][0]
                            , idx// - 1
                            , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                            , CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1
                            , labels
                            , label_length
                            , &start_index);

# ifdef CONFIG_LVP_DISABLE_XIP_WHILE_CODE_RUN_AT_SRAM
            LvpXipResume();
# endif

            float threshold = ((float)(g_kws_list.kws_param_list[i].threshold)/10.f);
            float score = ctc_score;
            // printf("score:%f\n", score);
# ifdef CONFIG_LVP_ENABLE_ONLY_PRINTF_SCORE_WITHOUT_ACTIVATE
            printf (LOG_TAG"ctx_id:%d, Kws:%s[%d], score:%d\n"
                        , context->ctx_index
                        , g_kws_list.kws_param_list[i].kws_words
                        , g_kws_list.kws_param_list[i].kws_value
                        , (int)(10*score));
            continue;
# else
# if 0
            if (score > threshold-5 && score < 120.f) {
                printf (LOG_TAG"ctx_id:%d, Kws:%s[%d], score:%d\n"
                        , context->ctx_index
                        , g_kws_list.kws_param_list[i].kws_words
                        , g_kws_list.kws_param_list[i].kws_value
                        , (int)(10*score));
            }
# endif

            if ((score > threshold) && (score <= 100.1f)) {
                if (g_kws_list.kws_param_list[i].major == 1) {
                    context->kws = g_kws_list.kws_param_list[i].kws_value;
                    printf (LOG_TAG" Activation ctx:%d,Kws:%s[%d],th:%d,S:%d,%d\n"
                            , context->ctx_index
                            , g_kws_list.kws_param_list[i].kws_words
                            , g_kws_list.kws_param_list[i].kws_value
                            , g_kws_list.kws_param_list[i].threshold
                            , (int)(10*score)
                            , (int)(10*threshold));
                    ResetCtcWinodw();

                    return 0;
                }
                else {
                    printf (LOG_TAG" ctc activation ctx:%d,Kws:%s[%d],th:%d,S:%d,%d\n"
                        , context->ctx_index
                        , g_kws_list.kws_param_list[i].kws_words
                        , g_kws_list.kws_param_list[i].kws_value
                        , g_kws_list.kws_param_list[i].threshold
                        , (int)(10*score)
                        , (int)(10*threshold));
                    LvpAudioInSuspend();
                    int idx = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;
                    int ret = 1;
                    if (s_gxdecoder_words == NULL) {
                        // while (gx_snpu_get_state() == GX_SNPU_BUSY);
                        gx_snpu_pause();
                        ret = LvpDoGXDecoder(context
                                            , &s_ctc_decoder_window[idx][0]
                                            , CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - idx// - 1
                                            , &s_ctc_decoder_window[0][0]
                                            , idx// - 1
                                            , g_kws_list.kws_param_list[i].kws_words
                                            , sizeof(g_kws_list.kws_param_list[i].kws_words)
                                            , (void *)s_gxdecoder_tmp_buffer
                                            , g_kws_list.kws_param_list[i].label_length
                                            , start_index
                                            , &s_gxdecoder_words);
                        gx_dcache_clean_range((unsigned int*)NPU_SRAM_ADDR, CONFIG_NPU_SRAM_SIZE_KB * 1024);
                        gx_snpu_resume();
                    }
                    LvpAudioInResume();
                    if(ret) {
                        ResetCtcWinodw();
# ifdef CONFIG_ENABLE_PRINTF_GXDECODER_MEMORY_MONITORING
                        static int cnt = 0;
                        if ((cnt++)%10 == 0) LvpGXdecoderMemoryMonitoring();
# endif
                        context->kws = g_kws_list.kws_param_list[i].kws_value;
                        printf (LOG_TAG" Activation ctx:%d,Kws:%s[%d],th:%d,S:%d,%d\n"
                                , context->ctx_index
                                , g_kws_list.kws_param_list[i].kws_words
                                , g_kws_list.kws_param_list[i].kws_value
                                , g_kws_list.kws_param_list[i].threshold
                                , (int)(10*score)
                                , (int)(10*threshold));
                    } else {
                        skip_frame_flag = 1;
                    }
                    // ResetCtcWinodw();

                }
            }
#endif
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

#ifdef CONFIG_ENABLE_SWITCH_NPU_MODEL_RUN_IN_FLASH_OR_SRAM
    LvpAudioInSuspend();
    while (gx_snpu_get_state() == GX_SNPU_BUSY);
    gx_disable_irq();
    if (s_state == VUI_KWS_ACTIVE_STATE) {
        printf(LOG_TAG"Switch NPU Run In Flash\n");
        LvpSetSnpuRunInFlash();
        s_gxdecoder_tmp_buffer = (void *)(NPU_SRAM_ADDR/4*4  + (LvpModelGetDataSize() + 3)/4*4);
        memset ((void *)NPU_SRAM_ADDR, 0, (LvpModelGetDataSize() + 3)/4*4);
        gx_dcache_clean_range((unsigned int *)NPU_SRAM_ADDR, (LvpModelGetDataSize() + 3)/4*4);
        printf(LOG_TAG"Switch Success\n");
    } else {
        printf(LOG_TAG"Switch NPU Run In Sram\n");
        LvpSetSnpuRunInSram();
        gx_dcache_clean_range((unsigned int *)NPU_SRAM_ADDR, (LvpModelGetDataSize() + 3)/4*4);
        printf(LOG_TAG"Switch Success\n");
    }
    gx_enable_irq();
    LvpAudioInResume();
#else
    s_gxdecoder_tmp_buffer = (unsigned char *)LvpGetAudioOutBuffer();
    printf(LOG_TAG"s_gxdecoder_tmp_buffer:%x\n", s_gxdecoder_tmp_buffer);
#endif

#ifdef CONFIG_ENABLE_DO_NOT_SLEEP_WHEN_AT_WAKE_UP
    if (VUI_KWS_ACTIVE_STATE == state) {
        LvpPmuSuspendLock(lock);
    } else {
        LvpPmuSuspendUnlock(lock);
    }
#endif

    return 0;
}

int LvpGetVuiKwsStates(void)
{
    return s_state;
}

LVP_KWS_PARAM *LvpGetKwsInfo(int kws_kv)
{
    for (int i = 0; i < g_kws_list.count; i++) {
        if (g_kws_list.kws_param_list[i].kws_value == kws_kv) {
            return &(g_kws_list.kws_param_list[i]);
        }
    }
    return NULL;
}

DRAM0_STAGE2_SRAM_ATTR int LvpDoKwsScore(LVP_CONTEXT *context)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION

    float *rnn_out;
    gx_dcache_invalid_range((unsigned int *)context->snpu_buffer, context->ctx_header->snpu_buffer_size);
    rnn_out = (float *)LvpCTCModelGetSnpuOutBuffer(context->snpu_buffer);

    context->kws = 0;// 挪到此处的原因是这段代码执行时间比较久,主要是 softmax
    s_gxdecoder_words = NULL;

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
    int ret = _LvpDoGroupScore(context, 0, g_kws_list.count, valid_frame_num, 1);//主唤醒词
    ret |= _LvpDoGroupScore(context, 0, g_kws_list.count, valid_frame_num, 2);//免唤醒词
    if (ret != 0 && s_state == VUI_KWS_ACTIVE_STATE) {
        // Detect Short Kws
        _LvpDoGroupScore(context, grout_offset, grout_count, valid_frame_num, 0);//指令词
    }
# ifdef CONFIG_ENABLE_CTC_DECODER_CYCLE_STATISTIC
    int end_ms = gx_get_time_ms();
    printf ("decoder:%d ms\n", end_ms - start_ms);
# endif



#endif

    return 0;
}
