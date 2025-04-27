#ifndef __LM_DECODER_H__
#define __LM_DECODER_H__

#include <lvp_context.h>
#include <lvp_param.h>
int LvpInitSegmentation(int *lens, int *offset, int *index_stash, char *str_stash,int deepth);
int LvpGetSegmentationDeepth(void);
int LvpSegmentationCtcMatched(int id, int next);
int LvpDoBeamSearch(LVP_CONTEXT        *context,
                     LVP_KWS_SEGMENTATION_PARAM *table,
                     PREFIX_LIST        *prefix_list,
                     float              *s_ctc_decoder_window,
                     int                valid_frame_num,
                     float              ctc_score,
                     int                ctc_kws_index,
                     int                s_ctc_index,
                     float              *bm_activation_score);
int cleanDeepth(void);

#endif /* __LM_DECODER_H__ */
