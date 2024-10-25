/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * grus_loop.c:
 *
 */
#include <autoconf.h>
#include <string.h>
#include <stdio.h>
#include <types.h>
#include <csi_core.h>

#include <driver/gx_audio_in.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_watchdog.h>
#include <driver/gx_delay.h>
#include <driver/gx_clock.h>
#include <driver/gx_irq.h>
#include <driver/gx_snpu.h>
#include <driver/gx_timer.h>
#include <driver/gx_cache.h>
#include <driver/gx_i2s/gx_i2s_v2.h>
#include <driver/gx_rtc.h>

#include <lvp_context.h>
#include <lvp_attr.h>
#include <lvp_buffer.h>
#include <lvp_param.h>
#include <lvp_board.h>

#include "lvp_audio_in.h"
#include "lvp_standby_ratio.h"

#define LOG_TAG "[LVP_AUD]"

//=================================================================================================

typedef struct {
    unsigned int pcm_read_index;
    unsigned int pcm_write_index;
    unsigned int start_ctx_index;
    unsigned int delayed_fft_vad;
    unsigned int last_fft_vad           :4;
    unsigned int standby_startup_flag   :4;
    unsigned int reserve                :24;
} AUDIO_IN_CTRL;
AUDIO_IN_CTRL s_audio_in_ctrl;

static AUDIO_IN_RECORD_CALLBACK s_audio_in_record_callback = NULL;

//=================================================================================================

unsigned int _LvpAudioInGetBufferSize(GX_AUDIO_IN_CHANNEL channel)
{
    switch (channel) {
        case AUDIO_IN_CHANNEL_PCM0:
#ifdef  CONFIG_ENABLE_HARDWARE_FFT
            return LvpGetFftBufferSize();
#elif (defined CONFIG_TYPE_AMIC)
            return ((unsigned int)LvpGetMicBufferSize() / LvpGetMicChannelNum()) * ((CONFIG_TYPE_AMIC_TRACK == 1) ? 1 : 2);
#elif (defined CONFIG_TYPE_I2S_MASTER || defined CONFIG_TYPE_I2S_SLAVE)
            return ((unsigned int)LvpGetMicBufferSize() / LvpGetMicChannelNum()) * ((CONFIG_TYPE_I2S_TRACK == 1) ? 1 : 2);
#endif
            break;
        case AUDIO_IN_CHANNEL_PCM1:
#ifdef CONFIG_TYPE_DMIC
            return ((unsigned int)LvpGetMicBufferSize() / LvpGetMicChannelNum()) * ((CONFIG_TYPE_DMIC_TRACK == 1) ? 1 : 2);
#elif ((defined CONFIG_TYPE_AMIC) && (defined CONFIG_ENABLE_HARDWARE_FFT))
            return ((unsigned int)LvpGetMicBufferSize() / LvpGetMicChannelNum()) * ((CONFIG_TYPE_AMIC_TRACK == 1) ? 1 : 2);
#elif (defined CONFIG_TYPE_I2S_MASTER || defined CONFIG_TYPE_I2S_SLAVE)
            return ((unsigned int)LvpGetMicBufferSize() / LvpGetMicChannelNum()) * ((CONFIG_TYPE_I2S_TRACK == 1) ? 1 : 2);
#endif
            break;
        case AUDIO_IN_CHANNEL_LOGFBANK:
            return (unsigned int)LvpGetLogfbankBufferSize();
            break;
        case AUDIO_IN_CHANNEL_I2SOUT:
            break;
        default:
            break;
    }

    return 0;
}

void *_LvpAudioInGetBufferAddr(GX_AUDIO_IN_CHANNEL channel)
{
    LVP_AUDIO_IN_PARAM_CTRL *boart_ctrl = AudioInBoardGetParamCtrl();
    LVP_AUDIO_IN_CHANNEL output_channel = boart_ctrl->output_channel;
    GX_AUDIO_IN_OUTPUT_PCM *pcm0 = &boart_ctrl->pcm0;

    switch (channel) {
        case AUDIO_IN_CHANNEL_PCM0:
#ifdef CONFIG_ENABLE_HARDWARE_FFT
            return (int *)LvpGetFftBuffer();
#else
            return (unsigned int*)LvpGetMicBufferAddr();
#endif
            break;
        case AUDIO_IN_CHANNEL_PCM1:
            return (unsigned int*)((unsigned char *)LvpGetMicBufferAddr() + \
                                   ((output_channel.pcm0 && pcm0->source != PCM_SOURCE_FSPEC) ? \
                                    _LvpAudioInGetBufferSize(AUDIO_IN_CHANNEL_PCM0) : 0));
            break;
        case AUDIO_IN_CHANNEL_LOGFBANK:
            return (unsigned int*)LvpGetLogfbankBufferAddr();
            break;
        case AUDIO_IN_CHANNEL_I2SOUT:
            break;
        default:
            break;
    }
    return NULL;
}

void _LvpAudioInSetOutput(GX_AUDIO_IN_SOURCE source, GX_AUDIO_IN_CHANNEL channel)
{
    unsigned int *buffer    = _LvpAudioInGetBufferAddr(channel);
    unsigned int size       = _LvpAudioInGetBufferSize(channel);

    LVP_AUDIO_IN_PARAM_CTRL *boart_ctrl = AudioInBoardGetParamCtrl();

    if (channel == AUDIO_IN_CHANNEL_PCM0) {
        unsigned int frame_num = 0;
        unsigned int one_channel_size = 0;
        unsigned int track = boart_ctrl->input_source_config[0].pcm_output_track;

        if ((source & AUDIO_IN_SADC) || (source & AUDIO_IN_FSPEC)) {
            one_channel_size = size/128*128;
        } else {
            if (track == TRACK_MONO)
                one_channel_size = size/128*128;
            else if (track == TRACK_STEREO)
                one_channel_size = size/2/128*128;
        }

        if (source & AUDIO_IN_FSPEC)
            frame_num = 1024; // 256*(32/8)
        else
            frame_num = PCM_FRAME_SIZE/64*64; //one_channel_size/2/(16/8);

        boart_ctrl->pcm0.size = one_channel_size;
        boart_ctrl->pcm0.left_buffer  = (unsigned int)MCU_TO_DEV(((unsigned int)buffer/8*8));

        if (source & AUDIO_IN_FSPEC) {
            boart_ctrl->spectrum.buffer = MCU_TO_DEV(buffer/8*8);
            boart_ctrl->spectrum.size = size;
            boart_ctrl->spectrum.frame_num = size/(32/8)/2;

            gx_audio_in_set_logfbank_enable(1, 1);
            gx_audio_in_set_output_spectrum(channel, boart_ctrl->spectrum);
        } else {
            boart_ctrl->pcm0.right_buffer = (track == TRACK_MONO) ? 0 : (boart_ctrl->pcm0.left_buffer + one_channel_size);
            boart_ctrl->pcm0.endian = 0;
        }

        boart_ctrl->pcm0.frame_num = frame_num;
        gx_audio_in_set_output_pcm(channel, boart_ctrl->pcm0);

    } else if (channel == AUDIO_IN_CHANNEL_PCM1) {
        unsigned int frame_num = 0;
        unsigned int one_channel_size = 0;
        unsigned int track = boart_ctrl->input_source_config[1].pcm_output_track;

        if (source & AUDIO_IN_SADC) {
            one_channel_size = size/128*128;
        } else {
            if (track == TRACK_MONO)
                one_channel_size = size/128*128;
            else if (track == TRACK_STEREO)
                one_channel_size = size/2/128*128;
        }

        frame_num = LvpGetPcmFrameSize()/64*64; //one_channel_size/2/(16/8);

        boart_ctrl->pcm1.size = one_channel_size;
        boart_ctrl->pcm1.left_buffer  = (unsigned int)MCU_TO_DEV(((unsigned int)buffer/8*8));

        boart_ctrl->pcm1.right_buffer = (track == TRACK_MONO) ? 0 : (boart_ctrl->pcm1.left_buffer + one_channel_size);
        boart_ctrl->pcm1.endian = 0;

        boart_ctrl->pcm1.frame_num = frame_num;

        gx_audio_in_set_output_pcm(channel, boart_ctrl->pcm1);

    } else if (channel == AUDIO_IN_CHANNEL_LOGFBANK) {
        boart_ctrl->logfbank.buffer = MCU_TO_DEV(buffer/8*8);
        boart_ctrl->logfbank.size = size;
        boart_ctrl->logfbank.frame_num = 1;

        gx_audio_in_set_logfbank_enable(1, 1);
        gx_audio_in_set_output_logfbank(boart_ctrl->logfbank);
    } else if (channel == AUDIO_IN_CHANNEL_I2SOUT) {
        gx_audio_in_set_output_i2s(boart_ctrl->i2s_out);
#ifdef CONFIG_BOARD_HAS_AIN_I2S_OUT_SLAVE
        gx_i2s_set_single_mode(MODULE_REC_OUT ,I2S_MODE_SLAVE_COMPLETE);
        gx_audio_in_set_i2sout_mode(CLK_MODE_SLAVE);
#else
        gx_audio_in_set_i2sout_mode(CLK_MODE_MASTER);
#endif
    }
}

DRAM0_STAGE2_SRAM_ATTR static int _LvpAudioInRecordDefaultCallback(unsigned int channel, unsigned int *sdc_addr)
{
#ifdef CONFIG_LVP_FFT_VAD_ENABLE
    static int invalid_vad_cnt = 0;
#endif
#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    if (channel & AUDIO_IN_CHANNEL_LOGFBANK) {
        int fft_vad = LvpAudioInQueryFFTVad(&sdc_addr[2]);
        LvpSetAudioInCtrlVad(fft_vad);

        LVP_CONTEXT_HEADER *ctx_header = LvpGetContextHeader();
        int frame_length = ctx_header->logfbank_dim_per_frame;
        int frame_number = ctx_header->pcm_frame_num_per_context;
        int ctx_size     = frame_length * frame_number * sizeof(short);
        int ctx_index    = sdc_addr[2] / ctx_size;
        int ctx_number   = LvpGetLogfbankFrameNumPerChannel() / LvpGetPcmFrameNumPerContext();

        if (s_audio_in_ctrl.standby_startup_flag) {
            s_audio_in_ctrl.standby_startup_flag = 0;
            s_audio_in_ctrl.pcm_write_index      = ctx_index + 2;
            s_audio_in_ctrl.pcm_read_index       = ctx_index + 2;
            s_audio_in_ctrl.start_ctx_index      = ctx_index;
            s_audio_in_ctrl.last_fft_vad         = 1;
            s_audio_in_ctrl.delayed_fft_vad      = 1;
            // printf ("ctx_index:%d, w_index:%d, %d\n", ctx_index, s_audio_in_ctrl.pcm_write_index%ctx_number, s_audio_in_ctrl.pcm_write_index);
# if 0
            // Debug for determine the standby position in the pcm data
            unsigned int  mic_buffer_len_per_context = ctx_header->pcm_frame_num_per_context * \
                                                       ctx_header->frame_length * \
                                                       ctx_header->sample_rate * 2 / 1000;
            unsigned char *cur_mic_buffer = (unsigned char *)ctx_header->mic_buffer + \
                                            mic_buffer_len_per_context * \
                                            (s_audio_in_ctrl.pcm_write_index % \
                                             (ctx_header->mic_buffer_size / ctx_header->mic_num / mic_buffer_len_per_context));

            gx_dcache_invalid_range((unsigned int *)cur_mic_buffer, mic_buffer_len_per_context);
            *(cur_mic_buffer + 1) = 0x80;
            *(cur_mic_buffer + 3) = 0x80;
            gx_dcache_clean_range((unsigned int *)cur_mic_buffer, mic_buffer_len_per_context);
# endif
        }

        while (s_audio_in_ctrl.pcm_write_index % ctx_number != ctx_index) {
            if (s_audio_in_ctrl.pcm_write_index > LvpGetContextNum() // 当write_index大于context数量
                && (s_audio_in_ctrl.pcm_write_index - s_audio_in_ctrl.pcm_read_index >= LvpGetContextNum() - LvpGetContextGap()) // context读写指针间隙小于阈值
                ) {
                break; // 不再上报中断生成新的context
            }

#ifdef CONFIG_LVP_FFT_VAD_ENABLE
            if (fft_vad || s_audio_in_ctrl.pcm_write_index <= (LvpGetLogfbankFrameNumPerChannel() / LvpGetPcmFrameNumPerContext())) {
                s_audio_in_ctrl.delayed_fft_vad = 1;
                invalid_vad_cnt = 0;
            } else {
                invalid_vad_cnt++;
                if (invalid_vad_cnt >= CONFIG_LVP_FFT_INVALID_VAD_NUM) {
                    s_audio_in_ctrl.delayed_fft_vad = 0;
                } else {
                    s_audio_in_ctrl.delayed_fft_vad = 1;
                }
            }
#else
            s_audio_in_ctrl.delayed_fft_vad = 1;
#endif

            // printf ("%d,%d,%d,%d\n", s_audio_in_ctrl.pcm_write_index%ctx_number, ctx_index, s_audio_in_ctrl.pcm_write_index, s_audio_in_ctrl.pcm_read_index);
            s_audio_in_record_callback(s_audio_in_ctrl.pcm_write_index, &sdc_addr[2]);
            s_audio_in_ctrl.pcm_write_index++;


        }
        return 0;
    }
#elif (defined CONFIG_ENABLE_HARDWARE_FFT)

    if (channel & AUDIO_IN_CHANNEL_PCM0) {
        int fft_vad = LvpAudioInQueryFFTVad(&sdc_addr[0]);
        LvpSetAudioInCtrlVad(fft_vad);

        LVP_CONTEXT_HEADER *ctx_header = LvpGetContextHeader();
        int frame_length = ctx_header->fft_dim_per_frame;
        int frame_number = ctx_header->pcm_frame_num_per_context;
        int ctx_size     = frame_length * frame_number * (2 * sizeof(int));
        int ctx_index    = sdc_addr[0] / ctx_size;
        int ctx_number   = LvpGetFftFrameNumPerChannel() / LvpGetPcmFrameNumPerContext();
        if (s_audio_in_ctrl.standby_startup_flag) {
            s_audio_in_ctrl.standby_startup_flag = 0;
            s_audio_in_ctrl.pcm_write_index      = ctx_index + 2;
            s_audio_in_ctrl.pcm_read_index       = ctx_index + 2;
            s_audio_in_ctrl.start_ctx_index      = ctx_index;
            s_audio_in_ctrl.last_fft_vad         = 1;
            s_audio_in_ctrl.delayed_fft_vad      = 1;
            // printf ("ctx_index:%d, w_index:%d, %d\n", ctx_index, s_audio_in_ctrl.pcm_write_index%ctx_number, s_audio_in_ctrl.pcm_write_index);
        }

        while (s_audio_in_ctrl.pcm_write_index % ctx_number != ctx_index) {
            if (s_audio_in_ctrl.pcm_write_index > LvpGetContextNum()
                && s_audio_in_ctrl.pcm_write_index - s_audio_in_ctrl.pcm_read_index >= LvpGetContextNum() - LvpGetContextGap()) {
                break;
            }

#ifdef CONFIG_LVP_FFT_VAD_ENABLE
            if (fft_vad || s_audio_in_ctrl.pcm_write_index <= (LvpGetLogfbankFrameNumPerChannel() / LvpGetPcmFrameNumPerContext())) {
                s_audio_in_ctrl.delayed_fft_vad = 1;
                invalid_vad_cnt = 0;
            } else {
                invalid_vad_cnt++;
                if (invalid_vad_cnt >= CONFIG_LVP_FFT_INVALID_VAD_NUM) {
                    s_audio_in_ctrl.delayed_fft_vad = 0;
                } else {
                    s_audio_in_ctrl.delayed_fft_vad = 1;
                }
            }
#else
            s_audio_in_ctrl.delayed_fft_vad = 1;
#endif

            // printf ("%d,%d,%d,%d\n", s_audio_in_ctrl.pcm_write_index%ctx_number, ctx_index, s_audio_in_ctrl.pcm_write_index, s_audio_in_ctrl.pcm_read_index);
            s_audio_in_record_callback(s_audio_in_ctrl.pcm_write_index, &sdc_addr[0]);
            s_audio_in_ctrl.pcm_write_index++;
        }
        return 0;
    } else if (channel & AUDIO_IN_CHANNEL_PCM1) { // 该case下必然使用fft，这个else不是必要的
        return 0;
        LVP_CONTEXT_HEADER *ctx_header = LvpGetContextHeader();
        int frame_length = ctx_header->fft_dim_per_frame;
        int frame_number = ctx_header->pcm_frame_num_per_context;
        int ctx_size     = frame_length * frame_number * (2 * sizeof(int));
        int ctx_index    = sdc_addr[1] / ctx_size;
        int ctx_number   = LvpGetFftFrameNumPerChannel() / LvpGetPcmFrameNumPerContext();
        if (s_audio_in_ctrl.standby_startup_flag) {
            s_audio_in_ctrl.standby_startup_flag = 0;
            s_audio_in_ctrl.pcm_write_index      = ctx_index + 2;
            s_audio_in_ctrl.pcm_read_index       = ctx_index + 2;
            s_audio_in_ctrl.start_ctx_index      = ctx_index;
            s_audio_in_ctrl.last_fft_vad         = 1;
            // printf ("ctx_index:%d, w_index:%d, %d\n", ctx_index, s_audio_in_ctrl.pcm_write_index%ctx_number, s_audio_in_ctrl.pcm_write_index);
        }

        while (s_audio_in_ctrl.pcm_write_index % ctx_number != ctx_index) {
            if (s_audio_in_ctrl.pcm_write_index > CONFIG_LVP_CONTEXT_NUM
                && s_audio_in_ctrl.pcm_write_index - s_audio_in_ctrl.pcm_read_index >= CONFIG_LVP_CONTEXT_NUM - CONFIG_LVP_CONTEXT_GAP) {
                break;
            }

            // printf ("%d,%d,%d,%d\n", s_audio_in_ctrl.pcm_write_index%ctx_number, ctx_index, s_audio_in_ctrl.pcm_write_index, s_audio_in_ctrl.pcm_read_index);
            s_audio_in_record_callback(s_audio_in_ctrl.pcm_write_index, &sdc_addr[1]);
            s_audio_in_ctrl.pcm_write_index++;
        }
        return 0;
    }
#else

    if (channel & AUDIO_IN_CHANNEL_PCM0) {
        LVP_CONTEXT_HEADER *ctx_header = LvpGetContextHeader();
        int frame_length = PCM_FRAME_SIZE;
        int frame_number = ctx_header->pcm_frame_num_per_context;
        int ctx_size     = frame_length * frame_number * sizeof(short);
        int ctx_index    = sdc_addr[0] / ctx_size;
        int ctx_number   = LvpGetPcmFrameNumPerChannel() / LvpGetPcmFrameNumPerContext();
        if (s_audio_in_ctrl.standby_startup_flag) {
            s_audio_in_ctrl.standby_startup_flag = 0;
            s_audio_in_ctrl.pcm_write_index      = ctx_index + 2;
            s_audio_in_ctrl.pcm_read_index       = ctx_index + 2;
            s_audio_in_ctrl.start_ctx_index      = ctx_index;
            s_audio_in_ctrl.last_fft_vad         = 1;
            // printf ("ctx_index:%d, w_index:%d, %d\n", ctx_index, s_audio_in_ctrl.pcm_write_index%ctx_number, s_audio_in_ctrl.pcm_write_index);
        }

        while (s_audio_in_ctrl.pcm_write_index % ctx_number != ctx_index) {
            if (s_audio_in_ctrl.pcm_write_index > LvpGetContextNum()
                && s_audio_in_ctrl.pcm_write_index - s_audio_in_ctrl.pcm_read_index >= LvpGetContextNum() - LvpGetContextGap()) {
                break;
            }

            // printf ("%d,%d,%d,%d\n", s_audio_in_ctrl.pcm_write_index%ctx_number, ctx_index, s_audio_in_ctrl.pcm_write_index, s_audio_in_ctrl.pcm_read_index);
            s_audio_in_record_callback(s_audio_in_ctrl.pcm_write_index, &sdc_addr[0]);
            s_audio_in_ctrl.pcm_write_index++;
        }
        return 0;
    } else if (channel & AUDIO_IN_CHANNEL_PCM1) {
        LVP_CONTEXT_HEADER *ctx_header = LvpGetContextHeader();
        int frame_length = PCM_FRAME_SIZE;
        int frame_number = ctx_header->pcm_frame_num_per_context;
        int ctx_size     = frame_length * frame_number * sizeof(short);
        int ctx_index    = sdc_addr[1] / ctx_size;
        int ctx_number   = LvpGetPcmFrameNumPerChannel() / LvpGetPcmFrameNumPerContext();
        if (s_audio_in_ctrl.standby_startup_flag) {
            s_audio_in_ctrl.standby_startup_flag = 0;
            s_audio_in_ctrl.pcm_write_index      = ctx_index + 2;
            s_audio_in_ctrl.pcm_read_index       = ctx_index + 2;
            s_audio_in_ctrl.start_ctx_index      = ctx_index;
            s_audio_in_ctrl.last_fft_vad         = 1;
            // printf ("ctx_index:%d, w_index:%d, %d\n", ctx_index, s_audio_in_ctrl.pcm_write_index%ctx_number, s_audio_in_ctrl.pcm_write_index);
        }

        while (s_audio_in_ctrl.pcm_write_index % ctx_number != ctx_index) {
            if (s_audio_in_ctrl.pcm_write_index > ctx_number
                && s_audio_in_ctrl.pcm_write_index - s_audio_in_ctrl.pcm_read_index >= ctx_number - CONFIG_LVP_CONTEXT_GAP) {
                break;
            }

//            printf ("%d,%d,%d,%d\n", s_audio_in_ctrl.pcm_write_index%ctx_number, ctx_index, s_audio_in_ctrl.pcm_write_index, s_audio_in_ctrl.pcm_read_index);
            s_audio_in_record_callback(s_audio_in_ctrl.pcm_write_index, &sdc_addr[1]);
            s_audio_in_ctrl.pcm_write_index++;
        }
        return 0;
    }
#endif

    return 0;
}

int LvpAudioInGetDelayedFFTVad(void)
{
    return s_audio_in_ctrl.delayed_fft_vad;
}

int _LvpAudioInConfigCallback(void)
{
    AudioInBoardInit();

    LVP_AUDIO_IN_PARAM_CTRL *boart_ctrl = AudioInBoardGetParamCtrl();
    GX_AUDIO_IN_SOURCE source = boart_ctrl->input_source;

    if (source & AUDIO_IN_SADC) {

        // set dc 去直流
        gx_audio_in_set_dc_enable(AUDIO_IN_SADC, boart_ctrl->input_source_config[0].dc_enable);

        // set evad
        gx_audio_in_set_evad_threshold(AUDIO_IN_SADC, boart_ctrl->input_source_config[0].evad_threshold);
        gx_audio_in_set_evad_enable(AUDIO_IN_SADC, boart_ctrl->input_source_config[0].evad);

        // set gain
        gx_audio_in_set_input_sadc(boart_ctrl->sadc);

        gx_audio_in_set_pga_gain(boart_ctrl->input_source_config[0].pga_gain);
        gx_audio_in_set_rough_gain(AUDIO_IN_SADC, boart_ctrl->input_source_config[0].audio_in_gain);
    }

    if (source & AUDIO_IN_PDM) {
        // set dc 去直流
        gx_audio_in_set_dc_enable(AUDIO_IN_PDM, boart_ctrl->input_source_config[1].dc_enable);

        // set evad
        gx_audio_in_set_evad_threshold(AUDIO_IN_PDM, boart_ctrl->input_source_config[1].evad_threshold);
        gx_audio_in_set_evad_enable(AUDIO_IN_PDM, boart_ctrl->input_source_config[1].evad);

        // set gain
        gx_audio_in_set_rough_gain(AUDIO_IN_PDM, boart_ctrl->input_source_config[1].audio_in_gain);


        gx_audio_in_set_input_pdm(boart_ctrl->pdm);

#ifndef CONFIG_DMIC_EXCHANGE_CHANNEL
            gx_audio_in_set_input_channel(AUDIO_IN_PDM, 0, 1); // 0 ,1 通道交换
#else
            gx_audio_in_set_input_channel(AUDIO_IN_PDM, 1, 0);
#endif
    }

    if (source & AUDIO_IN_I2S) {
        // set dc 去直流
        gx_audio_in_set_dc_enable(AUDIO_IN_I2S, boart_ctrl->input_source_config[2].dc_enable);

        // set evad
        gx_audio_in_set_evad_threshold(AUDIO_IN_I2S, boart_ctrl->input_source_config[2].evad_threshold);
        gx_audio_in_set_evad_enable(AUDIO_IN_I2S, boart_ctrl->input_source_config[2].evad);

        // set gain
        gx_audio_in_set_rough_gain(AUDIO_IN_I2S, boart_ctrl->input_source_config[2].audio_in_gain);


        gx_audio_in_set_i2s_clock(AUDIO_IN_I2SCLK_12M);
        gx_audio_in_set_input_i2s(boart_ctrl->i2s_in);

#ifndef CONFIG_I2S_EXCHANGE_CHANNEL    // 交换0，1通道
            gx_audio_in_set_input_channel(AUDIO_IN_I2S, 1, 0);
#else
            gx_audio_in_set_input_channel(AUDIO_IN_I2S, 0, 1);
#endif

#ifdef CONFIG_TYPE_I2S_SLAVE
        gx_i2s_set_single_mode(MODULE_I2S_IN ,I2S_MODE_SLAVE_COMPLETE);
        gx_audio_in_set_i2sin_mode(CLK_MODE_SLAVE);
#else
        gx_audio_in_set_i2sin_mode(CLK_MODE_MASTER);
#endif
    }

//    _LvpAudioInSetInput(source);
    LVP_AUDIO_IN_CHANNEL output_channel = boart_ctrl->output_channel;

#if (CONFIG_LVP_PCM_FRAME_NUM_PER_CHANNEL > 0)
    if (output_channel.pcm0) {
        _LvpAudioInSetOutput(source, AUDIO_IN_CHANNEL_PCM0);
    }
    if (output_channel.pcm1) {
        _LvpAudioInSetOutput(source, AUDIO_IN_CHANNEL_PCM1);
    }
#endif

    if (output_channel.logfbank) {
        _LvpAudioInSetOutput(source, AUDIO_IN_CHANNEL_LOGFBANK);
    }
    if (output_channel.i2s) {
        _LvpAudioInSetOutput(source, AUDIO_IN_CHANNEL_I2SOUT);
    }

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    gx_audio_in_set_fftvad_enable(1, 0, LvpGetLogfbankFrameNumPerChannel());
#elif (defined CONFIG_ENABLE_HARDWARE_FFT)
    gx_audio_in_set_fftvad_enable(1, 0, LvpGetFftFrameNumPerChannel());
#endif

//    gx_audio_in_set_data_auto_produce_enable(1);

#if 0
    for (int i = 0xa0a00000; i < 0xa0a00180; i += 4) {
        if (i % 16 == 0)
            printf("\n");
        printf("0x%08x:0x%08x\t", i, *(unsigned int*)i);
    }
#endif

    return 0;
}


void _LvpAudioInUpdateCallback(GX_AUDIO_IN_CHANNEL channel, unsigned int *saved_addr)
{
    if (channel & AUDIO_IN_CHANNEL_LOGFBANK) {
    }

    if (channel & AUDIO_IN_CHANNEL_PCM0) {
    }
}

static int _LvpAudioInGetVadStatus(unsigned int *sdc_addr)
{
    GX_AUDIO_IN_FFTVAD_STATE fftvad_state;
    gx_audio_in_get_fftvad_state (&fftvad_state);
    unsigned int frame_bit = (fftvad_state.vad_state_index + 95 - 1) % 95;

    if (frame_bit < 32) {
        return (fftvad_state.vad_state_l & (1 << frame_bit)) == 0 ? 0 : 1;
    } else if(frame_bit < 64) {
        return (fftvad_state.vad_state_m & (1 << (frame_bit - 32))) == 0 ? 0 : 1;
    } else if(frame_bit < 96) {
        return (fftvad_state.vad_state_h & (1 << (frame_bit - 64))) == 0 ? 0 : 1;
    }

    return -1;
}

int LvpAudioInUpdateReadIndex(int offset)
{
    if (s_audio_in_ctrl.pcm_read_index + offset > s_audio_in_ctrl.pcm_write_index) {
        return -1;
    } else {
        s_audio_in_ctrl.pcm_read_index += offset;
    }
    //printf ("#%d, %d\n", s_audio_in_ctrl.pcm_read_index, s_audio_in_ctrl.pcm_write_index);

    return 0;
}

int LvpGetAudioInCtrlVad(void)
{
    return s_audio_in_ctrl.last_fft_vad;
}

int LvpSetAudioInCtrlVad(int vad)
{
    s_audio_in_ctrl.last_fft_vad = vad;
    return 0;
}

int LvpGetAudioInCtrlStartCtxIndex(void)
{
    return s_audio_in_ctrl.start_ctx_index;
}

#ifdef CONFIG_LVP_ENABLE_ENERGY_VAD
int _LvpAudioInEvadCallback(GX_AUDIO_IN_SOURCE  source)
{
    printf(LOG_TAG"evad cb:%d\n", source);
    return 0;
}
#endif

int LvpAudioInInit(AUDIO_IN_RECORD_CALLBACK callback)
{
    memset (&s_audio_in_ctrl, 0, sizeof(AUDIO_IN_CTRL));
    GX_AUDIO_IN_CONFIG audio_in_config;
    audio_in_config.record_callback = (GX_AUDIO_IN_RECORD_CB)_LvpAudioInRecordDefaultCallback;
    audio_in_config.config_callback = (GX_AUDIO_IN_CONFIG_CB)_LvpAudioInConfigCallback;
    audio_in_config.update_callback = (GX_AUDIO_IN_UPDATE_CB)_LvpAudioInUpdateCallback;
#ifdef CONFIG_LVP_ENABLE_ENERGY_VAD
    audio_in_config.engvad_callback = _LvpAudioInEvadCallback;
#else
    audio_in_config.engvad_callback = NULL;
#endif
    audio_in_config.fftvad_callback = NULL;
#ifdef CONFIG_DMIC_PDM_SLAVE
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_IN_PDM , MODULE_SOURCE_PDM_IN);
#endif
    int ret = gx_audio_in_init(audio_in_config);

    s_audio_in_record_callback = callback;

    return ret;
}

int LvpAudioInDone(void)
{
    gx_audio_in_exit();
    return 0;
}

//-------------------------------------------------------------------------------------------------

int LvpAudioInQueryFFTVad(unsigned int *sdc_addr)
{
    static int vad_param_level = 0;
    unsigned int distance_noise_smooth = gx_audio_in_get_distance_noise_smooth();
#ifdef CONFIG_LVP_ADVANCE_HUMAN_VAD_ENABLE
    if (1) {// (distance_noise_smooth < 3145728 && vad_param_level != 4) { // 3*2^20
#else
    if (distance_noise_smooth < 3145728 && vad_param_level != 4) { // 3*2^20
#endif
        // printf (LOG_TAG"vad param 4 [%d]\n", distance_noise_smooth);
        vad_param_level = 4;
        unsigned short curve_1_a = 1229;
        unsigned short curve_1_b = 3840;
        unsigned short curve_2_a = 1229;
        unsigned short curve_2_b = 1280;
        unsigned short curve_3_a = 1229;
        unsigned short curve_3_b = 4096;
        unsigned short curve_4_a = 2560;
        unsigned short curve_4_b = 1024;
        unsigned short curve_5   = 3072;
        gx_audio_in_set_fftvad_curve_1(curve_1_a, curve_1_b);
        gx_audio_in_set_fftvad_curve_2(curve_2_a, curve_2_b);
        gx_audio_in_set_fftvad_curve_3(curve_3_a, curve_3_b);
        gx_audio_in_set_fftvad_curve_4(curve_4_a, curve_4_b);
        gx_audio_in_set_fftvad_curve_5(curve_5);

        GX_AUDIO_IN_FFTVAD_CHIPPING chipping;
        chipping.chipping_1 = 655;
        chipping.chipping_2 = 262;
        chipping.chipping_3 = 131;
        chipping.chipping_4 = 33;

        gx_audio_in_set_fftvad_chipping(chipping);

        GX_AUDIO_IN_FFTVAD_W w;
        w.w1 = 3;
        w.w2 = 0;
        w.w3 = 0;
        gx_audio_in_set_fftvad_w(w);
    } else if (distance_noise_smooth >= 3145728 && distance_noise_smooth < 6291456 && vad_param_level != 3) { // 6*2^20
        printf (LOG_TAG"vad param 3 [%d]\n", distance_noise_smooth);
        vad_param_level = 3;
        unsigned short curve_1_a = 1229;
        unsigned short curve_1_b = 3840;
        unsigned short curve_2_a = 1229;
        unsigned short curve_2_b = 1280;
        unsigned short curve_3_a = 1229;
        unsigned short curve_3_b = 4096;
        unsigned short curve_4_a = 1843;
        unsigned short curve_4_b = 819;
        unsigned short curve_5   = 2560;
        gx_audio_in_set_fftvad_curve_1(curve_1_a, curve_1_b);
        gx_audio_in_set_fftvad_curve_2(curve_2_a, curve_2_b);
        gx_audio_in_set_fftvad_curve_3(curve_3_a, curve_3_b);
        gx_audio_in_set_fftvad_curve_4(curve_4_a, curve_4_b);
        gx_audio_in_set_fftvad_curve_5(curve_5);

        GX_AUDIO_IN_FFTVAD_CHIPPING chipping;
        chipping.chipping_1 = 655;
        chipping.chipping_2 = 262;
        chipping.chipping_3 = 131;
        chipping.chipping_4 = 33;

        gx_audio_in_set_fftvad_chipping(chipping);

        GX_AUDIO_IN_FFTVAD_W w;
        w.w1 = 1;
        w.w2 = 1;
        w.w3 = 3;
        gx_audio_in_set_fftvad_w(w);
    } else if (distance_noise_smooth <= 20971520 && distance_noise_smooth >= 6291456 && vad_param_level != 2) { // 20*2^20
        printf (LOG_TAG"vad param 2 [%d]\n", distance_noise_smooth);
        vad_param_level = 2;
        unsigned short curve_1_a = 1229;
        unsigned short curve_1_b = 3840;
        unsigned short curve_2_a = 1229;
        unsigned short curve_2_b = 1280;
        unsigned short curve_3_a = 1229;
        unsigned short curve_3_b = 4096;
        unsigned short curve_4_a = 1843;
        unsigned short curve_4_b = 819;
        unsigned short curve_5   = 2560;
        gx_audio_in_set_fftvad_curve_1(curve_1_a, curve_1_b);
        gx_audio_in_set_fftvad_curve_2(curve_2_a, curve_2_b);
        gx_audio_in_set_fftvad_curve_3(curve_3_a, curve_3_b);
        gx_audio_in_set_fftvad_curve_4(curve_4_a, curve_4_b);
        gx_audio_in_set_fftvad_curve_5(curve_5);

        GX_AUDIO_IN_FFTVAD_CHIPPING chipping;
        chipping.chipping_1 = 655;
        chipping.chipping_2 = 262;
        chipping.chipping_3 = 131;
        chipping.chipping_4 = 33;

        gx_audio_in_set_fftvad_chipping(chipping);

        GX_AUDIO_IN_FFTVAD_W w;
        w.w1 = 1;
        w.w2 = 1;
        w.w3 = 10;
        gx_audio_in_set_fftvad_w(w);
    }  else if (distance_noise_smooth > 20971520 && vad_param_level != 1) { // 20*2^20
        printf (LOG_TAG"vad param 1 [%d]\n", distance_noise_smooth);
        vad_param_level = 1;
        unsigned short curve_1_a = 1024;
        unsigned short curve_1_b = 3840;
        unsigned short curve_2_a = 1229;
        unsigned short curve_2_b = 1536;
        unsigned short curve_3_a = 1229;
        unsigned short curve_3_b = 8192;
        unsigned short curve_4_a = 1843;
        unsigned short curve_4_b = 819;
        unsigned short curve_5   = 2560;
        gx_audio_in_set_fftvad_curve_1(curve_1_a, curve_1_b);
        gx_audio_in_set_fftvad_curve_2(curve_2_a, curve_2_b);
        gx_audio_in_set_fftvad_curve_3(curve_3_a, curve_3_b);
        gx_audio_in_set_fftvad_curve_4(curve_4_a, curve_4_b);
        gx_audio_in_set_fftvad_curve_5(curve_5);

        GX_AUDIO_IN_FFTVAD_CHIPPING chipping;
        chipping.chipping_1 = 655;
        chipping.chipping_2 = 262;
        chipping.chipping_3 = 131;
        chipping.chipping_4 = 33;

        gx_audio_in_set_fftvad_chipping(chipping);

        GX_AUDIO_IN_FFTVAD_W w;
        w.w1 = 1;
        w.w2 = 1;
        w.w3 = 10;
        gx_audio_in_set_fftvad_w(w);
    }

    int vad = _LvpAudioInGetVadStatus(sdc_addr);
    if (vad_param_level == 1) {
        vad = 1;
    }

    return vad;
}

//-------------------------------------------------------------------------------------------------
int LvpAuidoInQueryEnvNoise(LVP_CONTEXT *context)
{
#ifdef CONFIG_ENABLE_NOISE_JUDGEMENT
    static int high_noise_maintain_cnt = 0;
    static int mid_noise_maintain_cnt = 0;
    static int low_noise_maintain_cnt = 0;
    static int last_env_noise = ENV_LOW_NOISE;
    unsigned int distance_noise_smooth = gx_audio_in_get_distance_noise_smooth();

    static int standby_ratio = 1000;
    int threshold;
#ifdef CONFIG_ENABLE_CACULATE_STANDBY_RATIO
    threshold = CONFIG_CACULATE_STANDBY_RATIO_THRESHOLD;
    unsigned long time;
    gx_rtc_get_tick(&time);
    if ((context->ctx_index%30 == 0) && (time % CONFIG_CACULATE_STANDBY_RATIO_UPDATE_FREQUENCY == 0)) {
        standby_ratio = LvpCountRealTimeStandbyRatio();
    }
#else
    threshold = 0;
    standby_ratio = threshold;
#endif

    if (distance_noise_smooth > 66060288) {
        high_noise_maintain_cnt ++;
        if (high_noise_maintain_cnt > CONFIG_HIGH_NOISE_MAINTAIN_TIME) {
            mid_noise_maintain_cnt = 0;
            low_noise_maintain_cnt = 0;
            context->env_noise = ENV_HIGH_NOISE;
            last_env_noise     = ENV_HIGH_NOISE;
        } else {
            context->env_noise = last_env_noise;
        }
    } else if (distance_noise_smooth > 6291456 || standby_ratio < threshold){
        mid_noise_maintain_cnt ++;
        if (mid_noise_maintain_cnt > CONFIG_MID_NOISE_MAINTAIN_TIME) {
            high_noise_maintain_cnt = 0;
            low_noise_maintain_cnt  = 0;
            context->env_noise = ENV_MID_NOISE;
            last_env_noise     = ENV_MID_NOISE;
        } else {
            context->env_noise = last_env_noise;
        }
    } else {
        low_noise_maintain_cnt ++;
        if (low_noise_maintain_cnt > CONFIG_LOW_NOISE_MAINTAIN_TIME) {
            high_noise_maintain_cnt = 0;
            low_noise_maintain_cnt  = 0;
            context->env_noise = ENV_LOW_NOISE;
            last_env_noise     = ENV_LOW_NOISE;
        } else {
            context->env_noise = last_env_noise;
        }
    }
#else
    context->env_noise = ENV_LOW_NOISE;
#endif

    return 0;
}

//-------------------------------------------------------------------------------------------------

void LvpAudioInSuspend(void)
{
    gx_audio_in_set_interrupt_enable(AUDIO_IN_IRQ_PCM0_DONE | AUDIO_IN_IRQ_PCM1_DONE | AUDIO_IN_IRQ_LOGFBANK_DONE | AUDIO_IN_IRQ_FFTVAD_FINISH, 0);
#ifdef CONFIG_LVP_ENABLE_ENERGY_VAD
    LVP_AUDIO_IN_PARAM_CTRL *boart_ctrl = AudioInBoardGetParamCtrl();
    // set evad
    gx_audio_in_set_evad_enable(AUDIO_IN_SADC, boart_ctrl->input_source_config[0].evad);
    gx_audio_in_set_interrupt_enable(AUDIO_IN_IRQ_SADC_EVAD_DONE | AUDIO_IN_IRQ_PDM_EVAD_DONE | AUDIO_IN_IRQ_I2S_EVAD_DONE, 1);
#else
    gx_audio_in_set_interrupt_enable(AUDIO_IN_IRQ_FFTVAD_START, 1);
#endif
}

void LvpAudioInResume(void)
{
    // s_audio_in_ctrl.standby_startup_flag = 1;

#ifdef CONFIG_LVP_ENABLE_ENERGY_VAD
    gx_audio_in_set_interrupt_enable(AUDIO_IN_IRQ_PCM0_DONE | AUDIO_IN_IRQ_PCM1_DONE | AUDIO_IN_IRQ_LOGFBANK_DONE | \
                                     AUDIO_IN_IRQ_FFTVAD_START | AUDIO_IN_IRQ_FFTVAD_FINISH | \
                                     AUDIO_IN_IRQ_SADC_EVAD_DONE | AUDIO_IN_IRQ_PDM_EVAD_DONE | AUDIO_IN_IRQ_I2S_EVAD_DONE, 1);
#else
    gx_audio_in_set_interrupt_enable(AUDIO_IN_IRQ_PCM0_DONE | AUDIO_IN_IRQ_PCM1_DONE | AUDIO_IN_IRQ_LOGFBANK_DONE | \
                                     AUDIO_IN_IRQ_FFTVAD_START | AUDIO_IN_IRQ_FFTVAD_FINISH, 1);
#endif
}

void LvpAudioInStandbyToStartup(void)
{
    s_audio_in_ctrl.standby_startup_flag = 1;

#ifdef CONFIG_LVP_ENABLE_ENERGY_VAD
    gx_audio_in_set_interrupt_enable(AUDIO_IN_IRQ_PCM0_DONE | AUDIO_IN_IRQ_PCM1_DONE | AUDIO_IN_IRQ_LOGFBANK_DONE | \
                                     AUDIO_IN_IRQ_FFTVAD_START | AUDIO_IN_IRQ_FFTVAD_FINISH | \
                                     AUDIO_IN_IRQ_SADC_EVAD_DONE | AUDIO_IN_IRQ_PDM_EVAD_DONE | AUDIO_IN_IRQ_I2S_EVAD_DONE, 1);
#else
    gx_audio_in_set_interrupt_enable(AUDIO_IN_IRQ_PCM0_DONE | AUDIO_IN_IRQ_PCM1_DONE | AUDIO_IN_IRQ_LOGFBANK_DONE | \
                                     AUDIO_IN_IRQ_FFTVAD_START | AUDIO_IN_IRQ_FFTVAD_FINISH, 1);
#endif
}
