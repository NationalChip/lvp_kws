/* Grus
* Copyright (C) 2001-2021 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* ctc_model.h: Kws Model
*
*/

#ifndef __CTC_MODEL_H__
#define __CTC_MODEL_H__

#include <driver/gx_snpu.h>

int LvpModelSetUseXipModelFlag(int flag);
int LvpModelGetUseXipModelFlag(void);
int LvpModelGetOpsSize(void);
int LvpModelGetDataSize(void);
int LvpModelGetTmpSize(void);
int LvpModelGetCmdSize(void);
int LvpModelGetWeightSize(void);
void LvpSetSnpuTask(GX_SNPU_TASK* snpu_task);
int LvpCTCModelInitSnpuTask(GX_SNPU_TASK *snpu_task);
const char *LvpCTCModelGetKwsVersion(void);
void *LvpCTCModelGetSnpuOutBuffer(void *snpu_buffer);
void *LvpCTCModelGetSnpuFeatsBuffer(void *snpu_buffer);
void *LvpCTCModelGetSnpuStateBuffer(void *snpu_buffer);
unsigned int LvpCTCModelGetSnpuFeatsDim(void);
unsigned int LvpCTCModelGetSnpuStateDim(void);

#endif /* __CTC_MODEL_H__ */