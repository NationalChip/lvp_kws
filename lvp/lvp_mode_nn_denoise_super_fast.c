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

#define LOG_TAG "[LVP_NN_DENOISE]"


//=================================================================================================
#define NN_DENOISE_HARDWARE_FFT_LENGTH 256

#ifndef CONFIG_ENABLE_HARDWARE_FFT
#define WIN_LENGTH 400  //25ms
#define SHIFT_SIZE 160      //10ms
#define FRAME_SIZE 480      //30ms
#define HISTORY_LENGTH 240
#endif

#define FFT_INPUT_LENGTH 512
#define FFT_OUTPUT_LENGTH 514
#define PCM_WAV_LENGTH 160

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
#define NN_DENOISE_WAV_LENGTH 160
#endif

#if ((!defined(CONFIG_ENABLE_HARDWARE_FFT))&&(!defined(CONFIG_NN_DENOISE_USE_FIXED_POINT)))
static float32_t fft_input[FFT_INPUT_LENGTH];
static float32_t fft_tmp[FFT_INPUT_LENGTH];
static float32_t fft_output[FFT_OUTPUT_LENGTH];
static float32_t preemph_sample[PCM_WAV_LENGTH];
static short s_last_sample = 0;
#endif
static float32_t fft_amp[NN_DENOISE_AMP_LENGTH * CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH] ;//DRAM0_AUDIO_IN_ATTR;
static float32_t fft_phase[NN_DENOISE_AMP_LENGTH * CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH][2];
static float32_t ifft_input[NN_DENOISE_IFFT_INPUT_LENGTH];// DRAM0_AUDIO_IN_ATTR;
static float32_t ifft_output[NN_DENOISE_IFFT_OUTPUT_LENGTH];// DRAM0_AUDIO_IN_ATTR;
short wav_buffer[NN_DENOISE_WAV_LENGTH * CONFIG_NN_DENOISE_WAV_BUFFER_DEEPTH];
static float last_mask[CONFIG_DENOISE_MODEL_OUTPUT_LENGTH];

static int s_fft_pack_offset = 0;
static int s_fft_pack_count = 0;
static LVP_CONTEXT *work_context = NULL;
static int s_npu_idle = 1;
static volatile int s_denoise_trans_flag = 1;
typedef struct {
    unsigned int module_id;
    void * priv;
} MODULE_INFO;
static int start = 0;
static int last_frame_in_snpu = 0;
static int last_frame_out_snpu = 0;
static LVP_CONTEXT *out_ctx = NULL;

static int fft_phase_offset = CONFIG_DENOISE_MODEL_INPUT_WIN_LENGTH % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
static int wav_offset = 0;

static unsigned char s_denoise_task_queue_buffer[CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(MODULE_INFO)];
static LVP_QUEUE s_denoise_task_queue;

static unsigned char s_index_queue_buffer[CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(unsigned int *)];
static LVP_QUEUE s_index_queue;

static unsigned char s_work_context_queue_buffer[CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(unsigned int *)];
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
          //printf (LOG_TAG"Ctx:%d, Vad:%d\n", context->ctx_index, context->G_vad);
      }
      LvpQueuePut(&s_index_queue, (unsigned char *)&(context->ctx_index));
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
#ifndef CONFIG_NN_DENOISE_SAMPLE_RATE_16K
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
                s_npu_idle = 1;
#else
                s_npu_idle = 0;
#endif
		        last_frame_in_snpu = 1;
            }
            s_fft_pack_offset += CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH;
        }
    }
}

float rec_amp_tmp[NN_DENOISE_AMP_LENGTH];
static void _NnDenoiseGetHardwareFft(LVP_CONTEXT *context)
{
    int *fft_buffer_base_addr = LvpGetFftBuffer();
    int index = context->ctx_index;
    int frames = LvpGetPcmFrameNumPerContext();
    int *cur_fft_buffer = fft_buffer_base_addr + \
                          NN_DENOISE_HARDWARE_FFT_LENGTH * 2 * \
                         (index * LvpGetPcmFrameNumPerContext() % \
                         LvpGetFftFrameNumPerChannel());

    gx_dcache_invalid_range((uint32_t *)cur_fft_buffer, frames * NN_DENOISE_HARDWARE_FFT_LENGTH * 2 * sizeof(int));
    for(int i = 0; i < frames; i++) {
        int offset = s_fft_pack_count % CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
        CalcAmpPhaseF32(cur_fft_buffer + FFT_INPUT_LENGTH * i, &fft_amp[NN_DENOISE_AMP_LENGTH * offset],
                      &fft_phase[NN_DENOISE_AMP_LENGTH * offset][0],
                      NN_DENOISE_AMP_LENGTH, SHRT_MAX);
        ++s_fft_pack_count;
            memcpy(rec_amp_tmp, &fft_amp[fft_phase_offset * NN_DENOISE_AMP_LENGTH], CONFIG_DENOISE_MODEL_OUTPUT_LENGTH * sizeof(float));
			if (s_denoise_trans_flag) {
                RecoverDenoiseMaskWav(rec_amp_tmp,
                                      &fft_phase[fft_phase_offset * NN_DENOISE_AMP_LENGTH][0], \
                                      last_mask, CONFIG_DENOISE_MODEL_OUTPUT_LENGTH, \
                                      ifft_input, ifft_output, &wav_buffer[wav_offset * NN_DENOISE_WAV_LENGTH]);
            } else {
                RecoverWav(&fft_amp[fft_phase_offset * NN_DENOISE_AMP_LENGTH],
                        &fft_phase[fft_phase_offset * NN_DENOISE_AMP_LENGTH][0],
                        ifft_input, ifft_output,
                        &wav_buffer[wav_offset * NN_DENOISE_WAV_LENGTH]);
            }
            start = wav_offset % CONFIG_NN_DENOISE_WAV_BUFFER_DEEPTH;
            ++fft_phase_offset;
            ++wav_offset;
            if (wav_offset >= CONFIG_NN_DENOISE_WAV_BUFFER_DEEPTH)
                wav_offset %= CONFIG_NN_DENOISE_WAV_BUFFER_DEEPTH;
            if (fft_phase_offset >= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH)
                fft_phase_offset %= CONFIG_NN_DENOISE_FFT_TMP_BUFFER_DEEPTH;
			APP_EVENT event = {
            	.event_id = 1 * NN_DENOISE_WAV_LENGTH,
            	.ctx_index = &wav_buffer[(start) * NN_DENOISE_WAV_LENGTH],
        	};
        	LvpTriggerAppEvent(&event);
#ifdef CONFIG_LVP_HAS_UART_RECORD
       	 	UartRecordChannelTask((unsigned char *)&wav_buffer[(start) * NN_DENOISE_WAV_LENGTH],
                              	  1 * NN_DENOISE_WAV_LENGTH * sizeof(short), RECORD_CHANNEL_MIC0);
#endif
            if ((s_fft_pack_count % CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH == 0)) {
                LvpQueuePut(&s_work_context_queue, (unsigned char *)&context);
			    _NnDenoiseModeTryRunModel();
            }
    }
}

static void _NnDenoiseTick(void)
{
    unsigned int index;
    if (LvpQueueGet(&s_index_queue, (unsigned char *)&index)) {
        LVP_CONTEXT *context;
        unsigned int ctx_size;
        LvpGetContext(index, &context, &ctx_size);
        _NnDenoiseGetHardwareFft(context);
    }

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
	last_frame_out_snpu = 1;
	out_ctx = (LVP_CONTEXT*)priv;

    return 0;
}

static void _RecoverTick(void)
{
    MODULE_INFO module_info = {0};
    if (clean_window_flag) {
        ResetDeHanmingWiondowBuffer();
        clean_window_flag = 0;
        memset(wav_buffer, 0, sizeof(wav_buffer));
    }

    if (LvpQueueGet(&s_denoise_task_queue, (unsigned char *)&module_info)) {
        LVP_CONTEXT *context = (LVP_CONTEXT *)(module_info.priv);
        float *model_out = LvpDenoiseModelGetSnpuOutBuffer(context->snpu_buffer);
        int size = LvpDenoiseModelGetOutBufferSize(context->snpu_buffer);
        gx_dcache_invalid_range((uint32_t *)model_out, size);
        memcpy(last_mask, model_out, size);
    }
}

static int _NnDenoiseModeInit(LVP_MODE_TYPE mode)
{
    printf(LOG_TAG"NN Denoise Mode Init (sp)\n");
    _NnDenoiseModeInitFFTBuffer();
    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    LvpNnDenoiseInit(_LvpDenoiseSnpuCallback, start_mode);
    LvpQueueInit(&s_denoise_task_queue, s_denoise_task_queue_buffer,
                CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(MODULE_INFO), sizeof(MODULE_INFO));
    LvpQueueInit(&s_index_queue, s_index_queue_buffer,
                CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(unsigned int *), sizeof(unsigned int *));
    LvpQueueInit(&s_work_context_queue, s_work_context_queue_buffer,
                CONFIG_NN_DENOISE_QUENE_DEEPTH * sizeof(unsigned int *), sizeof(unsigned int *));
    for(int i = 0; i < CONFIG_DENOISE_MODEL_OUTPUT_LENGTH; i++) {
        last_mask[i] = 1;
    }

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

    //printf("addr: 0x%x, len: %d\n", *addr, *len);
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
