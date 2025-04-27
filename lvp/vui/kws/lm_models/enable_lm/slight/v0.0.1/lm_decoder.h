#ifndef __LM_DECODER_H__
#define __LM_DECODER_H__

#include <lvp_context.h>
#include <lvp_param.h>

int LvpDoBeamSearch(LVP_CONTEXT        *context,
                     LVP_KWS_PARAM_LIST g_kws_list,
                     PREFIX_LIST        *prefix_list,
                     float              *s_ctc_decoder_window,
                     int                valid_frame_num,
                     float              ctc_score,
                     int                ctc_kws_index,
                     int                s_ctc_index);

#endif /* __LM_DECODER_H__ */
