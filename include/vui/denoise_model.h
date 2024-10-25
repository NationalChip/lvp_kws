/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * denoise_model.h: Denoise Model
 *
 */

#ifndef __DENOISE_MODEL_H__
#define __DENOISE_MODEL_H__

#include <driver/gx_snpu.h>

int LvpDenoiseModelGetOpsSize(void);
int LvpDenoiseModelGetDataSize(void);
int LvpDenoiseModelGetTmpSize(void);
int LvpDenoiseModelGetCmdSize(void);
int LvpDenoiseModelGetWeightSize(void);
void LvpDenoiseSetSnpuTask(GX_SNPU_TASK* snpu_task);
int LvpDenoiseModelInitSnpuTask(GX_SNPU_TASK *snpu_task);
const char *LvpDenoiseModelGetVersion(void);
void *LvpDenoiseModelGetSnpuFeatsBuffer(void *snpu_buffer);
void *LvpDenoiseModelGetSnpuOutBuffer(void *snpu_buffer);
int LvpDenoiseModelGetSnpuFeatsSize(void *snpu_buffer);
int LvpDenoiseModelGetOutBufferSize(void *snpu_buffer);


#endif /* __CTC_MODEL_H__ */
