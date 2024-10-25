#include <autoconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <lvp_context.h>
#include <lvp_param.h>

#include "decoder.h"
#include "kws_strategy.h"

#define LOG_TAG "[##KWL##]"

static int s_kws_decoder_level = 0;

void KwsLevelDecoderInit(void)
{
    s_kws_decoder_level = 0;
}

int ClearKwsDecoderLevel(void)
{
    s_kws_decoder_level = 0;
    return 0;
}

int GetKwsDecoderLevel(void)
{
    return s_kws_decoder_level;
}

int KwsLevelDecoderTick(LVP_CONTEXT *context)
{
#ifdef CONFIG_ENABLE_KWS_LEVEL_DECODER
    static int cnt = 0;
    static int last_kws_decoder_level = 0;
    if (s_kws_decoder_level && (last_kws_decoder_level == s_kws_decoder_level)) { // 超时 s_kws_decoder_level 清0
        cnt++;
        if (cnt > CONFIG_KWS_MODEL_DECODER_WIN_LENGTH + 10) {
            printf (LOG_TAG"Clear s_kws_decoder_level !!!!\n");
            s_kws_decoder_level = 0;
            cnt = 0;
        }
    } else {
        last_kws_decoder_level = s_kws_decoder_level;
        cnt = 0;
    }
#endif
    return 0;
}

/***************************************************************************
**
** 按照从上到下的树状来遍历解码:
**             TOP              // 初始态：只去识别带 TOP 的 KWS
**           /  |  \
**          /   |   \
**        END  END  MID         // TOP 下，只去识别仅仅包含 END 或者 MID 的 KWS
**                   |
**                   |
**                MID|END       // MID 下，只去识别带 MID|END 的 KWS
**
***************************************************************************/
int KwsLevelDecoderRun(LVP_ACTIVATION_KWS *activation_kws)
{
#ifdef CONFIG_ENABLE_KWS_LEVEL_DECODER
    int kws_index = activation_kws->kws_index;
    LVP_KWS_PARAM_LIST *kws_list = LvpGetKwsParamList();

    LVP_KWS_PARAM *kws_src = &(kws_list->kws_param_list[kws_index]);
    int kws_level_type = kws_src->major & KWS_LEVEL_MASK;

    if (kws_level_type == 0 ) {
        s_kws_decoder_level = 0;
        printf (LOG_TAG"[%s]Kws Hit !!!!\n", kws_src->kws_words);
        return 0;
    } else if ((s_kws_decoder_level == 0) && (kws_level_type & KWS_LEVEL_TOP_MASK)) {
        s_kws_decoder_level = kws_level_type & KWS_LEVEL_TOP_MASK;
        printf (LOG_TAG"TOP%d Hit !!!!\n", kws_level_type>>8);
        goto reset;
    } else if ((s_kws_decoder_level != 0) && (s_kws_decoder_level == (kws_level_type & s_kws_decoder_level))) {
        if (KWS_LEVEL_MID == (kws_level_type & KWS_LEVEL_MID)) {
            s_kws_decoder_level = KWS_LEVEL_MID;
            printf (LOG_TAG"TOP%d-MID Hit !!!!\n", (kws_level_type & s_kws_decoder_level)>>8);
            goto reset;
        } else if (KWS_LEVEL_END == (kws_level_type & KWS_LEVEL_END)) {
            s_kws_decoder_level = 0;
            printf (LOG_TAG"TOP%d-END Hit !!!!\n", (kws_level_type & s_kws_decoder_level)>>8);
            return 0;
        }
    }  else if (s_kws_decoder_level == KWS_LEVEL_MID) {
        if (kws_level_type == (KWS_LEVEL_MID|KWS_LEVEL_END)) {
            s_kws_decoder_level = 0;
            printf (LOG_TAG"END Hit !!!!\n");
            return 0;
        }
    }

reset:
    {
        // extern int s_max_score_index;
        int delay_2_decode_num = (kws_list->kws_param_list[kws_index].major & KWS_TYPE_DEALY2DECODE_MASK) >> 16;
        if (delay_2_decode_num == 0) delay_2_decode_num = CONFIG_VPA_DELAY_TO_DECODE_CNT;// 保持原有方案不受影响
        int offset  = activation_kws->score_index - delay_2_decode_num;
        printf (LOG_TAG"score_index:%d\n", activation_kws->score_index);
        printf (LOG_TAG"offset:%d\n", offset);
        printf (LOG_TAG"delay_2_decode_num:%d\n", delay_2_decode_num);
        printf (LOG_TAG"offset2:%d\n", CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - delay_2_decode_num - offset/2 + 1);
# ifdef CONFIG_ENABLE_CLEAR_TOP_KWS_AUDIO
        ResetCtcWinodwUser(0, CONFIG_KWS_MODEL_DECODER_WIN_LENGTH - delay_2_decode_num - offset/2 + 1);
# endif
    }
    return -1;
#else
    return 0;
#endif
}
