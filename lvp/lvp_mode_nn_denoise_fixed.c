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
#include "recover_fixed.h"
#include "common_function.h"
#include "emphasis.h"
#include "hamming.h"
#include "vma_utinity_const_structs.h"

#define LOG_TAG "[LVP_NN_DENOISE_FIXED]"

#define SW_FFT

//=================================================================================================
#define WIN_LENGTH 400  //25ms
#define SHIFT_SIZE 160      //10ms
//#define FRAME_SIZE 480      //30ms
#define HISTORY_LENGTH 240

#define FFT_INPUT_LENGTH 512
#define FFT_OUTPUT_LENGTH 514
#define PCM_WAV_LENGTH 160

#define NN_DENOISE_AMP_LENGTH 257
#define NN_DENOISE_IFFT_INPUT_LENGTH 514
#define NN_DENOISE_IFFT_OUTPUT_LENGTH 512
#define NN_DENOISE_WAV_LENGTH 160

static float32_t fft_amp[NN_DENOISE_AMP_LENGTH * CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH];
static float32_t fft_phase[NN_DENOISE_AMP_LENGTH * CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH][2];
static short ifft_input[NN_DENOISE_IFFT_INPUT_LENGTH];
static short ifft_output[NN_DENOISE_IFFT_OUTPUT_LENGTH];
static short wav_buffer[NN_DENOISE_WAV_LENGTH * 8];

static int s_fft_pack_offset = 0;
static int s_fft_pack_count = 0;
static LVP_CONTEXT *work_context = NULL;
static int s_npu_idle = 1;
static volatile int s_denoise_trans_flag = 0;
typedef struct {
    unsigned int module_id;
    void * priv;
} MODULE_INFO;

static unsigned char s_denoise_task_queue_buffer[CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(MODULE_INFO)];
static LVP_QUEUE s_denoise_task_queue;

static unsigned char s_index_queue_buffer[CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(void*)];
static LVP_QUEUE s_index_queue;

static unsigned char s_work_context_queue_buffer[CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(unsigned int)];
static LVP_QUEUE s_work_context_queue;

#ifdef CONFIG_NN_DENOISE_AUDIOIN_SRC_HW
static int _LvpAudioInRecordCallback(int ctx_index, void *priv)
{
    if (ctx_index > 0) {
        LVP_CONTEXT *context;
        unsigned int ctx_size;
        LvpGetContext(ctx_index - 1, &context, &ctx_size);
        context->ctx_index = ctx_index - 1;
        context->kws        = 0;
        context->vad        = 0;

        if (context->ctx_index%15 == 0) {
            //printf (LOG_TAG"Ctx:%d, Vad:%d\n", context->ctx_index, context->G_vad);
        }
        LvpQueuePut(&s_index_queue, (unsigned char *)&context);
        LvpAudioInUpdateReadIndex(1);
    }

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
    memset(ifft_output, 0, sizeof(ifft_output));
}

static int module_use_fft_offset = 0;
static void _NnDenoiseModeTryRunModel(void)
{
    if (s_npu_idle && (s_fft_pack_offset < s_fft_pack_count)) {
        if (LvpQueueGet(&s_work_context_queue, (unsigned char*)&work_context) == 1) {
            int offset =  s_fft_pack_offset % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
            LvpNnDenoiseRun(work_context, &fft_amp[offset * NN_DENOISE_AMP_LENGTH]);
            if (work_context->ctx_index > PRE_FILL_CONTEXT_NUM) {
#ifdef CONFIG_NN_DENOISE_WAIT_NPU_CALLBACK
                s_npu_idle = 0;
#else
                s_npu_idle = 1;
#endif
                module_use_fft_offset = offset;
            }
            s_fft_pack_offset += CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH;
        }
    }
}

#ifdef SW_FFT
static inline int calc_expand_lshift_q15(short max_abs_in, char reserve_bits)
{
    return 15 - getHighBit(max_abs_in);
}
#define MIC_CONTEXT_SAMPLES_NUM    (LvpGetPcmFrameNumPerContext() * PCM_FRAME_LENGTH * PCM_SAMPLE_RATE / 1000)
#define MIC_CONTEXT_TOTAL   (LvpGetPcmFrameNumPerChannel() / LvpGetPcmFrameNumPerContext())
static void _NnDenoiseModeDoFixedFFT(LVP_CONTEXT *context)
{
    static short mic_window[WIN_LENGTH];
    static short last_sample = 0;
    short max_abs = 0;
    int shift = 0;
    short *window_tmp = (short *)ifft_output;
    short *spectrums = (short *)ifft_input;

    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    int frame_num = LvpGetPcmFrameNumPerContext();
    unsigned int  samples_per_context = MIC_CONTEXT_SAMPLES_NUM;
    short *cur_mic_buffer = ctx_header->mic_buffer + samples_per_context * (context->ctx_index % MIC_CONTEXT_TOTAL);

    gx_dcache_invalid_range((unsigned int*)cur_mic_buffer, samples_per_context * sizeof(short));

    for (int i = 0 ; i < frame_num; i++) {
        int offset = s_fft_pack_count % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
        PreEmphasisS16((cur_mic_buffer + SHIFT_SIZE * i), mic_window + HISTORY_LENGTH, SHIFT_SIZE, &last_sample);

        csky_copy_q15(mic_window, window_tmp, WIN_LENGTH);
        AddHammingWindowS16(&vma_hamming400_s16, window_tmp, WIN_LENGTH);

        csky_abs_max_q15(window_tmp, &max_abs, WIN_LENGTH);
        shift = calc_expand_lshift_q15(max_abs, 0);
        csky_shift_q15(window_tmp, shift, window_tmp, WIN_LENGTH);

        csky_fill_q15(0, window_tmp + WIN_LENGTH, FFT_INPUT_LENGTH - WIN_LENGTH);
        csky_rfft_q15(&csky_rfft_sR_q15_len512, window_tmp, spectrums);

        CalcAmpPhaseFixed(spectrums, &fft_amp[NN_DENOISE_AMP_LENGTH * offset],
                       &fft_phase[NN_DENOISE_AMP_LENGTH * offset][0], NN_DENOISE_AMP_LENGTH, 9 - shift);

        ++s_fft_pack_count;
        if ((s_fft_pack_count % CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH == 0)) {
            LvpQueuePut(&s_work_context_queue, (unsigned char *)&context);
        }
        _NnDenoiseModeTryRunModel();

        csky_copy_q15(mic_window + SHIFT_SIZE, mic_window, HISTORY_LENGTH);
    }
}

#else

#define NN_DENOISE_HARDWARE_FFT_LENGTH 256
static int _CalcAmpPhase(int *in, float32_t *amp, float32_t *phase, int len)
{
    for (int i = 0; i < len - 1; i++) {
        float tr = in[2*i] / 32768.f;
        float ti = in[2*i+1] / 32768.f;
        csky_sqrt_f32((tr * tr + ti * ti), &amp[i]);
        if (amp[i] > 0.f) {
            *(phase + i * 2 + 0) = tr / amp[i];
            *(phase + i * 2 + 1) = ti / amp[i];
        }
        else {
            *(phase + i * 2 + 0) = 0.f;
            *(phase + i * 2 + 1) = 0.f;
        }
    }
#ifdef CONFIG_NN_DENOISE_SAMPLE_RATE_16K
    amp[len-1] = 0.f;
    *(phase + (len-1) * 2 + 0) = 0.f;
    *(phase + (len-1) * 2 + 1) = 0.f;
#endif
    return 0;
}

static void _NnDenoiseGetHardwareFft(LVP_CONTEXT *context)
{
    int *fft_buffer_base_addr = LvpGetFftBuffer();
    int index = context->ctx_index;
    int frame_num = LvpGetPcmFrameNumPerContext();
    int *cur_fft_buffer = fft_buffer_base_addr + LvpGetPcmFrameNumPerContext() * FFT_DIM_PER_FRAME * 2 * (index % LvpGetFftFrameNumPerChannel());
    gx_dcache_invalid_range((uint32_t *)cur_fft_buffer, LvpGetPcmFrameNumPerContext() * FFT_DIM_PER_FRAME * 2 * sizeof(int));

    for(int i = 0; i < frame_num; i++) {
        int offset = s_fft_pack_count % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;

        _CalcAmpPhase(cur_fft_buffer + i * FFT_DIM_PER_FRAME * 2, &fft_amp[NN_DENOISE_AMP_LENGTH * offset],
                &fft_phase[NN_DENOISE_AMP_LENGTH * offset][0],
                NN_DENOISE_AMP_LENGTH);

        ++s_fft_pack_count;
        if ((s_fft_pack_count % CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH == 0)) {
            LvpQueuePut(&s_work_context_queue, (unsigned char *)&context);
        }
        _NnDenoiseModeTryRunModel();
    }
}
#endif

static void _NnDenoiseTick(void)
{
    LVP_CONTEXT *context;
    if (LvpQueueGet(&s_index_queue, (unsigned char *)&context)) {
#ifdef SW_FFT
        _NnDenoiseModeDoFixedFFT(context);
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

static int fft_phase_offset = 0;
static int wav_offset = 0;
static void _RecoverTick(void)
{
    MODULE_INFO module_info = {0};
    if (clean_window_flag) {
        ResetDeHanmingWiondowBuffer();
        clean_window_flag = 0;
        memset(wav_buffer, 0, sizeof(wav_buffer));
    }

    if (LvpQueueGet(&s_denoise_task_queue, (unsigned char *)&module_info)) {
        if (fft_phase_offset >= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH)
            fft_phase_offset %= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
        int start = wav_offset %= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
        for (int i = 0; i < CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH; i++) {
            if (s_denoise_trans_flag) {
                RecoverDenoiseWavFixed((LVP_CONTEXT *)module_info.priv,
                                &fft_amp[fft_phase_offset * NN_DENOISE_AMP_LENGTH],
                                &fft_phase[fft_phase_offset * NN_DENOISE_AMP_LENGTH][0],
                                ifft_input, ifft_output,
                                &wav_buffer[fft_phase_offset * NN_DENOISE_WAV_LENGTH]);
            } else {
                RecoverWavFixed(&fft_amp[fft_phase_offset * NN_DENOISE_AMP_LENGTH],
                        &fft_phase[fft_phase_offset * NN_DENOISE_AMP_LENGTH][0],
                        ifft_input, ifft_output,
                        &wav_buffer[fft_phase_offset * NN_DENOISE_WAV_LENGTH]);
            }

            ++fft_phase_offset;
            ++wav_offset;
            if (wav_offset >= CONFIG_NN_DENOISE_WAV_BUFFER_DEEPTH)
                wav_offset %= CONFIG_NN_DENOISE_WAV_BUFFER_DEEPTH;
            if (fft_phase_offset >= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH)
                fft_phase_offset %= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
            _NnDenoiseModeTryRunModel();
        }

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
    printf(LOG_TAG"NN Denoise Mode Init\n");
    _NnDenoiseModeInitFFTBuffer();
    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    LvpNnDenoiseInit(_LvpDenoiseSnpuCallback, start_mode);
    LvpQueueInit(&s_denoise_task_queue, s_denoise_task_queue_buffer,
            CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(MODULE_INFO), sizeof(MODULE_INFO));
    LvpQueueInit(&s_index_queue, s_index_queue_buffer,
            CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(void*), sizeof(void*));
    LvpQueueInit(&s_work_context_queue, s_work_context_queue_buffer,
            CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(unsigned int), sizeof(unsigned int));

#ifdef CONFIG_NN_DENOISE_AUDIOIN_SRC_HW
    if (0 != LvpAudioInInit(_LvpAudioInRecordCallback)) {
        printf(LOG_TAG"LvpAudioInInit Failed\n");
        return -1;
    }
#endif

#ifdef CONFIG_LVP_HAS_UART_RECORD
    UartRecordInit(-1, 0);
#endif

    return 0;
}

static void _NnDenoiseModeTick(void)
{
    //int t1, t2;
    //t1 = gx_get_time_ms();
    _NnDenoiseTick();
    _RecoverTick();
    //t2 = gx_get_time_ms() - t1;
    //printf("%d\n", t2);
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
    *len = LvpGetMicBufferSize() / CONFIG_MIC_CHANNEL_NUM;

    //printf("addr: 0x%x, len: %d\n", *addr, *len);
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

static int _NnDenoiseModeBufferInit(void)
{
    return LvpInitBuffer();
}
//-------------------------------------------------------------------------------------------------

const LVP_MODE_INFO lvp_nn_denoise_mode_info = {
    .type = LVP_MODE_NN_DENOISE,
    .buffer_init = _NnDenoiseModeBufferInit,
    .init = _NnDenoiseModeInit,
    .done = _NnDenoiseModeDone,
    .tick = _NnDenoiseModeTick,
};
