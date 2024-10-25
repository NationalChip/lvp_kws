/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_buffer.c:
 *
 */
#include <autoconf.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_attr.h>

#include <driver/gx_cache.h>
#include "lvp_buffer_v2.h"
#include <soc_config.h>
#include "autoconf.h"

#define LOG_TAG "[LVP_BUFF]"

//=================================================================================================

// Check some parameters
#if (PCM_FRAME_SIZE * CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CHANNEL * 2) % 128 != 0
#error "Active Channel Buffer Size SHOULD be times of 1024!"
#endif

#if CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CHANNEL % CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CONTEXT != 0
#error "SRAM Frame Number of Channel SHOULD be times of Frame Number of Context!"
#endif

#if CONFIG_LVP_TWS_LOGFBANK_FRAME_NUM_PER_CHANNEL % CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CONTEXT != 0
#error "Logfbank Frame Number of Channel SHOULD be times of Frame Number of Context!"
#endif

//-------------------------------------------------------------------------------------------------

#ifdef CONFIG_LVP_BUFFER_HAS_MIC
// Microphone Input Buffer in SRAM
// static short s_mic_buffer[PCM_FRAME_SIZE * CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CHANNEL * CONFIG_TWS_MIC_CHANNEL_NUM] ALIGNED_ATTR(16) DRAM0_AUDIO_IN_ATTR;
#define LVP_MIC_BUFFER_SIZE  (sizeof(short)*PCM_FRAME_SIZE * CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CHANNEL * CONFIG_TWS_MIC_CHANNEL_NUM)
#else
#define LVP_MIC_BUFFER_SIZE 0
#endif

#ifdef CONFIG_LVP_BUFFER_HAS_REF
// Ref Input Buffer in SRAM
#define LVP_REF_BUFFER_SIZE PCM_FRAME_SIZE * CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CHANNEL * CONFIG_TWS_REF_CHANNEL_NUM
// static short s_ref_buffer[PCM_FRAME_SIZE * CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CHANNEL * CONFIG_TWS_REF_CHANNEL_NUM] ALIGNED_ATTR(16);
#else
#define LVP_REF_BUFFER_SIZE 0
#endif

#ifdef CONFIG_ENABLE_HARDWARE_FFT
// FFT(complex) Input Buffer in SRAM
// static short s_fft_buffer[FFT_DIM_PER_FRAME * 2 * CONFIG_LVP_TWS_FFT_FRAME_NUM_PER_CHANNEL * CONFIG_TWS_FFT_CHANNEL_NUM] ALIGNED_ATTR(16);
#define LVP_FFT_BUFFER_SIZE FFT_DIM_PER_FRAME * 2 * CONFIG_LVP_TWS_FFT_FRAME_NUM_PER_CHANNEL * CONFIG_TWS_FFT_CHANNEL_NUM
#else
#define LVP_FFT_BUFFER_SIZE 0
#endif

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
# ifdef CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION
// Logfbank Input Buffer in SRAM
static short s_logfbank_buffer[LOGFBANK_DIM_PER_FRAME * CONFIG_LVP_TWS_LOGFBANK_FRAME_NUM_PER_CHANNEL * CONFIG_TWS_LOGFBANK_CHANNEL_NUM] ALIGNED_ATTR(16);
# define LVP_LOGFBANK_BUFFER_SIZE 0
# endif
#define LVP_LOGFBANK_BUFFER_SIZE (sizeof(short)*LOGFBANK_DIM_PER_FRAME * CONFIG_LVP_TWS_LOGFBANK_FRAME_NUM_PER_CHANNEL * CONFIG_TWS_LOGFBANK_CHANNEL_NUM)
#else
# define LVP_LOGFBANK_BUFFER_SIZE 0
#endif

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
// TODO
// static short s_feats_buffer[CONFIG_KWS_MODEL_INPUT_WIN_LENGTH * CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME] ALIGNED_ATTR(16);
#define LVP_FEATS_BUFFER_SIZE CONFIG_KWS_MODEL_INPUT_WIN_LENGTH * CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME
static unsigned char *s_feats_buffer;
#else
#define LVP_FEATS_BUFFER_SIZE 0
#endif

#ifdef CONFIG_LVP_TWS_HAS_OUT_CHANNEL
# ifdef CONFIG_LVP_TWS_OUT_CHANNEL_CUSTOMIZE
# define _OUT_FRAME_NUM_PER_CHANNEL_ CONFIG_TWS_LVP_OUT_FRAME_NUM_PER_CHANNEL
# define  LVP_OUT_BUFFER_SIZE CONFIG_TWS_OUT_CHANNEL_NUM * * CONFIG_LVP_TWS_OUT_FRAME_SIZE * _OUT_FRAME_NUM_PER_CHANNEL_ * sizeof(short)
# else
# define _OUT_FRAME_NUM_PER_CHANNEL_ (CONFIG_LVP_CONTEXT_NUM * CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CONTEXT)
# define  LVP_OUT_BUFFER_SIZE PCM_FRAME_SIZE * CONFIG_TWS_OUT_CHANNEL_NUM * _OUT_FRAME_NUM_PER_CHANNEL_ * sizeof(short)
# endif
#else
# define LVP_OUT_BUFFER_SIZE 0
#endif

static void *LvpTwsGetAudioOutBuffer(void)
{
#if CONFIG_AUDIO_OUT_BUFFER_SIZE_KB
    return (void *)s_audio_out_buffer;
#else
    return NULL;
#endif
}

static int LvpTwsGetAudioOutBufferSize(void)
{
#if CONFIG_AUDIO_OUT_BUFFER_SIZE_KB
    return CONFIG_AUDIO_OUT_BUFFER_SIZE_KB*1024;
#else
    return 0;
#endif
}

static int LvpTwsGetContextGap(void)
{
    return CONFIG_LVP_CONTEXT_GAP;
}

int LvpTwsGetOutBufferSize(void)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(0, &context, &ctx_size);

    LVP_CONTEXT_HEADER *ctx_header      = context->ctx_header;
    unsigned int out_num                = ctx_header->out_num;
#ifdef CONFIG_LVP_TWS_OUT_CHANNEL_CUSTOMIZE
    unsigned int out_size_per_channel   = out_num * CONFIG_LVP_TWS_OUT_FRAME_SIZE * _OUT_FRAME_NUM_PER_CHANNEL_;
#else
    unsigned int out_frame_size         = PCM_FRAME_SIZE* sizeof(short); ///
    unsigned int out_frame_nun_channel  = ctx_header->out_frame_num_per_channel;
    unsigned int out_size_per_channel   = out_num * out_frame_size * out_frame_nun_channel;
#endif
    return out_size_per_channel;
}

int LvpTwsGetOutBufferSizePerContext(void)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(0, &context, &ctx_size);

    LVP_CONTEXT_HEADER *ctx_header      = context->ctx_header;
    unsigned int out_num                = ctx_header->out_num;
#ifdef CONFIG_LVP_TWS_OUT_CHANNEL_CUSTOMIZE
    unsigned int out_size_per_context   = out_num * CONFIG_LVP_TWS_OUT_FRAME_SIZE;
#else
    unsigned int out_frame_size         = PCM_FRAME_SIZE* sizeof(short); ///
    unsigned int out_frame_nun_context  = ctx_header->pcm_frame_num_per_context;
    unsigned int out_size_per_context   = out_num * out_frame_size * out_frame_nun_context;
#endif
    return out_size_per_context;
}

void *LvpTwsGetOutBuffer(unsigned int index)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(0, &context, &ctx_size);

    LVP_CONTEXT_HEADER *ctx_header      = context->ctx_header;
    unsigned int out_num                = ctx_header->out_num;
    if(!out_num)
        return NULL;
#ifdef CONFIG_LVP_TWS_OUT_CHANNEL_CUSTOMIZE
    unsigned int out_size_per_context   = out_num * CONFIG_LVP_TWS_OUT_FRAME_SIZE;
    unsigned int out_size_per_channel   = out_num * CONFIG_LVP_TWS_OUT_FRAME_SIZE * _OUT_FRAME_NUM_PER_CHANNEL_;
#else
    unsigned int out_frame_size         = PCM_FRAME_SIZE* sizeof(short); ///
    unsigned int out_frame_nun_channel  = ctx_header->out_frame_num_per_channel;
    unsigned int out_frame_nun_context  = ctx_header->pcm_frame_num_per_context;
    unsigned int out_size_per_context   = out_num * out_frame_size * out_frame_nun_context;
    unsigned int out_size_per_channel   = out_num * out_frame_size * out_frame_nun_channel;
#endif

    return (void *)((unsigned int)s_context_header.out_buffer + index % (out_size_per_channel / out_size_per_context) * out_size_per_context);
}

int LvpInitTwsBuffer(void)
{
    //printf (LOG_TAG"Enter LvpInitBuffer\n");
    // printf("lvp_phase1_buf addr 0x%x\n", lvp_phase1_buf);
    printf("lvp_phase2_buf addr 0x%x\n", lvp_phase2_buf);
    printf("lvp_phase3_buf addr 0x%x\n", lvp_phase3_buf);
    printf("s_audio_out_buffer addr 0x%x\n", s_audio_out_buffer);

    // Clear
    memset(&s_context_header, 0, sizeof(s_context_header));
    memset(&s_context_buffer, 0, sizeof(s_context_buffer));

    // Initialize Context Header
    s_context_header.version = LVP_CONTEXT_VERSION;

/**********************************phase3 buf 16k*****************************************/
#ifdef CONFIG_LVP_BUFFER_HAS_MIC
    s_context_header.mic_num = CONFIG_TWS_MIC_CHANNEL_NUM;
    s_context_header.mic_buffer = (short *)lvp_phase3_buf;
    s_context_header.mic_buffer_size = LVP_MIC_BUFFER_SIZE;
#else
    s_context_header.mic_num = 0;
    s_context_header.mic_buffer = NULL;
    s_context_header.mic_buffer_size = 0;
#endif


#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    s_context_header.logfbank_num = CONFIG_TWS_LOGFBANK_CHANNEL_NUM;
# ifdef CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION
    s_context_header.logfbank_buffer = s_logfbank_buffer;
    s_context_header.logfbank_buffer_size = sizeof(s_logfbank_buffer);
# else
    s_context_header.logfbank_buffer =(short *)(lvp_phase3_buf + LVP_MIC_BUFFER_SIZE);
    s_context_header.logfbank_buffer_size = LVP_LOGFBANK_BUFFER_SIZE;
# endif
    s_context_header.logfbank_frame_num_per_channel = CONFIG_LVP_TWS_LOGFBANK_FRAME_NUM_PER_CHANNEL;
#else
    s_context_header.logfbank_num = 0;
    s_context_header.logfbank_buffer = NULL;
    s_context_header.logfbank_buffer_size = 0;
    s_context_header.logfbank_frame_num_per_channel = 0;
#endif
/***************************************************************************/
/*********************************phase2 buf******************************************/
#if defined(CONFIG_LVP_ENABLE_CTC_DECODER)|| defined(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)|| defined(CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER)
#if (CONFIG_KWS_MODEL_DECODER_WIN_LENGTH < 40)
    ctc_decoder_window = (unsigned char *)lvp_phase3_buf + LVP_MIC_BUFFER_SIZE + LVP_LOGFBANK_BUFFER_SIZE;
#else
    ctc_decoder_window = (unsigned char *)lvp_phase2_buf;
#endif
#endif

#ifdef CONFIG_LVP_BUFFER_HAS_REF
    s_context_header.ref_num = CONFIG_LVP_TWS_REF_CHANNEL_NUM;
    s_context_header.ref_buffer = lvp_phase2_buf + LVP_CTC_DECODER_SIZE;
    s_context_header.ref_buffer_size = LVP_REF_BUFFER_SIZE;
#else
    s_context_header.ref_num = 0;
    s_context_header.ref_buffer = NULL;    // NULL
    s_context_header.ref_buffer_size = 0;
#endif

#ifdef CONFIG_ENABLE_HARDWARE_FFT
    s_context_header.fft_num = CONFIG_TWS_FFT_CHANNEL_NUM;
    s_context_header.fft_buffer = lvp_phase2_buf + LVP_CTC_DECODER_SIZE + LVP_REF_BUFFER_SIZE;
    s_context_header.fft_buffer_size = LVP_FFT_BUFFER_SIZE;
    s_context_header.fft_frame_num_per_channel = CONFIG_LVP_TWS_FFT_FRAME_NUM_PER_CHANNEL;
#else
    s_context_header.fft_num = 0;
    s_context_header.fft_buffer = NULL;    // NULL
    s_context_header.fft_buffer_size = 0;
    s_context_header.fft_frame_num_per_channel = 0;
#endif

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    s_feats_buffer = (unsigned char *)lvp_phase2_buf + LVP_CTC_DECODER_SIZE + LVP_REF_BUFFER_SIZE + LVP_FFT_BUFFER_SIZE;
#endif

#ifdef CONFIG_LVP_TWS_HAS_OUT_CHANNEL
    s_context_header.out_num = CONFIG_TWS_OUT_CHANNEL_NUM;
    s_context_header.out_buffer = (short*)((unsigned char *)lvp_phase2_buf + LVP_CTC_DECODER_SIZE + LVP_REF_BUFFER_SIZE + LVP_FFT_BUFFER_SIZE + LVP_FEATS_BUFFER_SIZE);
    s_context_header.out_buffer_size = LVP_OUT_BUFFER_SIZE;
    s_context_header.out_frame_num_per_channel  = _OUT_FRAME_NUM_PER_CHANNEL_;
#else
    s_context_header.out_num = 0;
    s_context_header.out_buffer = NULL;
    s_context_header.out_buffer_size  = 0;
    s_context_header.out_frame_num_per_channel  = 0;
#endif

    s_context_header.frame_length = PCM_FRAME_LENGTH;
    s_context_header.sample_rate  = PCM_SAMPLE_RATE;
    s_context_header.pcm_frame_num_per_context = CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CONTEXT;
    s_context_header.pcm_frame_num_per_channel = CONFIG_LVP_TWS_PCM_FRAME_NUM_PER_CHANNEL;
    s_context_header.logfbank_dim_per_frame    = LOGFBANK_DIM_PER_FRAME;
    s_context_header.fft_dim_per_frame         = FFT_DIM_PER_FRAME;

#ifdef CONFIG_KWS_SNPU_BUFFER_SIZE
    s_context_header.snpu_buffer_size = CONFIG_KWS_SNPU_BUFFER_SIZE/16*16 + 16;
#else
    s_context_header.snpu_buffer_size = 0;
#endif

    s_context_header.ctx_buffer = s_context_buffer;
    s_context_header.ctx_size   = sizeof(LVP_CONTEXT_BUFFER);
    s_context_header.ctx_num    = CONFIG_LVP_CONTEXT_NUM;
#ifdef CONFIG_LVP_FFT_VAD_ENABLE
    s_context_header.fft_vad_en = 1;
#else
    s_context_header.fft_vad_en = 0;
#endif
    lvp_buffer_instance.name = "tws";
    lvp_buffer_instance.LvpGetContextGap = LvpTwsGetContextGap;
    lvp_buffer_instance.LvpGetAudioOutBuffer = LvpTwsGetAudioOutBuffer;
    lvp_buffer_instance.LvpGetAudioOutBufferSize = LvpTwsGetAudioOutBufferSize;
    lvp_buffer_instance.LvpGetOutBufferSize = LvpTwsGetOutBufferSize;
    lvp_buffer_instance.LvpGetOutBufferSizePerContext = LvpTwsGetOutBufferSizePerContext;
    lvp_buffer_instance.LvpGetOutBuffer = LvpTwsGetOutBuffer;
    return 0;
}

void *LvpGetFeatsBuffer(void)
{
#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    return (void *)s_feats_buffer;
#else
    return NULL;
#endif
}

#if defined(CONFIG_LVP_ENABLE_CTC_DECODER)||defined(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)||defined(CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER)
unsigned char *ctc_decoder_window = NULL;
#endif

void *LvpGetCtcDecoderWindowAddr(void)
{
#if defined(CONFIG_LVP_ENABLE_CTC_DECODER)||defined(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)||defined(CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER)
    return (void *)ctc_decoder_window;
#endif
    return NULL;
}

int LvpGetCtcDecoderWindowLength(void)
{
    return LVP_CTC_DECODER_SIZE;
}

