/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * kws_strategy.h:
 *
 */

#ifndef __KWS_STRATEGY_H__
#define __KWS_STRATEGY_H__

#include <lvp_context.h>

typedef struct {
    int   kws_index;
    int   kws_value;
    float kws_score;
    int   score_index;
    void *reverse;
} LVP_ACTIVATION_KWS;

typedef struct {
    float ctc_threshold_offset;
    float bunkws_threshold_offset;
    int ctc_context_counter;
    int bunkws_context_counter;
} BIONIC;

void KwsStrategyInit(void);
void KwsStrategyReset(void);
void KwsStragegyInsertKwsActivation(int kws_index, int kws_value, float kws_score, int score_index, void *priv);
LVP_ACTIVATION_KWS *KwsStragegyRun(LVP_CONTEXT *context);
float KwsStrategyGetBunkwsThresholdOffset(LVP_CONTEXT *context, float threshold);
float KwsStrategyGetThresholdOffset(LVP_CONTEXT *context, float threshold);
void KwsStrategyClearThresholdOffset(void);
int KwsStrategyRunBionic(LVP_CONTEXT *context, LVP_KWS_PARAM *kws, float score, float threshold, float *decoder_window);

float KwsStrategyFineTuneScore(LVP_CONTEXT *context, float score);

#endif /* __KWS_STRATEGY_H__ */
