
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
#include <levenshtein/levenshtein.h>
#include <dimsim/dimsim.h>

#include <decoder.h>

//#define __OPEN_PRINTF__
#define LOG_TAG "[LVP_CTC]"

static PREFIX_LIST g_prefix[CONFIG_BEAM_SIZE];
static char bm_str[CONFIG_BEAM_SIZE][80];

__attribute__((unused)) static void printf_syllable(float *ctc_decoder_window, char *kws_words, char *labels, int label_length)
{
    printf ("[%s]\n", kws_words);
    float max_syllable_prob = 0.f;
    int max_syllable_index = 0;
    for (int i = 0; i < CONFIG_KWS_MODEL_DECODER_WIN_LENGTH; i++) {
        for (int k = 0; k < label_length; k++) {
            for (int j = 0; j < CONFIG_KWS_MODEL_OUTPUT_LENGTH; j++) {
                float syllable_prob = ctc_decoder_window[i * CONFIG_KWS_MODEL_OUTPUT_LENGTH + j];
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

void _ClearPrefixList(int beam_size, PREFIX_LIST *prefix_list)
{
    for (int n = 0; n < beam_size; n++) {
        memset(prefix_list[n].prefix_set_prev, 65, CONFIG_PREFIX_CHAR_LENGTH);
        prefix_list[n].probs_nb_prev = 0.0;
        prefix_list[n].probs_b_prev = 1.0;
        prefix_list[n].probs_set_prev = 0.0;
    }
}

void _GetBeamSearchStr(int beam_size, int *prefix_length)
{
    for (int i = 0; i < beam_size; i++) {
        PREFIX_LIST *prefix = &g_prefix[i];
        memset (bm_str[i], '\0', sizeof(bm_str[i]));
        prefix_length[i] = 0;
        for (int m = 0; m < CONFIG_PREFIX_CHAR_LENGTH; m++) {
            int idx = prefix->prefix_set_prev[m];
            if (idx < 65 && idx > 1) {
                strcat (bm_str[i], g_syllable_table[idx - 1]);
                if (idx-1 < 57 && idx-1 > 20) strcat (bm_str[i], " ");
                prefix_length[i]++;
            }
        }
    }
}

float _GetKwsDimSim(LVP_KWS_PARAM_LIST g_kws_list, int kws_index, int beam_index)
{
    char *kws = g_kws_list.kws_param_list[kws_index].labels;
    float tot = g_kws_list.kws_param_list[kws_index].label_length/2*2.1;
    float dist = 0.f;

    PREFIX_LIST *prefix = &g_prefix[beam_index];
    for (int m = 0, n = 0; m < CONFIG_PREFIX_CHAR_LENGTH; ) {
        unsigned int a_id = kws[n];
        unsigned int b_id = prefix->prefix_set_prev[m] - 1;
        if (b_id == 64) {// 跳过开头、结尾的blank音节
            m ++;
            continue;
        }
        if (b_id > 20 && b_id < 57) { // BM 是韵母
            if (a_id > 20 && a_id < 57) {

            } else {// KWS 与之比较的是声母，则要跳过该 KWS 声母
                n++;
                continue;
            }
        }
        dist += GetEditDistance(a_id, b_id);
        m++;
        n++;
    }

    return dist/tot;
}

float _GetKwsLevenshtein(LVP_KWS_PARAM_LIST g_kws_list, int kws_index, char *str)
{
    char *kws = g_kws_list.kws_param_list[kws_index].kws_words;
    char *bm_kws = str;
    int tot = strlen(g_kws_list.kws_param_list[kws_index].kws_words);
    int dist = levenshtein(kws, bm_kws);
    // printf ("$$$ kws:%s, bm_kws:%s, dist:%d, tot:%d\n", kws, bm_kws, dist, tot);
    return (float)dist/tot;
}

static float s_bm_decoder_window[CONFIG_KWS_MODEL_OUTPUT_LENGTH] ALIGNED_ATTR(16);
int LvpDoBeamSearch(LVP_CONTEXT        *context,
                     LVP_KWS_PARAM_LIST g_kws_list,
                     PREFIX_LIST        *prefix_list,
                     float              *s_ctc_decoder_window,
                     int                valid_frame_num,
                     float              ctc_score,
                     int                ctc_kws_index,
                     int                s_ctc_index,
                     float              *bm_activation_score)
{
    int flag = 0;
    float mini_dimsim_dist = 3.4e+38;
    float levenshtein_dist = 3.4e+38;
    int mini_dimsim_index = -1;
    float levenshtein_threshold = (float)CONFIG_LEVENSHTEIN_THRESHOLD/100.f;
    float dimsim_threshold = (float)CONFIG_DIMSIM_THRESHOLD/100.f;

#ifdef CONFIG_LVP_KWS_HUAWEI_WATCH_V0DOT1DOT1_2020_1109
    if (g_kws_list.kws_param_list[ctc_kws_index].kws_value != CONFIG_KEYWORD_SHANG_YI_SHOU_VALUE) return 0;
#endif

#ifdef CONFIG_LVP_KWS_DRYER_DOUBLE_V0DOT0DOT1_2023_0427
    return 0;
#endif

    _ClearPrefixList(CONFIG_BEAM_SIZE, prefix_list);
    int beam_size = 0;
    int win_length = CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;

    int index = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;

#if 0
    float s_bm_decoder_window_1[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH*CONFIG_KWS_MODEL_OUTPUT_LENGTH] ALIGNED_ATTR(16);
    memcpy(s_bm_decoder_window_1
            , &s_ctc_decoder_window[index * CONFIG_KWS_MODEL_OUTPUT_LENGTH]
            , (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - index) * CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    memcpy(&s_bm_decoder_window_1[(CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - index) * CONFIG_KWS_MODEL_OUTPUT_LENGTH]
            , &s_ctc_decoder_window[0]
            , (index) * CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    printf_syllable(&s_bm_decoder_window_1[0], g_kws_list.kws_param_list[ctc_kws_index].kws_words, g_kws_list.kws_param_list[ctc_kws_index].labels, g_kws_list.kws_param_list[ctc_kws_index].label_length);
#endif

    int prefix_length[CONFIG_BEAM_SIZE] = {0};
    for (int p = index; p < win_length; p++) {
        memcpy(s_bm_decoder_window, &s_ctc_decoder_window[p * CONFIG_KWS_MODEL_OUTPUT_LENGTH], sizeof(float)*CONFIG_KWS_MODEL_OUTPUT_LENGTH);
        KwsCtcBeamSearchScore(&s_bm_decoder_window[0]
                    , &beam_size
                    , &flag
                    , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                    , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                    , 0.99f
                    , CONFIG_KWS_MODEL_BEAMSEARCH_AND_LM_CUTOFF_TOP_N
                    , prefix_list
                    , context);

        if (p >= win_length - 1) {
            memcpy (&g_prefix[0], prefix_list, CONFIG_BEAM_SIZE*sizeof(PREFIX_LIST));

            // str
            _GetBeamSearchStr(CONFIG_BEAM_SIZE, prefix_length);
#ifdef __OPEN_PRINTF__
            for (int i = 0; i < CONFIG_BEAM_SIZE; i++) {
                if (p == win_length - 1) {
                    printf ("[%d]%s\n", i, bm_str[i]);
                }
            }
#endif

            for (int i = 0; i < CONFIG_BEAM_SIZE; i++) {
                for (int j = 0; j < g_kws_list.count; j++) {
                    if (prefix_length[i] < g_kws_list.kws_param_list[j].label_length-1) continue;

                    float dimsim_dist = _GetKwsDimSim(g_kws_list, j, i);

                    // printf ("#@#@ [%s, %s] leven_dist:%d, dimsim:%f\n", g_kws_list.kws_param_list[j].kws_words, bm_str[i], dist, res);
                    if (dimsim_dist < mini_dimsim_dist && dimsim_dist < dimsim_threshold) {
                        mini_dimsim_dist = dimsim_dist;
                        mini_dimsim_index = j;
#ifdef __OPEN_PRINTF__
                        printf ("###  clr_add ### %s, %d, %d, %s, %f\n", g_kws_list.kws_param_list[j].kws_words, j, ctc_kws_index, bm_str[i], mini_dimsim_dist);
#endif
                    } else if (dimsim_dist < mini_dimsim_dist && dimsim_dist < 50.f) {
                        float dist = _GetKwsLevenshtein(g_kws_list, j, bm_str[i]);
                        if (dist < levenshtein_dist && dist < levenshtein_threshold && j == ctc_kws_index) {
                            levenshtein_dist = dist;
                            mini_dimsim_index = j;
#ifdef __OPEN_PRINTF__
                            printf ("#=#  lev_add #=# %s, %d, %d, %s, %f, %f, tot:%d, dist:%f\n"
                                                , g_kws_list.kws_param_list[j].kws_words
                                                , j, ctc_kws_index, bm_str[i]
                                                , dimsim_dist
                                                , dist
                                                , strlen(g_kws_list.kws_param_list[j].kws_words)
                                                , dist * strlen(g_kws_list.kws_param_list[j].kws_words));
#endif
                            continue;
                        }
#ifdef __OPEN_PRINTF__
                        printf ("#=#  waite  #=# %s, %d, %d, %s, %f, %f, tot:%d, dist:%f\n"
                                            , g_kws_list.kws_param_list[j].kws_words
                                            , j, ctc_kws_index, bm_str[i]
                                            , dimsim_dist
                                            , dist
                                            , strlen(g_kws_list.kws_param_list[j].kws_words)
                                            , dist * strlen(g_kws_list.kws_param_list[j].kws_words));
#endif
                    } else if (dimsim_dist == mini_dimsim_dist && j == ctc_kws_index) {
                        mini_dimsim_index = j;
#ifdef __OPEN_PRINTF__
                        printf ("=== same_add === %s, %d, %d, %s, %f\n", g_kws_list.kws_param_list[j].kws_words, j, ctc_kws_index, bm_str[i], mini_dimsim_dist);
#endif
                    }
                }
            }
        }
        if (index > 0 && p == (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - 1)) {
            p = -1;
            win_length = index;
        }
    }

    if(mini_dimsim_index != -1) {
        // printf ("########$$$$$$$$$$######### result:%d, %d\n", mini_dimsim_index, ctc_kws_index);
        if (mini_dimsim_index == ctc_kws_index) return 0;
    }
    return -1;
}