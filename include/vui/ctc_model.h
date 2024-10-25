/* Grus
* Copyright (C) 2001-2020 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* ctc_model.h: Kws Model
*
*/

#ifndef __CTC_MODEL_H__
#define __CTC_MODEL_H__

#include <driver/gx_snpu.h>

enum {
    MODEL_TYPE_CTC_KWS,
    MODEL_TYPE_BUN_KWS
};

int LvpModelGetOpsSize(void);
int LvpModelGetDataSize(void);
int LvpModelGetTmpSize(void);
int LvpModelGetCmdSize(void);
int LvpModelGetWeightSize(void);
void LvpSetSnpuTask(GX_SNPU_TASK* snpu_task);
void LvpSetSnpuRunInFlash(void);
void LvpSetSnpuRunInSram(void);
int LvpCTCModelInitSnpuTask(GX_SNPU_TASK *snpu_task);
const char *LvpCTCModelGetKwsVersion(void);
void *LvpCTCModelGetSnpuOutBuffer(void *snpu_buffer);
void *LvpCTCModelGetSnpuFeatsBuffer(void *snpu_buffer);
void *LvpCTCModelGetSnpuStateBuffer(void *snpu_buffer);
unsigned int LvpCTCModelGetSnpuFeatsDim(void);
unsigned int LvpCTCModelGetSnpuStateDim(void);
void LvpSwitchKwsModel(int model_type);
#endif /* __CTC_MODEL_H__ */