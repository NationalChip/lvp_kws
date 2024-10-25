/* Grus
 * Copyright (C) 2001-2023 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_rnn_sed.h:
 *
 */

#ifndef __LVP_DENOISE_H__
#define __LVP_DENOISE_H__

#include <autoconf.h>
#include <driver/gx_snpu.h>
#include <driver/gx_pmu_ctrl.h>
#include <lvp_context.h>

#ifndef CONFIG_ENABLE_HARDWARE_FFT
#define PRE_FILL_CONTEXT_NUM (CONFIG_SED_MODEL_INPUT_WIN_LENGTH / LvpGetPcmFrameNumPerContext())
#else
#define PRE_FILL_CONTEXT_NUM (CONFIG_SED_MODEL_INPUT_WIN_LENGTH / LvpGetPcmFrameNumPerContext())
#endif
int LvpRnnSedInit(GX_SNPU_CALLBACK callback, GX_WAKEUP_SOURCE start_mode);
int LvpRnnSedRun(LVP_CONTEXT *context, float *amp);
int LvpRnnSedDone(void);

#endif /* __LVP_KWS_H__ */
