
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
                strcat (bm_str[i], " ");
                prefix_length[i]++;
            }
        }
    }
}

int _GetKwsLevenshtein(LVP_KWS_PARAM_LIST g_kws_list, int kws_index, char *str)
{
    char *kws = g_kws_list.kws_param_list[kws_index].kws_words;
    char *bm_kws = str;
    int dist = levenshtein(kws, bm_kws);
    //printf ("$$$ kws:%s, bm_kws:%s, dist:%d\n", kws, bm_kws, dist);
    return dist;
}

void _PrintfLevenshteinResult(char* tag, LVP_KWS_PARAM_LIST g_kws_list, int kws_index, char *bm_kws)
{
#ifdef __OPEN_PRINTF__
    char *kws = g_kws_list.kws_param_list[kws_index].kws_words;
    int dist = levenshtein(kws, bm_kws);
    printf ("%s kws:%s, bm_kws:%s, dist:%d\n", tag, kws, bm_kws, dist);
#endif
}

#define _LEVENSHTEIN_LENGTH_    10
static int g_levenshtein_list[_LEVENSHTEIN_LENGTH_];
static int g_levenshtein_list_cnt = 0;
void _InsertLevenshteinIndex(int index)
{
    g_levenshtein_list[g_levenshtein_list_cnt] = index;
    g_levenshtein_list_cnt += 1;
}

void _ClearLevenshteinList(void)
{
    memset(g_levenshtein_list, 0, sizeof(g_levenshtein_list));
    g_levenshtein_list_cnt = 0;
}

int _CheckLevenshteinResult(LVP_KWS_PARAM_LIST g_kws_list, int mini_dist, int ctc_kws_index)
{
    if (mini_dist != -1) {
        for (int i = 0; i < CONFIG_BEAM_SIZE; i++) {
            for (int j = 0; j < g_levenshtein_list_cnt; j++) {
                _PrintfLevenshteinResult("#$#  result  #$#", g_kws_list, g_levenshtein_list[j], bm_str[i]);
                if (g_levenshtein_list[j] == ctc_kws_index) {
                    return 0;
                }
            }
        }
    }

    return -1;
}

static float s_bm_decoder_window[CONFIG_KWS_MODEL_OUTPUT_LENGTH];
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
    unsigned int mini_dist = -1;
    int levenshtein_threshold = CONFIG_LEVENSHTEIN_THRESHOLD/100;

#ifdef CONFIG_LVP_KWS_HUAWEI_WATCH_V0DOT1DOT1_2020_1109
    if (g_kws_list.kws_param_list[ctc_kws_index].kws_value != CONFIG_KEYWORD_SHANG_YI_SHOU_VALUE) return 0;
#endif
    _ClearLevenshteinList();
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
        KwsCtcBeamSearchLmScore(&s_bm_decoder_window[0]
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

                    int dist = _GetKwsLevenshtein(g_kws_list, j, bm_str[i]);
                    // printf ("#@#@ mini_dist: %d\n", mini_dist, dist);
                    if (dist < mini_dist && dist < levenshtein_threshold) {
                        mini_dist = dist;
                        _ClearLevenshteinList();
                        _InsertLevenshteinIndex(j);
                        _PrintfLevenshteinResult("###  clr_add ###", g_kws_list, j, bm_str[i]);
                    } else if (dist == mini_dist) {
                        _InsertLevenshteinIndex(j);
                        _PrintfLevenshteinResult("=== same_add ===", g_kws_list, j, bm_str[i]);
                    }
                }
            }
        }
        if (index > 0 && p == (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - 1)) {
            p = -1;
            win_length = index;
        }
    }

    int ret = _CheckLevenshteinResult(g_kws_list, mini_dist, ctc_kws_index);

    return ret;
}
