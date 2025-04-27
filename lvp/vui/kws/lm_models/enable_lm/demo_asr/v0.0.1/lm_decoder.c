
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

#define ANY_MATCH_STASHED (-2)
#define DELAY_COUNT 18
static int *s_list_lens = NULL;
static int *s_list_offset = NULL;
static int *s_active_index_stash = NULL;
static int s_max_deepth = 0;
static int any_match_flag = 0;
static int last_mask = 0;
static char *s_delay_any_match_stash = NULL;
static int deepth = 0;
static int delay_count = 0;

int cleanDeepth(void)
{
    deepth = 0;
    delay_count = 0;

    return 0;
}

int LvpInitSegmentation(int *lens, int *offset, int *index_stash, char *str_stash,int deepth)
{
    s_list_lens = lens;
    s_list_offset = offset;
    s_active_index_stash = index_stash;
    s_max_deepth = deepth;
    s_delay_any_match_stash = str_stash;
    for (int i = 0; i < s_max_deepth; i++)
        s_active_index_stash[i] = -1;

    return 0;
}

static int GetRegexInput(char *str, char *src, int len, int *prefix_length)
{
    for (int m = 0; m < CONFIG_PREFIX_CHAR_LENGTH; m++) {
        int idx = src[m];
        if (idx < 65 && idx > 1) {
            strcat (str, g_syllable_table[idx - 1]);
            strcat (str, " ");
            ++(*prefix_length);
        }
    }
    //if (deepth >= 2) printf ("[]%s\n", str);
    return 0;
}

static int GetRegexRule(char *kws, char *src, int len)
{
    int t = 1;
    // strcat(kws, ".* ?");
    for (int i = 0; i < len; i++) {
        char idx = src[i];
        if (idx > 0 && idx < 65) {
            strcat (kws, g_syllable_table_fuzzy[idx]);
        }
        else if (idx > 65 || idx < 0) {
            strcat(kws, ".*");
            any_match_flag = 1;
            t = 0;
        }

        if (i == len - 1) return 0;
            if (i % 2 == t) {
                strcat (kws, " ");
            } else {
                strcat (kws, " ");
        }

    }

    return 0;
}

static int GetFinalMatchedStr(char *str, LVP_KWS_SEGMENTATION_PARAM *src)
{
    for(int k = 0; k < s_max_deepth; k++) {
        if (s_active_index_stash[k] >= 0) {
            // printf("[%d] %d \n", k, src[s_list_offset[k] + s_active_index_stash[k]].id);
            strcat(str, src[s_list_offset[k] + s_active_index_stash[k]].kws_words);
            strcat(str, " ");
            s_active_index_stash[k] = -1;
        }
        else if (s_active_index_stash[k] == ANY_MATCH_STASHED) {
            // printf("[%d]%s \n", k, s_delay_any_match_stash + (MAX_STR_STASH * k));
            strcat(str, s_delay_any_match_stash + (MAX_STR_STASH * k));
            s_active_index_stash[k] = -1;
            // printf("[c]%d\n", s_active_index_stash[k]);
        }
    }


    return 0;
}

int LvpGetSegmentationDeepth(void)
{
    return deepth;
}

int LvpSegmentationCtcMatched(int id, int next)
{
    s_active_index_stash[deepth] = id;
    last_mask = next;
    ++deepth;

    return 0;
}


int LvpDoBeamSearch(LVP_CONTEXT        *context,
                     LVP_KWS_SEGMENTATION_PARAM *table,
                     PREFIX_LIST        *prefix_list,
                     float              *s_ctc_decoder_window,
                     int                valid_frame_num,
                     float              ctc_score,
                     int                ctc_kws_index,
                     int                s_ctc_index,
                     float              *bm_activation_score)
{
    int flag = 0;

    int beam_size = 0;
    int kws_list_len = 0;
    int is_full_compared = 1;
    static char last_active[120];
    static int last_active_id = 0;
    static int is_delay = 0;

    KwsCtcBeamSearchLmScore(s_ctc_decoder_window
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

    memcpy (&g_prefix[0], prefix_list, CONFIG_BEAM_SIZE*sizeof(PREFIX_LIST));

    // str
    if (delay_count) --delay_count;
    int prefix_length[CONFIG_BEAM_SIZE] = {0};

        for (int i = 0; i < CONFIG_BEAM_SIZE; i++) {
            PREFIX_LIST *prefix = &g_prefix[i];
            memset (str[i], '\0', sizeof(str[i]));
            prefix_length[i] = 0;
            GetRegexInput(str[i], prefix->prefix_set_prev, CONFIG_PREFIX_CHAR_LENGTH, &prefix_length[i]);
        }

        for (int i = 0; i < CONFIG_BEAM_SIZE; i++) {
            is_full_compared = 1;
            int loop_count = s_list_lens[deepth];
            LVP_KWS_SEGMENTATION_PARAM *list = &table[s_list_offset[deepth]];

            for (int j = 0; j < loop_count; j++) {

                kws_list_len = list[j].label_length - 0;

                if (list[j].ctc_enable)
                    continue;

                if (prefix_length[i] < kws_list_len) {
                    is_full_compared = 0;
                    continue;
                }

                if ((deepth > 0 && (!(list[j].prev & last_mask)))) {
                    continue;
                }

                char kws[120];
                memset(kws, '\0', sizeof(kws));
                GetRegexRule(kws, list[j].labels, kws_list_len);
                //if (deepth >= 2) printf("[reg]%s\n",kws);

                s_active_index_stash[deepth] = -1;
                re_t pattern = re_compile(kws);
                /* Check if the regex matches the text: */
                int match_length = 0;
                int match_idx = re_matchp(pattern, str[i], &match_length);
                int ret = 0;

                if (match_idx != -1) {
                    last_mask = list[j].next;
                    if (any_match_flag) {
                        // printf("c: %d\n", deepth);
                        memset(s_delay_any_match_stash + (deepth * MAX_STR_STASH), '\0', sizeof(char) * MAX_STR_STASH);
                        strcat(s_delay_any_match_stash + (MAX_STR_STASH * deepth), str[i]);
                        s_active_index_stash[deepth] = ANY_MATCH_STASHED;
                        any_match_flag = 0;
                    }
                    else
                        s_active_index_stash[deepth] = j;

                    deepth += 1;
                    if (list[j].next > 0) return 0;
                    ret = 1;
                }
                else ret = 0;

               if (ret && (list[j].next == LEAF)) {
                   char final_st[120];
                   memset(final_st, '\0', sizeof(final_st));
                   if (is_delay) {
                    //    printf("[la]%s \n", last_active);
                        strcat(final_st, last_active);
                        GetFinalMatchedStr(final_st, table);
                        printf("\n\n\n"LOG_TAG"[BM] Activation Kws:%s \n\n\n", final_st);
                        is_delay = 0;
                        delay_count = 0;
                        deepth = 0;
                        return 1;
                   }
                   else {
                        GetFinalMatchedStr(final_st, table);
                        printf("\n\n\n"LOG_TAG"[BM] Activation Kws:%s \n\n\n", final_st);

                        deepth = 0;
                        return 1;
                   }
               }
               else if (ret && (list[j].next == DELAY)) {
                   //延迟激活储存上次结果
                   is_delay = 1;
                   delay_count += DELAY_COUNT;
                   memset(last_active, '\0', sizeof(last_active));
                   GetFinalMatchedStr(last_active, table);
                   return 0;
               }
        }
        if ((deepth > 0 && is_full_compared) || (is_delay && delay_count == 0)) {
            if (!is_delay) {
                deepth = 0;
                return 0;
            }
            else {
                deepth = 0;
                is_delay = 0;
                printf("\n\ndelay active!\n");
                printf(LOG_TAG"[BM] Activation Kws:%s \n\n\n", last_active);
                return 1;
            }
        }
    }
    return -1;
}
