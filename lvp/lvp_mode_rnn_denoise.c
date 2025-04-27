/* Grus
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mode_nn_denoise.c:
 *
 */
#include <autoconf.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <csi_core.h>
#include <limits.h>

#include <driver/gx_audio_in.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_watchdog.h>
#include <driver/gx_delay.h>
#include <driver/gx_clock.h>
#include <driver/gx_irq.h>
#include <driver/gx_snpu.h>
#include <driver/gx_timer.h>
#include <driver/gx_gpio.h>
#include <driver/gx_dcache.h>

#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_board.h>

#include <lvp_queue.h>
#include <lvp_audio_in.h>
#include <uart_message_v2.h>
#include <lvp_uart_record.h>
#include <lvp_nn_denoise.h>

#include "lvp_mode.h"
#include "driver/dsp/csky_math.h"
#include "driver/dsp/csky_const_structs.h"
#include "app_core/lvp_app_core.h"

#include "recover.h"
#include "hamming.h"
#include "common_function.h"
#include "emphasis.h"
#include "vma_utinity_const_structs.h"
#include "denoise_model.h"

#define LOG_TAG "[LVP_NN_DENOISE]"

typedef LVP_CONTEXT VPA_CONTEXT;
typedef LVP_CONTEXT_HEADER VPA_CONTEXT_HEADER;

//=================================================================================================
#define NN_DENOISE_HARDWARE_FFT_LENGTH 256

#ifndef CONFIG_ENABLE_HARDWARE_FFT
# ifdef CONFIG_PCM_FRAME_LENGTH_10MS
#define WIN_LENGTH 400  //25ms
#define SHIFT_SIZE 160      //10ms
#define FRAME_SIZE 480      //30ms
#define HISTORY_LENGTH 240
# elif defined CONFIG_PCM_FRAME_LENGTH_16MS
#define WIN_LENGTH 512  //32ms
#define SHIFT_SIZE 256      //16ms

# endif
#endif

#define FFT_INPUT_LENGTH 512
#define FFT_OUTPUT_LENGTH 514
#ifdef  CONFIG_PCM_FRAME_LENGTH_10MS
#define PCM_WAV_LENGTH 160
#elif defined CONFIG_PCM_FRAME_LENGTH_16MS
#define PCM_WAV_LENGTH 256
#endif

#ifdef CONFIG_NN_DENOISE_SAMPLE_RATE_8K
#define NN_DENOISE_AMP_LENGTH 257
#define NN_DENOISE_IFFT_INPUT_LENGTH 258
#define NN_DENOISE_IFFT_OUTPUT_LENGTH 256
#define NN_DENOISE_WAV_LENGTH 80
#endif
#ifdef CONFIG_NN_DENOISE_SAMPLE_RATE_16K
#define NN_DENOISE_AMP_LENGTH 257
#define NN_DENOISE_IFFT_INPUT_LENGTH 514
#define NN_DENOISE_IFFT_OUTPUT_LENGTH 512
#define NN_DENOISE_WAV_LENGTH PCM_WAV_LENGTH
#endif

#if ((!defined(CONFIG_ENABLE_HARDWARE_FFT))&&(!defined(CONFIG_NN_DENOISE_USE_FIXED_POINT)))
static float32_t fft_input[FFT_INPUT_LENGTH];
static float32_t fft_tmp[FFT_INPUT_LENGTH];
static float32_t fft_output[FFT_OUTPUT_LENGTH];
static float32_t preemph_sample[PCM_WAV_LENGTH];
#endif
static float32_t fft_amp[NN_DENOISE_AMP_LENGTH * CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH] DRAM0_AUDIO_IN_ATTR;
static float32_t fft_phase[NN_DENOISE_AMP_LENGTH * CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH][2];
static float32_t ifft_input[NN_DENOISE_IFFT_INPUT_LENGTH];// DRAM0_AUDIO_IN_ATTR;
static float32_t ifft_output[NN_DENOISE_IFFT_OUTPUT_LENGTH];// DRAM0_AUDIO_IN_ATTR;
short wav_buffer[NN_DENOISE_WAV_LENGTH * CONFIG_NN_DENOISE_WAV_BUFFER_DEEPTH];

static int s_fft_pack_offset = 0;
static int s_fft_pack_count = 0;
static LVP_CONTEXT *work_context = NULL;
static int s_npu_idle = 1;
static volatile int s_denoise_trans_flag = 1;
typedef struct {
    unsigned int module_id;
    void * priv;
} MODULE_INFO;

static float last_mask[CONFIG_DENOISE_MODEL_OUTPUT_LENGTH];
static unsigned char s_denoise_task_queue_buffer[CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(MODULE_INFO)];
static LVP_QUEUE s_denoise_task_queue;

static unsigned char s_index_queue_buffer[CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(int *)];
static LVP_QUEUE s_index_queue;

static unsigned char s_work_context_queue_buffer[CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(unsigned int)];
static LVP_QUEUE s_work_context_queue;

#ifdef CONFIG_NN_DENOISE_AUDIOIN_SRC_HW
static int _LvpAudioInRecordCallback(int ctx_index, void *priv)
{

    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(ctx_index, &context, &ctx_size);
    context->ctx_index = ctx_index;
    context->kws        = 0;
    context->vad        = 0;

    if (context->ctx_index%15 == 0) {
    //   printf (LOG_TAG"Ctx:%d, Vad:%d\n", context->ctx_index, context->G_vad);
    }
    LvpQueuePut(&s_index_queue, (unsigned char *)&(context));
    LvpAudioInUpdateReadIndex(1);

    return 0;
}
#endif

static int clean_window_flag = 0;
void LvpDenoiseEnable(void)
{
    s_denoise_trans_flag = 1;
    clean_window_flag = 1;
}

void LvpDenoiseDisable(void)
{
    s_denoise_trans_flag = 0;
    clean_window_flag = 1;
}

int LvpDenoiseStatus(void)
{
    return s_denoise_trans_flag;
}

static void _NnDenoiseModeInitFFTBuffer(void)
{
#if ((!defined(CONFIG_ENABLE_HARDWARE_FFT))&&(!defined(CONFIG_NN_DENOISE_USE_FIXED_POINT)))
    memset(fft_input, 0, sizeof(fft_input));
    memset(fft_output, 0, sizeof(fft_output));
#endif
    memset(ifft_output, 0, sizeof(ifft_output));
}


static void _NnDenoiseModeTryRunModel(void)
{
    if (s_npu_idle && (s_fft_pack_offset < s_fft_pack_count)) {
        if (LvpQueueGet(&s_work_context_queue, (unsigned char*)&work_context) == 1) {
            int offset =  s_fft_pack_offset % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
            int ret = LvpNnDenoiseRun(work_context, &fft_amp[offset * NN_DENOISE_AMP_LENGTH]);
            if (ret == 0) {
#ifdef CONFIG_NN_DENOISE_WAIT_NPU_CALLBACK
                s_npu_idle = 0;
#else
                s_npu_idle = 1;
#endif
            }
            s_fft_pack_offset += CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH;
        }
    }
}

#define MIC_CONTEXT_SAMPLES_NUM    (LvpGetPcmFrameNumPerContext() * LvpGetPcmFrameSize())
#define MIC_CONTEXT_TOTAL   (LvpGetPcmFrameNumPerChannel() / LvpGetPcmFrameNumPerContext())
#ifndef CONFIG_ENABLE_HARDWARE_FFT

static int _NnDenoiseModeDoSoftFFT(VPA_CONTEXT *context)
{
    int frame_num = LvpGetPcmFrameNumPerContext();
    unsigned int  samples_per_context = MIC_CONTEXT_SAMPLES_NUM;
    short *cur_mic_buffer = LvpGetMicFrame(context, 0 ,0);// ctx_header->mic_buffer + samples_per_context * (context->ctx_index % MIC_CONTEXT_TOTAL);

    gx_dcache_invalid_range((unsigned int*)cur_mic_buffer, samples_per_context * sizeof(short));
    for (int i = 0 ; i < frame_num; i++) {
        int offset = s_fft_pack_count % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
        for (int m = 0; m < SHIFT_SIZE; m++) {
            preemph_sample[m] =(float)cur_mic_buffer[SHIFT_SIZE * i + m];
        }
# ifdef CONFIG_NN_DENOISE_FFT_CYCLES_DEBUG
    int re_start = gx_get_time_us();
# endif
        ShiftWindow(preemph_sample, fft_input, WIN_LENGTH, SHIFT_SIZE);
        AddHammingWindowF32(&vma_hamming512_f32, fft_input, fft_tmp);

        for(int j = WIN_LENGTH; j < FFT_INPUT_LENGTH; j++)
            fft_tmp[j] = 0;
        csky_rfft_fast_f32(&csky_rfft_sR_f32_len512, fft_tmp, fft_output, 0);
        CalcAmpPhaseF32(fft_output, &fft_amp[NN_DENOISE_AMP_LENGTH * offset],
                      &fft_phase[NN_DENOISE_AMP_LENGTH * offset][0],
                      NN_DENOISE_AMP_LENGTH, 1);

        ++s_fft_pack_count;

        if ((s_fft_pack_count % CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH == 0)) {
            LvpQueuePut(&s_work_context_queue, (unsigned char *)&context);
        }
        _NnDenoiseModeTryRunModel();
    }
# ifdef CONFIG_NN_DENOISE_FFT_CYCLES_DEBUG
        int re_end = gx_get_time_us();
        printf("[SoftFFT] ctx:%d, total: %d us. \n", context->ctx_index, re_end - re_start);
# endif

    return 0;
}
#else
static void _NnDenoiseGetHardwareFft(LVP_CONTEXT *context)
{
    int *fft_buffer_base_addr = LvpGetFftBuffer();
    int index = context->ctx_index;
    int *cur_fft_buffer = fft_buffer_base_addr + \
                          NN_DENOISE_HARDWARE_FFT_LENGTH * 2 * \
                         (index * LvpGetPcmFrameNumPerContext() % \
                         LvpGetFftFrameNumPerChannel());
# ifdef CONFIG_NN_DENOISE_CYCLES_DEBUG
    int pre_start = gx_get_time_us();
# endif
    gx_dcache_invalid_range((uint32_t *)cur_fft_buffer, LvpGetPcmFrameNumPerContext() * NN_DENOISE_HARDWARE_FFT_LENGTH * 2 * sizeof(int));
    for(int i = 0; i < LvpGetPcmFrameNumPerContext(); i++) {
        int offset = s_fft_pack_count % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
        CalcAmpPhaseF32(cur_fft_buffer + FFT_INPUT_LENGTH * i, &fft_amp[NN_DENOISE_AMP_LENGTH * offset],
                      &fft_phase[NN_DENOISE_AMP_LENGTH * offset][0],
                      NN_DENOISE_AMP_LENGTH, SHRT_MAX);
        ++s_fft_pack_count;
        if ((s_fft_pack_count % CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH == 0)) {
            LvpQueuePut(&s_work_context_queue, (unsigned char *)&context);
        }
        _NnDenoiseModeTryRunModel();
    }
# ifdef CONFIG_NN_DENOISE_CYCLES_DEBUG
    int pre_end = gx_get_time_us();
    printf ("[Preparation] ctx:%d, total:%d us\n", context->ctx_index, pre_end - pre_start);
# endif
}
#endif

static void _NnDenoiseTick(void)
{
    LVP_CONTEXT *context;
 if (LvpQueueGet(&s_index_queue, (unsigned char *)&context)) {
#ifndef CONFIG_ENABLE_HARDWARE_FFT
        _NnDenoiseModeDoSoftFFT(context);
#else
        _NnDenoiseGetHardwareFft(context);
#endif
    }
    _NnDenoiseModeTryRunModel();

}
//-------------------------------------------------------------------------------------------------
DRAM0_STAGE2_SRAM_ATTR static int _LvpDenoiseSnpuCallback(int module_id, GX_SNPU_STATE state, void *priv)
{
    MODULE_INFO module_info = {
        .module_id = module_id,
        .priv = priv
    };
    LvpQueuePut(&s_denoise_task_queue, (unsigned char *)&module_info);
    s_npu_idle = 1;

    return 0;
}

#ifdef CONFIG_NN_DENOISE_CYCLES_DEBUG
int reco_time[4] = {0};
#endif

#if CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH == 1
static int fft_phase_offset = CONFIG_DENOISE_MODEL_INPUT_WIN_LENGTH % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH - 1;
#elif CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH == 2
static int fft_phase_offset = CONFIG_DENOISE_MODEL_INPUT_WIN_LENGTH % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
#endif
static int wav_offset = 0;
float rec_amp_tmp[NN_DENOISE_AMP_LENGTH];
static void _RecoverTick(void)
{
    MODULE_INFO module_info = {0};
    if (clean_window_flag) {
        ResetDeHanmingWiondowBuffer();
        clean_window_flag = 0;
        memset(wav_buffer, 0, sizeof(wav_buffer));
    }

    if (LvpQueueGet(&s_denoise_task_queue, (unsigned char *)&module_info)) {
# ifdef CONFIG_NN_DENOISE_CYCLES_DEBUG
        int re_start = gx_get_time_us();
# endif
        VPA_CONTEXT *context = (VPA_CONTEXT *)(module_info.priv);
        float *model_out = (float*)LvpDenoiseModelGetSnpuOutBuffer(context->snpu_buffer);
        int size = LvpDenoiseModelGetOutBufferSize(context->snpu_buffer);
        gx_dcache_invalid_range((uint32_t *)model_out, size);
        // printf("mask output: %f %f %f %f %f %f\n", model_out[0],model_out[1],model_out[2],model_out[3],model_out[4],model_out[5]);
        memcpy(last_mask, model_out, size);
        if (fft_phase_offset >= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH)
            fft_phase_offset %= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;

        int start = wav_offset %= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
        for (int i = 0; i < CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH; i++) {
            if (s_denoise_trans_flag) {
                memcpy(rec_amp_tmp, &fft_amp[fft_phase_offset * NN_DENOISE_AMP_LENGTH], CONFIG_DENOISE_MODEL_OUTPUT_LENGTH * sizeof(float));
                RecoverDenoiseMaskWav(context, rec_amp_tmp,
                                &fft_phase[fft_phase_offset * NN_DENOISE_AMP_LENGTH][0], \
                                last_mask, CONFIG_DENOISE_MODEL_OUTPUT_LENGTH, \
                                ifft_input, ifft_output, &wav_buffer[wav_offset * NN_DENOISE_WAV_LENGTH]);
            } else {
                RecoverWav(&fft_amp[fft_phase_offset * NN_DENOISE_AMP_LENGTH],
                        &fft_phase[fft_phase_offset * NN_DENOISE_AMP_LENGTH][0],
                        ifft_input, ifft_output,
                        &wav_buffer[wav_offset * NN_DENOISE_WAV_LENGTH]);

            }
            ++fft_phase_offset;
            ++wav_offset;
            if (wav_offset >= CONFIG_NN_DENOISE_WAV_BUFFER_DEEPTH)
                wav_offset %= CONFIG_NN_DENOISE_WAV_BUFFER_DEEPTH;
            if (fft_phase_offset >= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH)
                fft_phase_offset %= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
            _NnDenoiseModeTryRunModel();
        }
# ifdef CONFIG_NN_DENOISE_CYCLES_DEBUG
        int re_end = gx_get_time_us();
        printf("[wav_recover] ctx:%d, total: %d us. \n", context->ctx_index, re_end - re_start);
        // printf("(post:%d, ifft:%d, de_ham:%d, de_emph:%d [need x 2])\n", reco_time[0], reco_time[1], reco_time[2], reco_time[3]);
# endif
        APP_EVENT event = {
            .event_id = CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH * NN_DENOISE_WAV_LENGTH,
            .ctx_index = (int)&wav_buffer[start * NN_DENOISE_WAV_LENGTH],
        };
        LvpTriggerAppEvent(&event);
#ifdef CONFIG_LVP_HAS_UART_RECORD
        UartRecordChannelTask((unsigned char *)&wav_buffer[start * NN_DENOISE_WAV_LENGTH],
                              CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH * NN_DENOISE_WAV_LENGTH * sizeof(short), RECORD_CHANNEL_MIC0);
#endif
    }
}

static int _NnDenoiseModeInit(LVP_MODE_TYPE mode)
{
    printf(LOG_TAG"NN Denoise Mode Init\n%s\n", LvpDenoiseModelGetVersion());
    _NnDenoiseModeInitFFTBuffer();
    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    LvpNnDenoiseInit(_LvpDenoiseSnpuCallback, start_mode);
    LvpQueueInit(&s_denoise_task_queue, s_denoise_task_queue_buffer,
                CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(MODULE_INFO), sizeof(MODULE_INFO));
    // LvpQueueInit(&s_index_queue, s_index_queue_buffer,
    //             CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(unsigned int), sizeof(unsigned int));
    LvpQueueInit(&s_index_queue, s_index_queue_buffer,
            CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(void*), sizeof(void*));
    LvpQueueInit(&s_work_context_queue, s_work_context_queue_buffer,
                CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(int *), sizeof(int *));
    printf("Sample: %d, Context: %d \n", MIC_CONTEXT_SAMPLES_NUM, MIC_CONTEXT_TOTAL);


#ifdef CONFIG_NN_DENOISE_AUDIOIN_SRC_HW
    if (0 != LvpAudioInInit(_LvpAudioInRecordCallback)) {
        printf(LOG_TAG"LvpAudioInInit Failed\n");
        return -1;
    }
    else {
        printf(LOG_TAG"LvpAudioInInit Success\n");
    }
#endif

#ifdef CONFIG_LVP_HAS_UART_RECORD
    UartRecordInit(-1, 0);
#endif

#ifndef CONFIG_NN_DENOISE_USE_FIXED_POINT
    RecoverInit();
#endif

    return 0;
}

static void _NnDenoiseModeTick(void)
{
    _NnDenoiseTick();
    _RecoverTick();
#ifdef CONFIG_LVP_HAS_UART_RECORD
    UartRecordTick();
#endif
}

static void _NnDenoiseModeDone(LVP_MODE_TYPE next_mode)
{
    printf(LOG_TAG"Exit NN Denoise mode\n");
}

void LvpDenoiseGetAudioInBuffer(void **addr, int *len)
{
    if((addr == NULL) || (len == NULL)) {
        return;
    }
    *addr = LvpGetMicBufferAddr();
    *len = LvpGetMicBufferSize() / LvpGetMicChannelNum();
}

static unsigned int input_index = 0;
int LvpDenoiseUpdateContext(void)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;

    LvpGetContext(input_index, &context, &ctx_size);
    context->ctx_index = input_index++;
    LvpQueuePut(&s_index_queue, (unsigned char *)&context);

    return 0;
}

static int _NnDenoiseBufferInit(void)
{
    return LvpInitBuffer();
}
//-------------------------------------------------------------------------------------------------

const LVP_MODE_INFO lvp_nn_denoise_mode_info = {
    .type = LVP_MODE_NN_DENOISE,
    .buffer_init = _NnDenoiseBufferInit,
    .init = _NnDenoiseModeInit,
    .done = _NnDenoiseModeDone,
    .tick = _NnDenoiseModeTick,
};
