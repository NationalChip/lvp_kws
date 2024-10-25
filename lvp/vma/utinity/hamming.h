/* Voice Signal Preprocess
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * emphasis.h: emphasis
 *
 */
#ifndef __LVP_VMA_HANMING_H__
#define __LVP_VMA_HANMING_H__

typedef struct
{
    float *hamming_window;
    float *synthesis_window;
    int hamming_win_len;
    int shift_len;
    int rest_in_current_frame;
    int rest_in_last_frame;
    int fft_points;
    int wav_points;
}VMA_HAMMING_FLOAT_STATE;

typedef struct
{
    int *hamming_window;
    int *synthesis_window;
    int hamming_win_len;
    int shift_len;
    int rest_in_current_frame;
    int rest_in_last_frame;
    int fft_points;
    int wav_points;
}VMA_HAMMING_FIXED_STATE;

int ResetDeHanmingLastBufferF32(float *last_buffer, int len);
int AddHammingWindowF32(VMA_HAMMING_FLOAT_STATE *st, float *in, float *out);
int DeHanmingWindowF32L400(VMA_HAMMING_FLOAT_STATE *st, float *in, float *out, float *last_buffer);
int DeHanmingWindowF32L512(float *wav_inbuffer, float *wav_outbuffer);

int ResetDeHanmingLastBufferS16(short *last_fixed, int len);
int AddHammingWindowS16(VMA_HAMMING_FIXED_STATE *st, short *sample, int frame_length);
int DeHammingWindowS16(VMA_HAMMING_FIXED_STATE *st, short *in, short *out, short *last_fixed);
#endif
