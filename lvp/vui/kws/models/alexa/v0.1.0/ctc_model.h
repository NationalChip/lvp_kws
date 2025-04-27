/* Grus
 * Copyright (C) 2001-2019 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * ctc_model.h: Kws Model
 *
 */

#ifndef __CTC_MODEL_H__
#define __CTC_MODEL_H__

#include <driver/gx_snpu.h>

struct kws_in_out {
	short Feats[1][11][40];
	short State_c0[1][3][64];
	short State_c1[1][3][64];
	short State_c2[1][3][64];
	short State_c0_out[1][3][64];
	short State_c1_out[1][3][64];
	short State_c2_out[1][3][64];
	float Model_rnn_out[1][1][64];
} ALIGNED_ATTR(16);

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
