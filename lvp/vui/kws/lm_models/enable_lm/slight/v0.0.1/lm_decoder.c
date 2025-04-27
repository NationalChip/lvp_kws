
#include <autoconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <lvp_context.h>
#include <lvp_param.h>

//#include <kws_list.h>

#include <syllable_table.h>
#include <tiny-regex-c/re.h>

#include "decoder.h"

#define LOG_TAG "[LVP_CTC]"

static PREFIX_LIST g_prefix[CONFIG_BEAM_SIZE];
static char str[CONFIG_BEAM_SIZE][80];

__attribute__((unused)) static void printf_syllable(float *ctc_decoder_window, char *kws_words, char *labels, int label_length)
{
    int window_length = CONFIG_KWS_MODEL_DECODER_WIN_LENGTH * CONFIG_KWS_MODEL_OUTPUT_LENGTH;
    printf ("[%s]\n", kws_words);
    float max_syllable_prob = 0.f;
    int max_syllable_index = 0;
    for (int i = 0; i < CONFIG_KWS_MODEL_DECODER_WIN_LENGTH; i++) {
        for (int k = 0; k < label_length; k++) {
            for (int j = 0; j < CONFIG_KWS_MODEL_OUTPUT_LENGTH; j++) {
                float syllable_prob = ctc_decoder_window[window_length + i * CONFIG_KWS_MODEL_OUTPUT_LENGTH + j];
                if (j == labels[k]) {
                    printf ("%f[%d-%s],", syllable_prob, j, g_syllable_table[j]);
                }
                if (max_syllable_prob < syllable_prob) {
                    max_syllable_prob = syllable_prob;
                    max_syllable_index = j;
                }
            }
        }
        printf ("[%d][%s-%f]\n", i, g_syllable_table[max_syllable_index], max_syllable_prob);
        max_syllable_prob = 0.f;
        max_syllable_index = 0;
    }
}

int LvpDoBeamSearch(LVP_CONTEXT        *context,
                     LVP_KWS_PARAM_LIST g_kws_list,
                     PREFIX_LIST        *prefix_list,
                     float              *s_ctc_decoder_window,
                     int                valid_frame_num,
                     float              ctc_score,
                     int                ctc_kws_index,
                     int                s_ctc_index)
{
    int flag = 0;

    for (int n = 0; n < CONFIG_BEAM_SIZE; n++) {
        memset(prefix_list[n].prefix_set_prev, 65, CONFIG_PREFIX_CHAR_LENGTH);
        prefix_list[n].probs_nb_prev = 0.0;
        prefix_list[n].probs_b_prev = 1.0;
        prefix_list[n].probs_set_prev = 0.0;
    }

    int beam_size = 0;
    int kws_list_len = 0;
    int win_length = CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;

    int index = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;

    for (int p = index; p < win_length; p++) {
        
        if (index > 0 && p == (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - 1)) {
            p = -1;
            win_length = index;
        }

        KwsCtcBeamSearchLmScore(&s_ctc_decoder_window[p * CONFIG_KWS_MODEL_OUTPUT_LENGTH]
                    , &beam_size
                    , CONFIG_KWS_MODEL_BEAMSEARCH_AND_LM_ALPHA/10.f
                    , CONFIG_KWS_MODEL_BEAMSEARCH_AND_LM_BETA/10.f
                    , &flag
                    , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                    , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                    , 0.999f
                    , CONFIG_KWS_MODEL_BEAMSEARCH_AND_LM_CUTOFF_TOP_N
                    , prefix_list
                    , context);
    
        if (p >= win_length - 3) {
            memcpy (&g_prefix[0], prefix_list, CONFIG_BEAM_SIZE*sizeof(PREFIX_LIST));

            // str
            int prefix_length[CONFIG_BEAM_SIZE] = {0};
            for (int i = 0; i < CONFIG_BEAM_SIZE; i++) {
                PREFIX_LIST *prefix = &g_prefix[i];

                memset (str[i], '\0', sizeof(str[i]));
                prefix_length[i] = 0;
                for (int m = 0; m < CONFIG_PREFIX_CHAR_LENGTH; m++) {
                    int idx = prefix->prefix_set_prev[m];
                    if (idx < 65 && idx > 1) {
                        strcat (str[i], g_syllable_table[idx - 1]);
                        strcat (str[i], " ");
                        prefix_length[i]++;
                    }
                }
                if (p == win_length - 1) {
                    printf ("[%d]%s\n", i, str[i]);
                }
            }
            
            for (int i = 0; i < CONFIG_BEAM_SIZE; i++) {
                
                for (int j = 0; j < g_kws_list.count; j++) {
                    kws_list_len = g_kws_list.kws_param_list[j].label_length - 0;
                    if (prefix_length[i] < kws_list_len) continue;

                    char kws[80];
                    memset (kws, '\0', sizeof(kws));
                    for (int m = 0; m < kws_list_len; m++) {
                        int idx = g_kws_list.kws_param_list[j].labels[m];
                        strcat (kws, g_syllable_table_fuzzy[idx]);

                        if (m == kws_list_len - 1)break;
                        if (m % 2 == 1) {
                            strcat (kws, " .*");
                        } else {
                            strcat (kws, " ");
                        }
                    }
                    re_t pattern = re_compile(kws);
                    /* Check if the regex matches the text: */
                    int match_length = 0;
                    int match_idx = re_matchp(pattern, str[i], &match_length);
                    int ret = 0;
                    int beamsearch_kws_index = -1;

                    if (match_idx != -1) {
                        if (p >= win_length - 3) {
                            printf("##### beam[%d] match at idx %i, %i chars long\n", i, match_idx, match_length);
                            printf ("str:%s\n", str[i]);
                            printf ("kws[%s]:%s\n", g_kws_list.kws_param_list[j].kws_words, kws);
                            for (int m = 0; m < beam_size; m++) {
                                printf ("[%d]:", m);
                                PREFIX_LIST *prefix = &g_prefix[m];
                                for (int j = 0; j < CONFIG_PREFIX_CHAR_LENGTH; j++) {
                                    int idx = prefix->prefix_set_prev[j];
                                    if (idx != 65 && idx > 0) printf("%s ", g_syllable_table[idx - 1]);
                                }
                                if (strlen(prefix->prefix_set_prev)) printf (":%f[log:%f]\n", prefix->probs_set_prev, 100 + fastlogf(prefix->probs_set_prev));
                                else printf ("\n");
                            }

                            for (int mm = 0; mm < CONFIG_PREFIX_CHAR_LENGTH; mm++) {
                                printf("%s ", g_syllable_table[(g_prefix[i].prefix_set_prev)[mm] - 1]);
                            }
                            printf("]\n[%s]", g_kws_list.kws_param_list[j].kws_words);
                            printf("[%f]", g_prefix[i].probs_set_prev);
                            printf ("ctx:%d, beam_size:%d, beam_idx:%d\n", context->ctx_index, beam_size, i);

                            beamsearch_kws_index = j;
                            ret = 1;                    
                        } else {
                            ret = -1;   //激活匹配的位置不在最后三帧，认为是串词
                        }
                    }
                    
                    if (ret == 1) {
//                        printf_syllable(&s_ctc_decoder_window[0], g_kws_list.kws_param_list[*beamsearch_kws_index].kws_words, g_kws_list.kws_param_list[*beamsearch_kws_index].labels, g_kws_list.kws_param_list[*beamsearch_kws_index].label_length);
                        if (strcmp(g_kws_list.kws_param_list[ctc_kws_index].kws_words, g_kws_list.kws_param_list[beamsearch_kws_index].kws_words) == 0) {
                            printf (LOG_TAG"[BM] Activation ctx:%d,Kws:%s[%d],th:%d\n"
                                    , context->ctx_index
                                    , g_kws_list.kws_param_list[beamsearch_kws_index].kws_words
                                    , g_kws_list.kws_param_list[beamsearch_kws_index].kws_value
                                    , g_kws_list.kws_param_list[beamsearch_kws_index].threshold);
                            return 0;
                        } else {
                            float tmp_score = LvpFastCtcBlockScore(&s_ctc_decoder_window[index]
                                    , valid_frame_num - index// - 1
                                    , &s_ctc_decoder_window[0]
                                    , index// - 1
                                    , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                                    , CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1
                                    , (unsigned char *)g_kws_list.kws_param_list[beamsearch_kws_index].labels
                                    , g_kws_list.kws_param_list[beamsearch_kws_index].label_length);

                            if (tmp_score > ctc_score) {
                                return 1;
                            }
                        }
                    } else if (ret == -1) {
                        return -1;
                    }
                }
            }
        }
    }
    return -1;
}