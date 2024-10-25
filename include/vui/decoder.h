/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * decoder.h: Kws Model
 *
 */

#ifndef __DECODER_H__
#define __DECODER_H__

#include <lvp_param.h>
#include <lvp_context.h>
#include <autoconf.h>

typedef struct {
    int cnt;
    int ctx_index;
} KWS_DELAY2DECODE;

typedef struct {
    int kws_value;
    int mask_threshold; // 0:default
} LVP_CTC_MASK;

# if defined(CONFIG_LVP_ENABLE_CTC_DECODER)||defined(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)
typedef struct {
    char prefix_set_prev[CONFIG_PREFIX_CHAR_LENGTH];
    float probs_set_prev;
    float probs_nb_prev;
    float probs_b_prev;
}PREFIX_LIST;
# endif

typedef enum
{
    VUI_KWS_VAD_STATE = 0,
    VUI_KWS_ACTIVE_STATE,
}VUI_KWS_STATE;

int LvpSetVuiKwsStates(VUI_KWS_STATE state);
int LvpGetVuiKwsStates(void);

void ResetCtcWinodw(void);
void ResetCtcWinodwUser(int start, int len);
float LvpCtcScoreKws(float* probs, int T, int alphabet_size, int blank, unsigned short *labels, int labels_length);
float LvpFastCtcScore(float* probs, int T, int alphabet_size, int blank, unsigned short *labels, int labels_length);
float LvpFastCtcBlockScore(float* probs, int T1, float* probs2, int T2, int alphabet_size, int blank, unsigned short * labels, int labels_length);
float LvpFastCtcBlockScorePlus(float* probs, int T1, float* probs2, int T2, int alphabet_size, int blank, unsigned short * labels, int labels_length, int *score_index);
int LvpFastCtcJudge(float* probs, int T1, float* probs2, int T2, int alphabet_size, int blank, unsigned short * labels, int labels_length, float threshold);
void LvpInitCtcKws(void);
void LvpPrintCtcKwsList(void);
LVP_KWS_PARAM_LIST *LvpGetKwsParamList(void);
int LvpCtcMask(LVP_CTC_MASK *mask, int mask_num);
int LvpDoKwsScore(LVP_CONTEXT *context);
int LvpDoUserDecoder(LVP_CONTEXT *context);
float fastlogf (float x);

# if defined(CONFIG_LVP_ENABLE_CTC_DECODER)||defined(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)
int KwsCtcBeamSearchLmInit(unsigned char *lm, unsigned char *lst);
int KwsCtcBeamSearchLmScore(float          *probs_seq
        , int            *beam_size
        , float          alpha
        , float          beta
        , int            *flag
        , int            probs_len
        , int            blank_id
        , float          cutoff_prob
        , int            cutoff_top_n
        , PREFIX_LIST    *prefix_list
        , LVP_CONTEXT    *context);
int KwsCtcBeamSearchScore(float          *probs_seq
        , int            *beam_size
        , int            *flag
        , int            probs_len
        , int            blank_id
        , float          cutoff_prob
        , int            cutoff_top_n
        , PREFIX_LIST    *prefix_list
        , LVP_CONTEXT    *context);
# endif

int LvpDoGXDecoder(LVP_CONTEXT *context
                , float *decoder_window1
                , int t1
                , float *decoder_window2
                , int t2
                , char *ctc_words
                , int ctc_words_size
                , void *xdecoder_tmp_buffer
                , int label_length
                , int start_index
                , char **gxdecoder_words);

#endif /* __DECODER_H__ */
