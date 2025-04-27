/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_vpr.h:
 *
 */

#ifndef __LVP_VPR_H__
#define __LVP_VPR_H__

#include <driver/gx_snpu.h>
#include <driver/gx_pmu_ctrl.h>
#include <lvp_context.h>

int LvpVpRun(LVP_CONTEXT *context);
int LvpVpDecoder(void);
int LvpVpLearn(void);

void LvpLoadVpNpuModle(void); // load npu mode
#endif
