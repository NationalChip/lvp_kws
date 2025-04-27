/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_pmu.h:
 *
 */

#ifndef __LVP_G_VAD_H__
#define __LVP_G_VAD_H__

typedef struct {
    enum{
        G_VAD_DIM_1 = 0x1,
        G_VAD_DIM_2 = 0x2,
        G_VAD_DIM_3 = 0x4,
    } useful_dim_mask;
    float G_vad_threshold_1;
    char G_vad_window_1;
    char G_vad_threshold_2;
    char G_vad_window_2;
    char fft_frame_shift;
    char G_vad_voice_count_window;
    char G_vad_voice_count_threshold;
    char G_vad_fft_result_count_threshold;
    char reserve;
} G_VAD_CONFIG;

int LvpGsensorVadInit( G_VAD_CONFIG *config);

int LvpGsensorVad(short *G_data, int G_size, int G_dim);
int LvpGetGvad(void);

#endif // __LVP_G_VAD_H__

