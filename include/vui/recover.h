/* Grus
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * recover.h: recover wave
 *
 */

#ifndef __RECOVER_H__
#define __RECOVER_H__

#ifdef CONFIG_NN_DENOISE_SAMPLE_RATE_8K
#define RECOVER_WAV_POINTS 80
#define RECOVER_FFT_POINTS 129
#else
#define RECOVER_WAV_POINTS 160
#define RECOVER_FFT_POINTS 257
#endif

int RecoverInit(void);
int ResetDeHanmingWiondowBuffer(void);
int RecoverDenoiseWav(LVP_CONTEXT *context, float *amp, float *phase, float *ifft_tmp_buffer, float *ifft_out_buffer, short *wav);
int RecoverWav(float *amp, float *phase, float *ifft_tmp_buffer, float *ifft_out_buffer, short *wav);
float getNoisyWin(void);
#endif /* __RECOVER_H__ */
