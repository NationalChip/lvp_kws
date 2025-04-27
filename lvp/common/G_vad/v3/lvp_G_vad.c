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
#include <lvp_attr.h>
#include <stdlib.h>

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

static float hamming_buff[33] ALIGNED_ATTR(16) =
    {0.080000, 0.082215, 0.088839, 0.099807, 0.115015, 0.134316, 0.157524, 0.184415, \
     0.214731, 0.248179, 0.284438, 0.323158, 0.363966, 0.406469, 0.450259, 0.494912, \
     0.540000, 0.585088, 0.629742, 0.673531, 0.716034, 0.756843, 0.795562, 0.831821, \
     0.865269, 0.895585, 0.922476, 0.945684, 0.964985, 0.980193, 0.991161, 0.997785, \
     1.000000};
//, 0.997785, 0.991161, 0.980193, 0.964985, 0.945684, 0.922476, 0.895585, \
     0.865269, 0.831821, 0.795562, 0.756842, 0.716034, 0.673531, 0.629742, 0.585088, \
     0.540000, 0.494912, 0.450258, 0.406469, 0.363966, 0.323158, 0.284438, 0.248179, \
     0.214731, 0.184415, 0.157524, 0.134316, 0.115015, 0.099807, 0.088839, 0.082215, };
DRAM0_STAGE2_SRAM_ATTR unsigned int hamming(unsigned int win, float *buffer) {
    for (int i = 0; i < win; i++) {
        buffer[i] = hamming_buff[i < 32 ? i : (64 - i) ] * buffer[i];
    }
    return 0;
}

#if 0
static void sum_of_square(float32_t * pSrc, float32_t * pDst, uint32_t num)
{
    for (int i = 0; i < num; i += 2) {
        pDst [i / 2] = pSrc[i] * pSrc[i] + pSrc[i + 1] * pSrc[i + 1];
    }
    return;
}
#endif

#define G_SENSOR_SAMPLE_RATE 1600
#define G_SENSOR_SAMPLE_DIM 3
#define G_SENSOR_SAMPLE_SIZE (G_SENSOR_SAMPLE_DIM * sizeof(short))

#define G_WINDOWS  (CONFIG_LVP_G_SENSOR_VAD_ENERGY_WINDOWS_1 + CONFIG_LVP_G_SENSOR_VAD_ENERGY_WINDOWS_2)

static float s_win_buffer[1][G_VAD_FFT_LEN] ALIGNED_ATTR(16) DRAM0_AUDIO_IN_ATTR = {0}; // for one window G-sensor data to float , solo channel
static float s_fft_in_buffer[G_VAD_FFT_LEN] ALIGNED_ATTR(16) = {0}; // data to float , solo channel
static float s_fft_out_buffer[G_VAD_FFT_LEN + 2] ALIGNED_ATTR(16) = {0}; // for one context G-sensor data to float , solo channel

typedef struct {
    unsigned char   counter[G_WINDOWS];
    unsigned char   counter_p;
    unsigned char   past_max_frequency_index;
    unsigned char   cur_max_frequency_index;
    unsigned char   reserve;
} S_VOICEPRINT_STATE;

static S_VOICEPRINT_STATE s_voiceprint_state[1] DRAM0_AUDIO_IN_ATTR ALIGNED_ATTR(16) ;

static struct {
    G_VAD_CONFIG config;
    float *win_buffer;
    float *fft_in_buffer;
    float *fft_out_buffer;
    unsigned int fft_frame_offset;
    unsigned int fft_frame_offset_next;
    unsigned int G_vad_state;
    unsigned char feature_state[G_WINDOWS];
} s_G_vad_handle ALIGNED_ATTR(16)  DRAM0_AUDIO_IN_ATTR;


DRAM0_STAGE2_SRAM_ATTR unsigned int short_2_float(short *short_data, float *float_data, int data_num, int step, int offset)  // 归一化
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
    s_G_vad_handle.fft_frame_offset_next = G_VAD_FFT_LEN - s_G_vad_handle.config.fft_frame_shift;
    return 0;
}

DRAM0_STAGE2_SRAM_ATTR static int _LvpGsensorFFT(short *G_data, int G_dim, int G_chose_dim, int G_stride_index)
{
//            unsigned int ttt = gx_get_time_us();

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

        hamming(G_VAD_FFT_LEN, s_G_vad_handle.fft_in_buffer);
//            printf("T1 %d\n", gx_get_time_us() - ttt);
//            ttt = gx_get_time_us();

#if G_VAD_LOG
        printf(LOG_TAG"float data:");
        for (int i = 0; i < G_VAD_FFT_LEN; i++) {
            if (i % 16 == 0)
                printf("\n");
            printf("%1.4f,", s_G_vad_handle.fft_in_buffer[i]);
        }
        printf("\n");
#endif

//        memset(s_G_vad_handle.fft_out_buffer, 0, (G_VAD_FFT_LEN + 2) * sizeof(float));
        csky_rfft_fast_f32(&csky_rfft_sR_f32_len64, (float32_t *)s_G_vad_handle.fft_in_buffer, (float32_t *)s_G_vad_handle.fft_out_buffer, 0);
//            printf("T2 %d\n", gx_get_time_us() - ttt);
//            ttt = gx_get_time_us();

#if G_VAD_LOG
        printf(LOG_TAG"fft data:");
        for (int i = 0; i < G_VAD_FFT_LEN + 1; i++) {
            if (i % 8 == 0)
                printf("\n");

            printf("%1.4f + %1.4fi,", s_G_vad_handle.fft_out_buffer[i * 2], s_G_vad_handle.fft_out_buffer[i * 2 + 1]);
        }
        printf("\n");
#endif

//            printf("T2.5 %d\n", gx_get_time_us() - ttt);
//            ttt = gx_get_time_us();
        csky_cmplx_mag_f32(s_G_vad_handle.fft_out_buffer, s_G_vad_handle.fft_in_buffer, 33); // 模
//        sum_of_square(s_G_vad_handle.fft_out_buffer, s_G_vad_handle.fft_in_buffer, 33); // 平方和
//            printf("T3 %d\n", gx_get_time_us() - ttt);
//            ttt = gx_get_time_us();

#if G_VAD_LOG
        printf(LOG_TAG"fft mag data:");
        for (int i = 0; i < G_VAD_FFT_LEN / 2 + 1; i++) {
            if (i % 16 == 0)
                printf("\n");

            printf("%1.4f,", s_G_vad_handle.fft_in_buffer[i]);
        }
        printf("\n");
#endif

        vp_state->cur_max_frequency_index = 0;
        float m1 = 0,m2 = 0,m3 = 0,m4 = 0;
        int index_max = -1;
        for (int j = 3; j < 20; j ++) {
            s_G_vad_handle.fft_in_buffer[j] = s_G_vad_handle.fft_in_buffer[j];
            if (s_G_vad_handle.fft_in_buffer[j] > m1) {
                m4 = m3;
                m3 = m2;
                m2 = m1;
                m1 = s_G_vad_handle.fft_in_buffer[j];
                index_max = j;
            } else if (s_G_vad_handle.fft_in_buffer[j] > m2) {
                m4 = m3;
                m3 = m2;
                m2 = s_G_vad_handle.fft_in_buffer[j];
            } else if (s_G_vad_handle.fft_in_buffer[j] > m3) {
                m4 = m3;
                m3 = s_G_vad_handle.fft_in_buffer[j];
            } else if (s_G_vad_handle.fft_in_buffer[j] > m4) {
                m4 = s_G_vad_handle.fft_in_buffer[j];
            }
        }

#if G_VAD_LOG
        printf(LOG_TAG"%d\t%f\n", index_max, m1 + m2 + m3 + m4);
#endif
//        printf("%f\n", m1 + m2 + m3 + m4);
        vp_state->cur_max_frequency_index = (m1 < CONFIG_LVP_G_SENSOR_BACKGRUND_NOISE_FILTER * 0.001f)  ? 255 : index_max; // 最高能频带能量低于一定值，认为是底噪，覆盖为无效值

//            printf("T4 %d\n", gx_get_time_us() - ttt);
//            ttt = gx_get_time_us();

        vp_state->cur_max_frequency_index -= 2; // [3,19) => [1, 17) , 找能量最高的频带的序号

        char sub = vp_state->cur_max_frequency_index > vp_state->past_max_frequency_index ? \
                   vp_state->cur_max_frequency_index - vp_state->past_max_frequency_index : \
                   vp_state->past_max_frequency_index - vp_state->cur_max_frequency_index; // 与历史高能频带序号对比，得到变化值

        vp_state->past_max_frequency_index = vp_state->cur_max_frequency_index; // 更新历史记录

        unsigned char past_index = vp_state->counter[vp_state->counter_p];

        vp_state->counter_p = (vp_state->counter_p + 1) >= G_WINDOWS \
                              ? ((vp_state->counter_p + 1) - G_WINDOWS) \
                              : (vp_state->counter_p + 1);
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

//            printf("T5 %d\n", gx_get_time_us() - ttt);
//            ttt = gx_get_time_us();

        s_G_vad_handle.feature_state[vp_state->counter_p] = (m1 + m2 + m3 + m4) > s_G_vad_handle.config.G_vad_threshold_1;
        return 0;
}

DRAM0_STAGE2_SRAM_ATTR int LvpGsensorPrepareFFTData(short *G_data, unsigned int G_size, unsigned int frame_offset, unsigned int dim)
{
        unsigned int stride_index = 0;
        short *data = G_data;
        short size = G_size;
        s_G_vad_handle.fft_frame_offset = s_G_vad_handle.fft_frame_offset_next;
        while (size >= (G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset) * G_SENSOR_SAMPLE_SIZE) {
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
            s_G_vad_handle.win_buffer = s_win_buffer[dim];
            s_G_vad_handle.fft_in_buffer = s_fft_in_buffer;
            memset(s_G_vad_handle.fft_in_buffer, 0, G_VAD_FFT_LEN * sizeof(float));

            float *tmp_buffer = (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset;
            int    data_num   = G_VAD_FFT_LEN - s_G_vad_handle.fft_frame_offset;
            short_2_float((short *)data, tmp_buffer, data_num, G_SENSOR_SAMPLE_DIM, dim);

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
//            unsigned int ttt = gx_get_time_us();
            for (int i = 0; i < G_VAD_FFT_LEN; i++) {
                *(s_G_vad_handle.fft_in_buffer + i) = *(s_G_vad_handle.win_buffer + i);
            }
            _LvpGsensorFFT(data, G_SENSOR_SAMPLE_DIM, dim, stride_index);
            stride_index ++;

            size -= data_num * G_SENSOR_SAMPLE_SIZE;
            data += data_num * G_SENSOR_SAMPLE_DIM;

            s_G_vad_handle.fft_frame_offset = G_VAD_FFT_LEN - s_G_vad_handle.config.fft_frame_shift;

            for(int i = 0;i < s_G_vad_handle.fft_frame_offset; i++) {
                *(s_G_vad_handle.win_buffer + i) = *(s_G_vad_handle.win_buffer + i + s_G_vad_handle.config.fft_frame_shift);
            }
        }

        float *tmp_buffer = (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset;
        int    data_num   = size / G_SENSOR_SAMPLE_SIZE ;
        short_2_float((short *)data, tmp_buffer, data_num, G_SENSOR_SAMPLE_DIM, dim);
        s_G_vad_handle.fft_frame_offset += data_num;

    return 0;
}
DRAM0_STAGE2_SRAM_ATTR int LvpGsensorMaintainFFTWin(short *data, unsigned int size, unsigned int frame_offset, unsigned int dim)
{
    s_G_vad_handle.fft_frame_offset = s_G_vad_handle.fft_frame_offset_next;

    S_VOICEPRINT_STATE *vp_state = &s_voiceprint_state[dim];
    s_G_vad_handle.win_buffer = s_win_buffer[dim];

    size = size / G_SENSOR_SAMPLE_SIZE ;  // sample num

    for (int i = 0; \
         i < (size + (s_G_vad_handle.fft_frame_offset % s_G_vad_handle.config.fft_frame_shift)) / s_G_vad_handle.config.fft_frame_shift; \
         i++) {
        vp_state->counter_p = (vp_state->counter_p + 1) >= G_WINDOWS \
                              ? ((vp_state->counter_p + 1) - G_WINDOWS) \
                              : (vp_state->counter_p + 1);
        vp_state->counter[vp_state->counter_p] = 0;
        vp_state->past_max_frequency_index = 255; // 更新历史记录
    }

    if ((size + s_G_vad_handle.fft_frame_offset) < G_VAD_FFT_LEN) {
        short_2_float((short *)data, \
                       (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset\
                      , size, G_SENSOR_SAMPLE_DIM, dim);
        s_G_vad_handle.fft_frame_offset += size;
    } else {
        int a = (s_G_vad_handle.fft_frame_offset + size) % s_G_vad_handle.config.fft_frame_shift;
        if ((G_VAD_FFT_LEN - s_G_vad_handle.config.fft_frame_shift) > (size - a)) {
            int new_offset = (G_VAD_FFT_LEN - s_G_vad_handle.config.fft_frame_shift) - (size - a);
            for(int i = 0;i < new_offset; i++) {
                *(s_G_vad_handle.win_buffer + i) = *(s_G_vad_handle.win_buffer + i + (s_G_vad_handle.fft_frame_offset - new_offset));
            }
            s_G_vad_handle.fft_frame_offset = new_offset;
        } else {
            s_G_vad_handle.fft_frame_offset = 0;
        }

        data += (size - ((G_VAD_FFT_LEN - s_G_vad_handle.config.fft_frame_shift - s_G_vad_handle.fft_frame_offset) + a)) * G_SENSOR_SAMPLE_DIM;
        size = ((G_VAD_FFT_LEN - s_G_vad_handle.config.fft_frame_shift - s_G_vad_handle.fft_frame_offset) + a);
        short_2_float((short *)data, \
                       (float *)s_G_vad_handle.win_buffer + s_G_vad_handle.fft_frame_offset\
                      , size, G_SENSOR_SAMPLE_DIM, dim);
        s_G_vad_handle.fft_frame_offset += size;
    }



//    printf("N %d %d\n", s_G_vad_handle.fft_frame_offset, dim);

    return 0;
}


int LvpGsensorVad(short *G_data, int G_size, int G_dim)
{
    static unsigned int ignore_dim = 0;
    if (s_G_vad_handle.config.useful_dim_mask == 0x111) {
        ignore_dim = (abs(*(G_data + 0)) > abs(*(G_data + 1)) && abs(*(G_data + 0)) > abs(*(G_data + 2))) ? 0 :
                    ((abs(*(G_data + 1)) > abs(*(G_data + 0)) && abs(*(G_data + 1)) > abs(*(G_data + 2))) ? 1 : 2);
    } else {
        ignore_dim = 255;
    }
    if (s_G_vad_handle.config.useful_dim_mask & G_VAD_DIM_1) {
        if (ignore_dim == 0)
            LvpGsensorMaintainFFTWin(G_data, G_size, s_G_vad_handle.fft_frame_offset_next, 0);
        else
            LvpGsensorPrepareFFTData(G_data, G_size, s_G_vad_handle.fft_frame_offset_next, 0);
    }
#if 0
    if (s_G_vad_handle.config.useful_dim_mask & G_VAD_DIM_2) {
        if (ignore_dim == 1)
            LvpGsensorMaintainFFTWin(G_data, G_size, s_G_vad_handle.fft_frame_offset_next, 1);
        else
            LvpGsensorPrepareFFTData(G_data, G_size, s_G_vad_handle.fft_frame_offset_next, 1);
    }
    if (s_G_vad_handle.config.useful_dim_mask & G_VAD_DIM_3) {
        if (ignore_dim == 2)
            LvpGsensorMaintainFFTWin(G_data, G_size, s_G_vad_handle.fft_frame_offset_next, 2);
        else
            LvpGsensorPrepareFFTData(G_data, G_size, s_G_vad_handle.fft_frame_offset_next, 2);
    }
#endif
    s_G_vad_handle.fft_frame_offset_next = s_G_vad_handle.fft_frame_offset;



//    LvpGetGvad();
    return 0;
}

int LvpGetGvad(void)
{
    unsigned int dim = 1;
    s_G_vad_handle.G_vad_state = 0;
    unsigned char flag_1 = 0;
    unsigned char flag_2 = 0;

    unsigned int cur_index = s_voiceprint_state[0].counter_p;

    for (int i = 0; i < s_G_vad_handle.config.G_vad_window_2; i++) {
        int rf2 = 0;
        for (int j = 0; j < s_G_vad_handle.config.G_vad_window_1; j++) {
            int index = (G_WINDOWS + cur_index - i - j) % G_WINDOWS;
            rf2 += s_G_vad_handle.feature_state[index];
//            printf("%d ", s_G_vad_handle.feature_state[index]);
        }
//        printf("||");
//        printf("%d ", rf2);
//        printf("\n");
        if (rf2 > s_G_vad_handle.config.G_vad_threshold_2) {
            flag_1 = 1;
            break;
        }
    }

    for (int d = 0; d < dim; d++) {
        S_VOICEPRINT_STATE *vp_state = &s_voiceprint_state[d];
        unsigned char max_voice_hold_frame_num = 0;
        for (int j = 0; \
             j < s_G_vad_handle.config.G_vad_voice_count_window; \
             j++) {
            max_voice_hold_frame_num = vp_state->counter[j] > max_voice_hold_frame_num ? vp_state->counter[j] : max_voice_hold_frame_num;
        }
        printf("h_num %d\n", max_voice_hold_frame_num);
#if 0
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
        if (max_voice_hold_frame_num >= s_G_vad_handle.config.G_vad_voice_count_threshold) {
            flag_2 = 1;
            if (flag_1 == 1 && flag_2 == 1) {
                s_G_vad_handle.G_vad_state = 1;
                //            printf("r %d\n", gx_get_time_us() - ttt);
#if 1
                printf(LOG_TAG"v %d\n", s_G_vad_handle.G_vad_state);
#endif
                return s_G_vad_handle.G_vad_state;
            }
        }
    }
    return s_G_vad_handle.G_vad_state;
}



