/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_i2c_msg.c:
 *
 */
#include <autoconf.h>
#include <stdio.h>
#include <lvp_buffer.h>

#include <driver/dsp/csky_math.h>
#include <driver/dsp/csky_const_structs.h>
#include <driver/gx_dcache.h>
#include <driver/gx_timer.h>
#include <driver/gx_irq.h>

#include "lvp_G_vad.h"

#define LOG_TAG "[G_VAD]"

#define G_VAD_LOG 0
#define G_VAD_FFT_LEN 64
#define M_2PI 6.2831853

static float hamming_buff[] = {0.080000, 0.082215, 0.088839, 0.099807, 0.115015, 0.134316, 0.157524, 0.184415, 0.214731, 0.248179, 0.284438, 0.323158, 0.363966, 0.406469, 0.450259, 0.494912, 0.540000, 0.585088, 0.629742, 0.673531, 0.716034, 0.756843, 0.795562, 0.831821, 0.865269, 0.895585, 0.922476, 0.945684, 0.964985, 0.980193, 0.991161, 0.997785, 1.000000, 0.997785, 0.991161, 0.980193, 0.964985, 0.945684, 0.922476, 0.895585, 0.865269, 0.831821, 0.795562, 0.756842, 0.716034, 0.673531, 0.629742, 0.585088, 0.540000, 0.494912, 0.450258, 0.406469, 0.363966, 0.323158, 0.284438, 0.248179, 0.214731, 0.184415, 0.157524, 0.134316, 0.115015, 0.099807, 0.088839, 0.082215, };
unsigned int hamming(unsigned int win, float *buffer) {
    for (int i = 0; i < win; i++) {
        buffer[i] = hamming_buff[i] * buffer[i];
    }
    return 0;
}



#define G_SENSOR_SAMPLE_RATE 1600
static float s_win_buffer[3][G_VAD_FFT_LEN] = {0}; // for one window G-sensor data to float , solo channel
static float s_fft_in_buffer[3][G_VAD_FFT_LEN] = {0}; // data to float , solo channel
static float s_fft_out_buffer[G_VAD_FFT_LEN + 2] = {0}; // for one context G-sensor data to float , solo channel

typedef struct {
    unsigned char   counter[CONFIG_LVP_G_SENSOR_VAD_VOICE_COUNT_WINDOW];
    unsigned char   feature_state[CONFIG_LVP_G_SENSOR_VAD_VOICE_COUNT_WINDOW];
    unsigned char   counter_p;
    unsigned char   past_max_frequency_index;
    unsigned char   cur_max_frequency_index;
    unsigned char   reserve;
} S_VOICEPRINT_STATE;

static S_VOICEPRINT_STATE s_voiceprint_state[3];


static struct {
    G_VAD_CONFIG config;
    float *win_buffer;
    float *fft_in_buffer;
    float *fft_out_buffer;
    unsigned int G_vad_counter;
    unsigned int fft_frame_offset;
    unsigned int fft_frame_offset_next;
    unsigned int G_vad_state;
} s_G_vad_handle;


unsigned int short_2_float(short *short_data, float *float_data, int data_num, int step, int offset)  // 归一化
{
    for (int i = 0; i < data_num; i++) {
        float_data[i] = short_data[i * step + offset] / 32768.0f; // 0x8000
    }

    return 0;
}

int LvpGsensorVadInit(G_VAD_CONFIG *config)
{
    memcpy(&s_G_vad_handle.config, config, sizeof(G_VAD_CONFIG));
    s_G_vad_handle.fft_out_buffer = s_fft_out_buffer;

    return 0;
}

static int _LvpGsensorFFT(short *G_data, int G_size, int G_dim, int G_chose_dim)
{
    S_VOICEPRINT_STATE *vp_state = &s_voiceprint_state[G_chose_dim];
#if G_VAD_LOG
        printf(LOG_TAG"float data:");
        for (int i = 0; i < G_VAD_FFT_LEN; i++) {
            if (i % 16 == 0)
                printf("\n");
            printf("%1.4f,", s_G_vad_handle.fft_in_buffer[i]);
        }
        printf("\n");
#endif

        hamming(64, s_G_vad_handle.fft_in_buffer);

#if G_VAD_LOG
        printf(LOG_TAG"float data:");
        for (int i = 0; i < G_VAD_FFT_LEN; i++) {
            if (i % 16 == 0)
                printf("\n");
            printf("%1.4f,", s_G_vad_handle.fft_in_buffer[i]);
        }
        printf("\n");
#endif

        memset(s_G_vad_handle.fft_out_buffer, 0, G_VAD_FFT_LEN + 2);

        csky_rfft_fast_f32(&csky_rfft_sR_f32_len64, (float32_t *)s_G_vad_handle.fft_in_buffer, (float32_t *)s_G_vad_handle.fft_out_buffer, 0);

#if G_VAD_LOG
        printf(LOG_TAG"fft data:");
        for (int i = 0; i < G_VAD_FFT_LEN + 1; i++) {
            if (i % 8 == 0)
                printf("\n");

            printf("%1.4f + %1.4fi,", s_G_vad_handle.fft_out_buffer[i * 2], s_G_vad_handle.fft_out_buffer[i * 2 + 1]);
        }
        printf("\n");
#endif

        csky_cmplx_mag_f32(s_G_vad_handle.fft_out_buffer, s_G_vad_handle.fft_in_buffer, 33); // 模

#if G_VAD_LOG
        printf(LOG_TAG"fft mag data:");
        for (int i = 0; i < G_VAD_FFT_LEN; i++) {
            if (i % 16 == 0)
                printf("\n");

            printf("%1.4f,", s_G_vad_handle.fft_in_buffer[i]);
        }
        printf("\n");
#endif

        vp_state->cur_max_frequency_index = 0;
        float feature = 0;
        unsigned int once = 1;
        for (int i = 4; i > 0; i--) {
            float tmp = 0;
            int index = -1;
            for (int j = 3; j < 20; j ++) {
                if (s_G_vad_handle.fft_in_buffer[j] > tmp) {
                    tmp = s_G_vad_handle.fft_in_buffer[j];
                    index = j;
                }
            }
#if G_VAD_LOG
            printf(LOG_TAG"%d\t%f\n", index, tmp);
#endif

            if (once) {
//                vp_state->cur_max_frequency_index = index;

                vp_state->cur_max_frequency_index = tmp < 0.02f ? 255 : index;
                once = 0;
            }

            s_G_vad_handle.fft_in_buffer[index] = 0;
            feature += tmp;
            tmp = 0;
        }
        vp_state->cur_max_frequency_index -= 2; // [3,19) => [1, 17) , 找能量最高的频带的序号


        char sub = vp_state->cur_max_frequency_index > vp_state->past_max_frequency_index ? \
                   vp_state->cur_max_frequency_index - vp_state->past_max_frequency_index : \
                   vp_state->past_max_frequency_index - vp_state->cur_max_frequency_index; // 与历史高能频带序号对比，得到变化值

        vp_state->past_max_frequency_index = vp_state->cur_max_frequency_index; // 更新历史记录

        unsigned char past_index = vp_state->counter[vp_state->counter_p];

        vp_state->counter_p = (vp_state->counter_p + 1) % CONFIG_LVP_G_SENSOR_VAD_VOICE_COUNT_WINDOW;
//        if (sub < 2)  // 若变化值小于2，则高能频带没有跃变
        if (sub < 2 && vp_state->past_max_frequency_index < 20)  // 若变化值小于2，则高能频带没有跃变; index 异常，计数归零
            if (vp_state->cur_max_frequency_index == 1) // 若高能频带是第1频带，则可能背景音影响
                if (sub == 0) // 确认是背景音影响，语音帧计数归零
                    vp_state->counter[vp_state->counter_p] = 0;
                else if (past_index > 0) // 可能是背景音影响，语音帧计数减一
                    vp_state->counter[vp_state->counter_p] = past_index - 1;
                else // 语音帧计数为0,，保持不变
                    vp_state->counter[vp_state->counter_p] = past_index;
            else // 高能频带没有跃变且不是第1频带，语音帧计数加一
                vp_state->counter[vp_state->counter_p] = past_index + 1;
        else // 若频带跃变则不是语音信号
            vp_state->counter[vp_state->counter_p] = 0;

        vp_state->feature_state[vp_state->counter_p] = feature > s_G_vad_handle.config.G_vad_threshold;

        unsigned char max_voice_hold_frame_num = 0;
        unsigned char feature_state_counter = 0;
        for (int i = 0; i < s_G_vad_handle.config.G_vad_voice_count_window; i++) {
            max_voice_hold_frame_num = vp_state->counter[i] > max_voice_hold_frame_num ? vp_state->counter[i] : max_voice_hold_frame_num;
            feature_state_counter += vp_state->feature_state[i];
        }


#if 0
        printf(LOG_TAG"%f/%f\n", feature, s_G_vad_handle.config.G_vad_threshold);
#endif
#if 0
        printf("%d ", feature_state_counter);
        int t = s_G_vad_handle.config.G_vad_voice_count_threshold;
        for (int i = 0; i < (max_voice_hold_frame_num > t ? max_voice_hold_frame_num : t); i++) {
            if (i == (t - 1))
                printf("!");
            else if (i == (max_voice_hold_frame_num - 1))
                printf("|");
            else if (i < vp_state->counter[vp_state->counter_p])
                printf(">");
            else
                printf(" ");
        }

        printf(" %d %d\n", vp_state->counter[vp_state->counter_p], max_voice_hold_frame_num);
#endif

        if (feature_state_counter >= s_G_vad_handle.config.G_vad_fft_result_count_threshold
            &&
            max_voice_hold_frame_num >= s_G_vad_handle.config.G_vad_voice_count_threshold) {
            return 1;
        } else {
            return 0;
        }
}

int LvpGsensorVad(short *G_data, int G_size, int G_dim)
{
    int vad = 0;
    if (s_G_vad_handle.config.useful_dim_mask & G_VAD_DIM_1) {
        short *data = G_data;
        int size = G_size;
        int this_dim = 0;
        s_G_vad_handle.fft_frame_offset = s_G_vad_handle.fft_frame_offset_next;
        while (size / (G_dim * sizeof(short)) >= (G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset)) {
#if G_VAD_LOG
            printf(LOG_TAG"G_size = %d\n", size);
            printf(LOG_TAG"G-sensor data:");
            for (int i = 0; i < G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset; i ++) {
                if (i % 16 == 0)
                    printf("\n");

                printf("%d,", ((short *)data)[i * 3]);
            }
            printf("\n");
#endif
            s_G_vad_handle.win_buffer = s_win_buffer[this_dim];


            s_G_vad_handle.fft_in_buffer = s_fft_in_buffer[this_dim];

            float *tmp_buffer = (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset;
            int    data_num   = G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset;
            short_2_float((short *)data, tmp_buffer, data_num, G_dim, this_dim);

#if G_VAD_LOG
            printf(LOG_TAG"G_size = %d\n", size);
            printf(LOG_TAG"win_buffer data:");
            for (int i = 0; i < G_VAD_FFT_LEN; i ++) {
                if (i % 16 == 0)
                    printf("\n");

                printf("%f,", *(s_G_vad_handle.win_buffer + i));
            }
            printf("\n");
#endif
            for (int i = 0; i < G_VAD_FFT_LEN; i++) {
                *(s_G_vad_handle.fft_in_buffer + i) = *(s_G_vad_handle.win_buffer + i);
            }
            gx_dcache_clean_range((unsigned int*)s_G_vad_handle.fft_in_buffer, G_VAD_FFT_LEN);

            if (!vad)
                vad += _LvpGsensorFFT(data, size, G_dim, 0);

            size -= s_G_vad_handle.config.fft_frame_shift * G_dim * sizeof(short);
            data += s_G_vad_handle.config.fft_frame_shift * G_dim;


            s_G_vad_handle.fft_frame_offset = G_VAD_FFT_LEN - s_G_vad_handle.config.fft_frame_shift;
            for(int i = 0;i < s_G_vad_handle.fft_frame_offset; i++) {
                *(s_G_vad_handle.win_buffer + i) = *(s_G_vad_handle.win_buffer + i + s_G_vad_handle.config.fft_frame_shift);
            }

#if 0
            if (vad) {
                break;
            }
#endif
        }

        float *tmp_buffer = (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset;
        int    data_num   = size / (G_dim * sizeof(short));
        short_2_float((short *)G_data, tmp_buffer, data_num, G_dim, 0);
        s_G_vad_handle.fft_frame_offset += size / (G_dim * sizeof(short));
    }
    if (s_G_vad_handle.config.useful_dim_mask & G_VAD_DIM_2) {
        short *data = G_data;
        int size = G_size;
        int this_dim = 1;
        s_G_vad_handle.fft_frame_offset = s_G_vad_handle.fft_frame_offset_next;
        while (size / (G_dim * sizeof(short)) >= (G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset)) {
#if G_VAD_LOG
            printf(LOG_TAG"G_size = %d\n", size);
            printf(LOG_TAG"G-sensor data:");
            for (int i = 0; i < G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset; i ++) {
                if (i % 16 == 0)
                    printf("\n");

                printf("%d,", ((short *)data)[i * 3]);
            }
            printf("\n");
#endif
            s_G_vad_handle.win_buffer = s_win_buffer[this_dim];


            s_G_vad_handle.fft_in_buffer = s_fft_in_buffer[this_dim];

            float *tmp_buffer = (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset;
            int    data_num   = G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset;
            short_2_float((short *)data, tmp_buffer, data_num, G_dim, this_dim);

#if G_VAD_LOG
            printf(LOG_TAG"G_size = %d\n", size);
            printf(LOG_TAG"win_buffer data:");
            for (int i = 0; i < G_VAD_FFT_LEN; i ++) {
                if (i % 16 == 0)
                    printf("\n");

                printf("%f,", *(s_G_vad_handle.win_buffer + i));
            }
            printf("\n");
#endif
            for (int i = 0; i < G_VAD_FFT_LEN; i++) {
                *(s_G_vad_handle.fft_in_buffer + i) = *(s_G_vad_handle.win_buffer + i);
            }
            gx_dcache_clean_range((unsigned int*)s_G_vad_handle.fft_in_buffer, G_VAD_FFT_LEN);

            if (!vad)
                vad += _LvpGsensorFFT(data, size, G_dim, 0);

            size -= s_G_vad_handle.config.fft_frame_shift * G_dim * sizeof(short);
            data += s_G_vad_handle.config.fft_frame_shift * G_dim;


            s_G_vad_handle.fft_frame_offset = G_VAD_FFT_LEN - s_G_vad_handle.config.fft_frame_shift;
            for(int i = 0;i < s_G_vad_handle.fft_frame_offset; i++) {
                *(s_G_vad_handle.win_buffer + i) = *(s_G_vad_handle.win_buffer + i + s_G_vad_handle.config.fft_frame_shift);
            }

#if 0
            if (vad) {
                break;
            }
#endif
        }

        float *tmp_buffer = (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset;
        int    data_num   = size / (G_dim * sizeof(short));
        short_2_float((short *)G_data, tmp_buffer, data_num, G_dim, 0);
        s_G_vad_handle.fft_frame_offset += size / (G_dim * sizeof(short));
    }
    if (s_G_vad_handle.config.useful_dim_mask & G_VAD_DIM_3) {
        short *data = G_data;
        int size = G_size;
        int this_dim = 2;
        s_G_vad_handle.fft_frame_offset = s_G_vad_handle.fft_frame_offset_next;
        while (size / (G_dim * sizeof(short)) >= (G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset)) {
#if G_VAD_LOG
            printf(LOG_TAG"G_size = %d\n", size);
            printf(LOG_TAG"G-sensor data:");
            for (int i = 0; i < G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset; i ++) {
                if (i % 16 == 0)
                    printf("\n");

                printf("%d,", ((short *)data)[i * 3]);
            }
            printf("\n");
#endif
            s_G_vad_handle.win_buffer = s_win_buffer[this_dim];


            s_G_vad_handle.fft_in_buffer = s_fft_in_buffer[this_dim];

            float *tmp_buffer = (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset;
            int    data_num   = G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset;
            short_2_float((short *)data, tmp_buffer, data_num, G_dim, this_dim);

#if G_VAD_LOG
            printf(LOG_TAG"G_size = %d\n", size);
            printf(LOG_TAG"win_buffer data:");
            for (int i = 0; i < G_VAD_FFT_LEN; i ++) {
                if (i % 16 == 0)
                    printf("\n");

                printf("%f,", *(s_G_vad_handle.win_buffer + i));
            }
            printf("\n");
#endif
            for (int i = 0; i < G_VAD_FFT_LEN; i++) {
                *(s_G_vad_handle.fft_in_buffer + i) = *(s_G_vad_handle.win_buffer + i);
            }
            gx_dcache_clean_range((unsigned int*)s_G_vad_handle.fft_in_buffer, G_VAD_FFT_LEN);

            if (!vad)
                vad += _LvpGsensorFFT(data, size, G_dim, 0);

            size -= s_G_vad_handle.config.fft_frame_shift * G_dim * sizeof(short);
            data += s_G_vad_handle.config.fft_frame_shift * G_dim;


            s_G_vad_handle.fft_frame_offset = G_VAD_FFT_LEN - s_G_vad_handle.config.fft_frame_shift;
            for(int i = 0;i < s_G_vad_handle.fft_frame_offset; i++) {
                *(s_G_vad_handle.win_buffer + i) = *(s_G_vad_handle.win_buffer + i + s_G_vad_handle.config.fft_frame_shift);
            }

#if 0
            if (vad) {
                break;
            }
#endif
        }

        float *tmp_buffer = (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset;
        int    data_num   = size / (G_dim * sizeof(short));
        short_2_float((short *)G_data, tmp_buffer, data_num, G_dim, 0);
        s_G_vad_handle.fft_frame_offset += size / (G_dim * sizeof(short));
    }

    if(vad == 0) {
        s_G_vad_handle.G_vad_counter += s_G_vad_handle.G_vad_counter < s_G_vad_handle.config.G_vad_hold_context_counter ? 1 : 0;
        s_G_vad_handle.fft_frame_offset_next = 0;
    }
    s_G_vad_handle.fft_frame_offset_next = s_G_vad_handle.fft_frame_offset;

    s_G_vad_handle.G_vad_state = (vad != 0);
#if 1
    printf(LOG_TAG"v %d\n", s_G_vad_handle.G_vad_state);
#endif
    return s_G_vad_handle.G_vad_counter < s_G_vad_handle.config.G_vad_hold_context_counter;
}

int LvpGetGvad(void)
{
    return s_G_vad_handle.G_vad_state;
}



