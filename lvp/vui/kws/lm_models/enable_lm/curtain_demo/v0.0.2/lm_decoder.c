
#include <autoconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <lvp_context.h>
#include <lvp_param.h>

//#include <kws_list.h>

#include <syllable_table.h>

#include <decoder.h>

// #define __OPEN_PRINTF__
#define LOG_TAG "[LVP_CTC]"

static PREFIX_LIST g_prefix[CONFIG_BEAM_SIZE];
static char bm_str[CONFIG_BEAM_SIZE][38];
static float s_bm_decoder_window[CONFIG_KWS_MODEL_OUTPUT_LENGTH];

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
                if ((idx-1 < 57 && idx-1 > 20) || (idx-1 == 61)) strcat (bm_str[i], " ");
                prefix_length[i]++;
            }
        }
    }
}

float _GetKwsDimSim(LVP_KWS_PARAM_LIST g_kws_list, int kws_index, int beam_index, int beam_str_len, int kws_offset)
{
    char *kws = g_kws_list.kws_param_list[kws_index].labels;
    float tot = g_kws_list.kws_param_list[kws_index].label_length/2*2.1;
    float dist = 0.f;

    PREFIX_LIST *prefix = &g_prefix[beam_index];

    int len;
    int bm_offset = 0;
    if (g_kws_list.kws_param_list[kws_index].label_length > beam_str_len) {
        len = beam_str_len;
    }
    else {
        len = g_kws_list.kws_param_list[kws_index].label_length;
        bm_offset = beam_str_len - g_kws_list.kws_param_list[kws_index].label_length;
    }

    //printf("bm_str:%s, kws:%s\n", bm_str[beam_index], g_kws_list.kws_param_list[kws_index].kws_words);
    //printf("bm_offset: %d, kws_offset:%d\n", bm_offset, kws_offset) ;

    for (int m = 0, n = 0; n < len; ) {
        unsigned int a_id = kws[n + kws_offset];
        unsigned int b_id = prefix->prefix_set_prev[m + bm_offset] - 1;
        if (b_id == 64) {// 跳过开头、结尾的blank音节
            m ++;
            continue;
        }
        if ((b_id > 20 && b_id < 57) || (a_id == 61)) { // BM 是韵母
            if ((a_id > 20 && a_id < 57) || (a_id == 61)) {

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

    int kws_len = strlen(kws);
    int bm_len = strlen(bm_kws);
    if (kws_len>bm_len) kws = kws + (kws_len - bm_len);
    int dist = levenshtein(kws, bm_kws);
    //printf ("$$$ kws:%s, bm_kws:%s, [%d, %d], dist:%d, tot:%d\n", kws, bm_kws, kws_len, bm_len, dist, tot);
    return (float)dist/tot;
}

float _GetBmstrAndKwsDimsim(LVP_KWS_PARAM_LIST g_kws_list, int kws_index, int beam_index, int beam_str_len)
{
    float dimsim_dist;
    if (g_kws_list.kws_param_list[kws_index].label_length > CONFIG_PREFIX_CHAR_LENGTH) {
        // 比较前面 CONFIG_PREFIX_CHAR_LENGTH 个音节
        int kws_offset = 0;
        float head_dimsim_dist = _GetKwsDimSim(g_kws_list, kws_index, beam_index, beam_str_len, kws_offset);
        if (head_dimsim_dist == 0.f) {
#ifdef __OPEN_PRINTF__
            printf("head_dimsim_dist:%f\n", head_dimsim_dist);
            printf("bm_str:%s, kws:%s\n", bm_str[beam_index], g_kws_list.kws_param_list[kws_index].kws_words);
            printf("bm_offset: %d, kws_offset:%d\n",  beam_str_len - g_kws_list.kws_param_list[kws_index].label_length, kws_offset) ;
#endif
        }

        // 比较后面 CONFIG_PREFIX_CHAR_LENGTH 个音节
        kws_offset = g_kws_list.kws_param_list[kws_index].label_length - CONFIG_PREFIX_CHAR_LENGTH;
        float tail_dimsim_dist = _GetKwsDimSim(g_kws_list, kws_index, beam_index, beam_str_len, kws_offset);
        if (tail_dimsim_dist == 0.f) {
#ifdef __OPEN_PRINTF__
            printf("tail_dimsim_dist:%f\n", tail_dimsim_dist);
            printf("bm_str:%s, kws:%s\n", bm_str[beam_index], g_kws_list.kws_param_list[kws_index].kws_words);
            printf("bm_offset: %d, kws_offset:%d\n",  beam_str_len - g_kws_list.kws_param_list[kws_index].label_length, kws_offset) ;
#endif
        }

        if (head_dimsim_dist > tail_dimsim_dist) dimsim_dist = tail_dimsim_dist;
        else dimsim_dist = head_dimsim_dist;
    } else {
        int kws_offset = 0;
        dimsim_dist = _GetKwsDimSim(g_kws_list, kws_index, beam_index, beam_str_len, kws_offset);
        if (dimsim_dist == 0.f) {
#ifdef __OPEN_PRINTF__
            printf("dimsim_dist:%f\n", dimsim_dist);
#endif
            //_GetKwsDimSimTest(g_kws_list, kws_index, beam_index, beam_str_len, kws_offset);
        }
    }

    return dimsim_dist;
}

float _FastCtcBlockScore(float *ctc_decoder_window
                        , int total_valid_frame_num
                        , int T2
                        , LVP_KWS_PARAM *kws_param_list
                        , int label_length)
{
    float ctc_score = LvpFastCtcBlockScore(&ctc_decoder_window[T2 * CONFIG_KWS_MODEL_OUTPUT_LENGTH]
                    , total_valid_frame_num - T2// - 1
                    , &ctc_decoder_window[0]
                    , T2// - 1
                    , CONFIG_KWS_MODEL_OUTPUT_LENGTH
                    , CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1
                    , (unsigned char *)(kws_param_list->labels)
                    , label_length);

    return ctc_score;
}
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
    float mini_levenshtein_dist = 3.4e+38;
    int mini_dimsim_kws_index = -1;

    // 两种相似度的阈值
    float levenshtein_threshold = (float)CONFIG_LEVENSHTEIN_THRESHOLD/100.f;
    float dimsim_threshold = (float)CONFIG_DIMSIM_THRESHOLD/100.f;

    if ((g_kws_list.kws_param_list[ctc_kws_index].major&0xf0) != 0x80) {
        return 0;
    }

    _ClearPrefixList(CONFIG_BEAM_SIZE, prefix_list);
    int beam_size = 0;
    int win_length = CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;

    int index = s_ctc_index % CONFIG_KWS_MODEL_DECODER_WIN_LENGTH;

#if 0
    // 查看激活词各音节的每一帧的概率值
    float s_bm_decoder_window_1[CONFIG_KWS_MODEL_DECODER_WIN_LENGTH*CONFIG_KWS_MODEL_OUTPUT_LENGTH] ALIGNED_ATTR(16);
    memcpy(s_bm_decoder_window_1
            , &s_ctc_decoder_window[index * CONFIG_KWS_MODEL_OUTPUT_LENGTH]
            , (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - index) * CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    memcpy(&s_bm_decoder_window_1[(CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - index) * CONFIG_KWS_MODEL_OUTPUT_LENGTH]
            , &s_ctc_decoder_window[0]
            , (index) * CONFIG_KWS_MODEL_OUTPUT_LENGTH * sizeof(float));
    printf_syllable(&s_bm_decoder_window_1[0], g_kws_list.kws_param_list[ctc_kws_index].kws_words, g_kws_list.kws_param_list[ctc_kws_index].labels, g_kws_list.kws_param_list[ctc_kws_index].label_length);
#endif
    if (ctc_score > 85.f && ctc_score > g_kws_list.kws_param_list[ctc_kws_index].threshold/10.f + 5.f) {
        // int kws_value = g_kws_list.kws_param_list[ctc_kws_index].kws_value;
        if (ctc_score > g_kws_list.kws_param_list[ctc_kws_index].threshold/10.f + 8.f) return 0;
    }
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
                    , 0.99f
                    , CONFIG_KWS_MODEL_BEAMSEARCH_AND_LM_CUTOFF_TOP_N
                    , prefix_list
                    , context);
        if (index > 0 && p == (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - 1)) {
            p = -1;
            win_length = index;
        }
    }

    memcpy (&g_prefix[0], prefix_list, CONFIG_BEAM_SIZE*sizeof(PREFIX_LIST));

    // 获取 bm+lm 识别到的拼音字符串
    _GetBeamSearchStr(CONFIG_BEAM_SIZE, prefix_length);
#ifdef __OPEN_PRINTF__
    for (int i = 0; i < CONFIG_BEAM_SIZE; i++) {
        printf ("[%d][%d]%s\n", i, prefix_length[i], bm_str[i]);
    }
#endif
    for (int i = 0; i < 2; i++) {
        if (prefix_length[i] < 8) break;
        for (int j = 0; j < g_kws_list.count; j++) {
            if (prefix_length[i] <= (g_kws_list.kws_param_list[ctc_kws_index].label_length - 1) && prefix_length[i] <= (CONFIG_PREFIX_CHAR_LENGTH - 1)) continue;

            float dimsim_dist = _GetBmstrAndKwsDimsim(g_kws_list, j, i, prefix_length[i]);

            if (j == ctc_kws_index) {
#ifdef __OPEN_PRINTF__
                printf ("#@#@ [%s, %s] leven_dist:%f[%f], dimsim:%f[%f], s_diff:%f\n"
                                    , g_kws_list.kws_param_list[j].kws_words, bm_str[i], _GetKwsLevenshtein(g_kws_list, j, bm_str[i]), levenshtein_threshold, dimsim_dist, dimsim_threshold, ctc_score - g_kws_list.kws_param_list[j].threshold/10.f);
#endif
            }
            if (dimsim_dist < mini_dimsim_dist && dimsim_dist < dimsim_threshold) {
                mini_dimsim_dist = dimsim_dist;
                mini_dimsim_kws_index = j;

#ifdef __OPEN_PRINTF__
                printf ("###  clr_add ### %s, %d, %d, %s, mini_dimsim:%f[%f]\n", g_kws_list.kws_param_list[j].kws_words, j, ctc_kws_index, bm_str[i], mini_dimsim_dist, dimsim_threshold);
#endif
                if (dimsim_dist == 0.f) {
                    *bm_activation_score = _FastCtcBlockScore(s_ctc_decoder_window
                                                        , valid_frame_num
                                                        , index
                                                        , &g_kws_list.kws_param_list[j]
                                                        , prefix_length[i]);
#ifdef __OPEN_PRINTF__
                    printf (LOG_TAG"ctc_score:%f\n", *bm_activation_score);
#endif
                    if (*bm_activation_score > 100 - prefix_length[i]/2.f * 5.f) {
                        if ((j == ctc_kws_index)
                        || (g_kws_list.kws_param_list[j].label_length >= g_kws_list.kws_param_list[ctc_kws_index].label_length
                        && g_kws_list.kws_param_list[ctc_kws_index].label_length >= 8)) {
                            return j;
                        }
                    }
                }
            } else if (dimsim_dist < mini_dimsim_dist && (dimsim_dist < 110.f || ctc_score - g_kws_list.kws_param_list[j].threshold/10.f > 8.f )) {
                float tmp_levenshtein_dist = _GetKwsLevenshtein(g_kws_list, j, bm_str[i]);
                if (tmp_levenshtein_dist < mini_levenshtein_dist && tmp_levenshtein_dist < levenshtein_threshold && j == ctc_kws_index) {
                    mini_levenshtein_dist = tmp_levenshtein_dist;
                    mini_dimsim_kws_index = j;
#ifdef __OPEN_PRINTF__
                    printf ("#=#  lev_add #=# %s, %d, %d, %s, dim:%f[%f], lev:%f[%f], tot:%d, dist:%f\n"
                                        , g_kws_list.kws_param_list[j].kws_words
                                        , j, ctc_kws_index, bm_str[i]
                                        , dimsim_dist
                                        , dimsim_threshold
                                        , tmp_levenshtein_dist
                                        , levenshtein_threshold
                                        , strlen(g_kws_list.kws_param_list[j].kws_words)
                                        , tmp_levenshtein_dist * strlen(g_kws_list.kws_param_list[j].kws_words));
#endif
                    continue;
                }
#ifdef __OPEN_PRINTF__
# if 0
                printf ("#=#  waite  #=# %s, %d, %d, %s, dim:%f[%f], lev:%f[%f], tot:%d, tmp_levenshtein_dist:%f\n"
                                    , g_kws_list.kws_param_list[j].kws_words
                                    , j, ctc_kws_index, bm_str[i]
                                    , dimsim_dist
                                    , dimsim_threshold
                                    , tmp_levenshtein_dist
                                    , levenshtein_threshold
                                    , strlen(g_kws_list.kws_param_list[j].kws_words)
                                    , tmp_levenshtein_dist * strlen(g_kws_list.kws_param_list[j].kws_words));
# endif
#endif
            } else if (dimsim_dist == mini_dimsim_dist && j == ctc_kws_index) {
                mini_dimsim_kws_index = j;
#ifdef __OPEN_PRINTF__
                printf ("=== same_add === %s, %d, %d, %s, %f\n", g_kws_list.kws_param_list[j].kws_words, j, ctc_kws_index, bm_str[i], mini_dimsim_dist);
#endif
            }
        }
    }

    if (mini_dimsim_kws_index == ctc_kws_index) return 0;

    return -1;
}
