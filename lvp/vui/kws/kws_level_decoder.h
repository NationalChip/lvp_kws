#ifndef __KWS_LEVEL_DECODER_H__
#define __KWS_LEVEL_DECODER_H__

#include <lvp_context.h>
#include <lvp_param.h>
#include <kws_strategy.h>

void KwsLevelDecoderInit(void);
int ClearKwsDecoderLevel(void);
int GetKwsDecoderLevel(void);
int KwsLevelDecoderTick(LVP_CONTEXT *context);
int KwsLevelDecoderRun(LVP_ACTIVATION_KWS *activation_kws);

#endif /* __KWS_LEVEL_DECODER_H__ */
