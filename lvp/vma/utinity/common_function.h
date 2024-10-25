/* Voice Signal Preprocess
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * emphasis.h: emphasis
 *
 */

#ifndef __LVP_VMA_COMMON_FUNC_H__
#define __LVP_VMA_COMMON_FUNC_H__

#define SAFE_MAX(x,y)  ({\
          typeof(x) _x = x;\
          typeof(y) _y = y;\
          (void)(&_x == &_y);\
          _x>_y?_x:_y;\
        })

float GetSumF32(float *data, int start, int end, int is_avg);
int HardFftDeemphasis(void *in);
int CalcAmpPhaseF32(void *in, float *amp, float *phase, int len, int scale);
int CalcAmpPhaseFixed(short *in, float *amp, float *phase, int len, int fft_shift);
int getHighBit(int value);
int ShiftWindow(float *in, float *out, int win_length, int shift_length);
#endif
