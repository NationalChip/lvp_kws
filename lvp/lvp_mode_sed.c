/* Grus
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mode_sed.c:
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
#include <lvp_rnn_sed.h>

#include "lvp_mode.h"
#include "driver/dsp/csky_math.h"
#include "driver/dsp/csky_const_structs.h"
#include "app_core/lvp_app_core.h"

#include "common/lvp_pmu.h"

#include <sed_decode.h>
#include "vma/utinity/hamming.h"
#include "vma/utinity/common_function.h"
#include "vma/utinity/emphasis.h"
#include "vma/utinity/vma_utinity_const_structs.h"

#define LOG_TAG "[LVP_RNN_SED]"

typedef LVP_CONTEXT VPA_CONTEXT;
typedef LVP_CONTEXT_HEADER VPA_CONTEXT_HEADER;

//=================================================================================================
#define RNN_SED_HARDWARE_FFT_LENGTH 256

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

#define RNN_SED_AMP_LENGTH 257
#define RNN_SED_IFFT_INPUT_LENGTH 514
#define RNN_SED_IFFT_OUTPUT_LENGTH 512
#define RNN_SED_WAV_LENGTH PCM_WAV_LENGTH


#define CONFIG_RNN_SED_QUENE_DEEPTH (4)
#define CONFIG_RNN_SED_FFT_TMP_BUFFER_DEEPTH (4)

#ifdef CONFIG_LVP_STANDBY_ENABLE
# ifdef CONFIG_LVP_STATE_FVAD_COUNT_DOWN
#  define LVP_STATE_FVAD_COUNT_DOWN         CONFIG_LVP_STATE_FVAD_COUNT_DOWN
# else
#  define LVP_STATE_FVAD_COUNT_DOWN         (18)
# endif
typedef enum {
    LVP_STANDBY_STATE_WAITING,
    LVP_STANDBY_STATE_AVAD,
    LVP_STANDBY_STATE_FVAD,
    LVP_STANDBY_STATE_WAKEUP,
    LVP_STANDBY_STATE_STANDBY,
}LVP_STANDBY_STATE;

static struct {
    LVP_STANDBY_STATE     state;
    int                 count_down;
} s_standby_state;
DRAM0_STAGE2_SRAM_ATTR void _LvpSetStandbyState(LVP_STANDBY_STATE state)
{
    s_standby_state.state = state;
    switch (state) {
        case LVP_STANDBY_STATE_WAITING:
            break;
        case LVP_STANDBY_STATE_AVAD:
            break;
        case LVP_STANDBY_STATE_FVAD:
            s_standby_state.count_down = LVP_STATE_FVAD_COUNT_DOWN;
            break;
        case LVP_STANDBY_STATE_WAKEUP:
            break;
        default:
            break;
    }
}

DRAM0_STAGE2_SRAM_ATTR void _LvpStandbyStateLoop(void){
    if (s_standby_state.state == LVP_STANDBY_STATE_FVAD) {
        s_standby_state.count_down--;
        if (!s_standby_state.count_down) {
            _LvpSetStandbyState(LVP_STANDBY_STATE_STANDBY);
        }
    }
}
#endif

#if ((!defined(CONFIG_ENABLE_HARDWARE_FFT))&&(!defined(CONFIG_RNN_SED_USE_FIXED_POINT)))
static float32_t fft_input[FFT_INPUT_LENGTH];
static float32_t fft_tmp[FFT_INPUT_LENGTH];
static float32_t fft_output[FFT_OUTPUT_LENGTH];
static float32_t preemph_sample[PCM_WAV_LENGTH];
#endif
static float32_t fft_amp[RNN_SED_AMP_LENGTH * CONFIG_RNN_SED_FFT_TMP_BUFFER_DEEPTH] DRAM0_AUDIO_IN_ATTR;
static float32_t fft_phase[RNN_SED_AMP_LENGTH * CONFIG_RNN_SED_FFT_TMP_BUFFER_DEEPTH][2];



static int s_fft_pack_offset = 0;
static int s_fft_pack_count = 0;
static LVP_CONTEXT *work_context = NULL;
static int s_npu_idle = 1;
static volatile int s_sed_trans_flag = 1;
typedef struct {
    unsigned int module_id;
    void * priv;
} MODULE_INFO;



static unsigned char s_sed_task_queue_buffer[CONFIG_RNN_SED_QUENE_DEEPTH * sizeof(MODULE_INFO)];
static LVP_QUEUE s_sed_task_queue;

static unsigned char s_index_queue_buffer[CONFIG_RNN_SED_QUENE_DEEPTH * sizeof(int *)];
static LVP_QUEUE s_index_queue;

static unsigned char s_work_context_queue_buffer[CONFIG_RNN_SED_QUENE_DEEPTH * sizeof(unsigned int)];
static LVP_QUEUE s_work_context_queue;

static int _LvpAudioInRecordCallback(int ctx_index, void *priv)
{

    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(ctx_index, &context, &ctx_size);
    context->ctx_index = ctx_index;
    context->kws        = 0;
    context->vad        = 0;

    if (context->ctx_index%15 == 0) {
//        printf (LOG_TAG"Ctx:%d, Vad:%d\n", context->ctx_index, context->G_vad);
    }
    LvpQueuePut(&s_index_queue, (unsigned char *)&(context));
    LvpAudioInUpdateReadIndex(1);

        // Get FFTVad
        int vad = LvpAudioInGetDelayedFFTVad();
        static int last_vad = 0;

        if (context->ctx_header->fft_vad_en) {
            LVP_CONTEXT_HEADER *ctx_header = LvpGetContextHeader();
            if (context->ctx_index <= ctx_header->logfbank_frame_num_per_channel / ctx_header->pcm_frame_num_per_context) {
                context->fft_vad = 1;
            } else {
                context->fft_vad = vad;
            }
        } else {
            context->fft_vad = 1;
        }

#ifdef CONFIG_LVP_STANDBY_ENABLE
        _LvpStandbyStateLoop();

        if (context->fft_vad) {
            _LvpSetStandbyState(LVP_STANDBY_STATE_FVAD);
        }
#endif

    APP_EVENT plc_event = {
        .event_id = AUDIO_IN_RECORD_DONE_EVENT_ID,
        .ctx_index = context->ctx_index
    };
    LvpTriggerAppEvent(&plc_event);

    return 0;
}

static int clean_window_flag = 0;
void LvpSedEnable(void)
{
    s_sed_trans_flag = 1;
    clean_window_flag = 1;
}

void LvpSedDisable(void)
{
    s_sed_trans_flag = 0;
    clean_window_flag = 1;
}

int LvpSedStatus(void)
{
    return s_sed_trans_flag;
}

static void _RnnSedModeInitFFTBuffer(void)
{
#if ((!defined(CONFIG_ENABLE_HARDWARE_FFT))&&(!defined(CONFIG_RNN_SED_USE_FIXED_POINT)))
    memset(fft_input, 0, sizeof(fft_input));
    memset(fft_output, 0, sizeof(fft_output));
#endif
}

static void _RnnSedModeTryRunModel(void)
{
    if (s_npu_idle && (s_fft_pack_offset < s_fft_pack_count)) {
        if (LvpQueueGet(&s_work_context_queue, (unsigned char*)&work_context) == 1) {
            int offset =  s_fft_pack_offset % CONFIG_RNN_SED_FFT_TMP_BUFFER_DEEPTH;
            int ret = LvpRnnSedRun(work_context, &fft_amp[offset * RNN_SED_AMP_LENGTH]);
            if (ret == 0) {
#ifdef CONFIG_RNN_SED_WAIT_NPU_CALLBACK
                s_npu_idle = 0;
#else
                s_npu_idle = 1;
#endif
            }
            s_fft_pack_offset += CONFIG_SED_MODEL_INPUT_STRIDE_LENGTH;
        }
    }
}

#define MIC_CONTEXT_SAMPLES_NUM    (LvpGetPcmFrameNumPerContext() * LvpGetPcmFrameSize())
#define MIC_CONTEXT_TOTAL   (LvpGetPcmFrameNumPerChannel() / LvpGetPcmFrameNumPerContext())
#ifndef CONFIG_ENABLE_HARDWARE_FFT
static int _RnnSedModeDoSoftFFT(VPA_CONTEXT *context)
{
    int frame_num = LvpGetPcmFrameNumPerContext();
    unsigned int  samples_per_context = MIC_CONTEXT_SAMPLES_NUM;
    short *cur_mic_buffer = LvpGetMicFrame(context, 0 ,0);// ctx_header->mic_buffer + samples_per_context * (context->ctx_index % MIC_CONTEXT_TOTAL);

    gx_dcache_invalid_range((unsigned int*)cur_mic_buffer, samples_per_context * sizeof(short));
    for (int i = 0 ; i < frame_num; i++) {
        int offset = s_fft_pack_count % CONFIG_RNN_SED_FFT_TMP_BUFFER_DEEPTH;
        for (int m = 0; m < SHIFT_SIZE; m++) {
            preemph_sample[m] =(float)cur_mic_buffer[SHIFT_SIZE * i + m];
        }
        ShiftWindow(preemph_sample, fft_input, WIN_LENGTH, SHIFT_SIZE);
        AddHammingWindowF32(&vma_hamming400_f32, fft_input, fft_tmp);

        for(int j = WIN_LENGTH; j < FFT_INPUT_LENGTH; j++)
            fft_tmp[j] = 0;
        csky_rfft_fast_f32(&csky_rfft_sR_f32_len512, fft_tmp, fft_output, 0);
        CalcAmpPhaseF32(fft_output, &fft_amp[RNN_SED_AMP_LENGTH * offset],
                      &fft_phase[RNN_SED_AMP_LENGTH * offset][0],
                      RNN_SED_AMP_LENGTH, 1);

        ++s_fft_pack_count;

        if ((s_fft_pack_count % CONFIG_SED_MODEL_INPUT_STRIDE_LENGTH == 0)) {
            LvpQueuePut(&s_work_context_queue, (unsigned char *)&context);
        }
        _RnnSedModeTryRunModel();
    }


    return 0;
}
#else
static void _RnnSedGetHardwareFft(LVP_CONTEXT *context)
{
    int *fft_buffer_base_addr = LvpGetFftBuffer();
    int index = context->ctx_index;
    int *cur_fft_buffer = fft_buffer_base_addr + \
                          RNN_SED_HARDWARE_FFT_LENGTH * 2 * \
                         (index * LvpGetPcmFrameNumPerContext() % \
                         LvpGetFftFrameNumPerChannel());
#if 0
    printf("index %d\n", index);

    if (index == 2) {
        LvpAudioInDone();  
        float scale_re = 1.f / (SHRT_MAX + 0.f);
        for (int i = 0; i < 3; i++) {

            int *fft_buffer = fft_buffer_base_addr + \
                              RNN_SED_HARDWARE_FFT_LENGTH * 2 * \
                              (i * LvpGetPcmFrameNumPerContext() % \
                               LvpGetFftFrameNumPerChannel());

            printf("[");
            for (int i = 0; i < 256; i++) {
                //                tr = ((int*)in)[2*i] * scale_re; //(scale + 0.f);
                //                ti = ((int*)in)[2*i+1] * scale_re; // / (scale + 0.f);
                printf("[%f, %f], ", ((int*)fft_buffer)[2*i] * scale_re,
                        ((int*)fft_buffer)[2*i+1] * scale_re );
            }
            printf("],\n");

        }
    }
#endif

# ifdef CONFIG_RNN_SED_CYCLES_DEBUG
    int pre_start = gx_get_time_us();
# endif
    gx_dcache_invalid_range((uint32_t *)cur_fft_buffer, LvpGetPcmFrameNumPerContext() * RNN_SED_HARDWARE_FFT_LENGTH * 2 * sizeof(int));
    for(int i = 0; i < LvpGetPcmFrameNumPerContext(); i++) {
        int offset = s_fft_pack_count % CONFIG_RNN_SED_FFT_TMP_BUFFER_DEEPTH;

        HardFftDeemphasis(cur_fft_buffer + FFT_INPUT_LENGTH * i);
        CalcAmpPhaseF32(cur_fft_buffer + FFT_INPUT_LENGTH * i, &fft_amp[RNN_SED_AMP_LENGTH * offset],
                      &fft_phase[RNN_SED_AMP_LENGTH * offset][0],
                      RNN_SED_AMP_LENGTH, 1); // HardFftDeemphasis中fft已转为float，此处参数用1
        ++s_fft_pack_count;
        if ((s_fft_pack_count % CONFIG_SED_MODEL_INPUT_STRIDE_LENGTH == 0)) {
            LvpQueuePut(&s_work_context_queue, (unsigned char *)&context);
        }
        _RnnSedModeTryRunModel();
    }
# ifdef CONFIG_RNN_SED_CYCLES_DEBUG
    int pre_end = gx_get_time_us();
    printf ("[Preparation] ctx:%d, total:%d us\n", context->ctx_index, pre_end - pre_start);
# endif
}
#endif

static void _RnnSedTick(void)
{
    LVP_CONTEXT *context;
    if (LvpQueueGet(&s_index_queue, (unsigned char *)&context)) {
#ifndef CONFIG_ENABLE_HARDWARE_FFT
        _RnnSedModeDoSoftFFT(context);
#else
        _RnnSedGetHardwareFft(context);
#endif
    }
    _RnnSedModeTryRunModel();

}
//-------------------------------------------------------------------------------------------------
DRAM0_STAGE2_SRAM_ATTR static int _LvpSedSnpuCallback(int module_id, GX_SNPU_STATE state, void *priv)
{
    MODULE_INFO module_info = {
        .module_id = module_id,
        .priv = priv
    };
    LvpQueuePut(&s_sed_task_queue, (unsigned char *)&module_info);
    s_npu_idle = 1;

    return 0;
}

float rec_amp_tmp[RNN_SED_AMP_LENGTH];
static void _DecoderTick(void)
{
    MODULE_INFO module_info = {0};
    if (clean_window_flag) {
        clean_window_flag = 0;
    }

    if (LvpQueueGet(&s_sed_task_queue, (unsigned char *)&module_info)) {
#ifdef CONFIG_RNN_SED_CYCLES_DEBUG
        int re_start = gx_get_time_us();
#endif
        VPA_CONTEXT *context = (VPA_CONTEXT *)(module_info.priv);

        SedDecodeSed(context);

        APP_EVENT event = {
            .event_id = LVP_SED_DONE_ID,
            .ctx_index = context->ctx_index,
        };
        LvpTriggerAppEvent(&event);

        for (int i = 0; i < CONFIG_SED_MODEL_INPUT_STRIDE_LENGTH; i++) {
            _RnnSedModeTryRunModel();
        }
#ifdef CONFIG_RNN_SED_CYCLES_DEBUG
        int re_end = gx_get_time_us();
        printf("[sed_decode] ctx:%d, total: %d us. \n", context->ctx_index, re_end - re_start);
#endif
    }
}

static int _RnnSedModeInit(LVP_MODE_TYPE mode)
{
    // printf(LOG_TAG"RNN Sed Mode Init\n%s\n", LvpSedModelGetVersion());
    _RnnSedModeInitFFTBuffer();
    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    LvpRnnSedInit(_LvpSedSnpuCallback, start_mode);
    LvpQueueInit(&s_sed_task_queue, s_sed_task_queue_buffer,
                CONFIG_RNN_SED_QUENE_DEEPTH * sizeof(MODULE_INFO), sizeof(MODULE_INFO));
    LvpQueueInit(&s_index_queue, s_index_queue_buffer,
            CONFIG_RNN_SED_QUENE_DEEPTH * sizeof(void*), sizeof(void*));
    LvpQueueInit(&s_work_context_queue, s_work_context_queue_buffer,
                CONFIG_RNN_SED_QUENE_DEEPTH * sizeof(int *), sizeof(int *));
   
    if ((mode != LVP_MODE_INIT_FLAG) || (start_mode == GX_WAKEUP_SOURCE_COLD || start_mode == GX_WAKEUP_SOURCE_WDT)) {
        printf("Sample: %d, Context: %d \n", MIC_CONTEXT_SAMPLES_NUM, MIC_CONTEXT_TOTAL);
        if (0 != LvpAudioInInit(_LvpAudioInRecordCallback)) {
            printf(LOG_TAG"LvpAudioInInit Failed\n");
            return -1;
        } else {
            printf(LOG_TAG"LvpAudioInInit Success\n");
        }
    } else {
        LvpAudioInStandbyToStartup();
    }

    SedDecodeInit();

#ifdef CONFIG_LVP_STANDBY_ENABLE
    _LvpSetStandbyState(LVP_STANDBY_STATE_FVAD);
#endif

    return 0;
}

static void _RnnSedModeTick(void)
{
    _RnnSedTick();
    _DecoderTick();

#ifdef CONFIG_LVP_STANDBY_ENABLE
    if ((s_standby_state.state == LVP_STANDBY_STATE_STANDBY) && !LvpPmuSuspendIsLocked()) {
        LvpPmuSuspend(LRT_AUDIO_IN);
    }
#endif
    
}

static void _RnnSedModeDone(LVP_MODE_TYPE next_mode)
{
    SedDecodeDone();
    printf(LOG_TAG"Exit RNN Sed mode\n");
}

void LvpSedGetAudioInBuffer(void **addr, int *len)
{
    if((addr == NULL) || (len == NULL)) {
        return;
    }
    *addr = LvpGetMicBufferAddr();
    *len = LvpGetMicBufferSize() / LvpGetMicChannelNum();
}


static int _RnnSedBufferInit(void)
{
    return LvpInitBuffer();
}
//-------------------------------------------------------------------------------------------------

const LVP_MODE_INFO lvp_sed_mode_info = {
    .type = LVP_MODE_SED,
    .buffer_init = _RnnSedBufferInit,
    .init = _RnnSedModeInit,
    .done = _RnnSedModeDone,
    .tick = _RnnSedModeTick,
};
