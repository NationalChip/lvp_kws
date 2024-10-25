/* Voice Signal Preprocess
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * emphasis.h: emphasis
 *
 */

#ifndef __LVP_VMA_EMPHASIS_H__
#define __LVP_VMA_EMPHASIS_H__

int PreEmphasisF32(short *in, float *out, int len, short *last_in);
int DeEmphasisF32(float *in, float *out, int len, float *last_in);

int PreEmphasisS16(short *in, short *out, int len, short *last_in);
int DeEmphasisS16(short *in, short *out, int len, short *last_in);

#endif
