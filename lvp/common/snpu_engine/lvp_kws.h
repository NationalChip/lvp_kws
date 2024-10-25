/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mode.h: vsp mode
 *
 */

#ifndef __LVP_KWS_H__
#define __LVP_KWS_H__

#include <driver/gx_snpu.h>
#include <driver/gx_pmu_ctrl.h>
#include <lvp_context.h>
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
#include <ctc_model.h>
#include <decoder.h>
#endif

int LvpKwsRun(LVP_CONTEXT *context);
int LvpKwsInit(GX_SNPU_CALLBACK callback, GX_WAKEUP_SOURCE start_mode);
int LvpKwsDone(void);
void LvpLoadVpNpuModle(void); // load npu mode
int LvpKwsSwitchModelByEnvNoise(LVP_CONTEXT *context);
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
int LvpKwsSwitchModelByKwsState(VUI_KWS_STATE state);
#endif

#endif /* __LVP_KWS_H__ */
