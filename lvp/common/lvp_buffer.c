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

#define LOG_TAG "[LVP_BUFF]"

//=================================================================================================

// Check some parameters
#if (PCM_FRAME_SIZE * CONFIG_LVP_PCM_FRAME_NUM_PER_CHANNEL * 2) % 128 != 0
#error "Active Channel Buffer Size SHOULD be times of 1024!"
#endif

#if CONFIG_LVP_PCM_FRAME_NUM_PER_CHANNEL % CONFIG_LVP_PCM_FRAME_NUM_PER_CONTEXT != 0
#error "SRAM Frame Number of Channel SHOULD be times of Frame Number of Context!"
#endif

#if (defined(CONFIG_LVP_LOGFBANK_FRAME_NUM_PER_CHANNEL) && CONFIG_LVP_LOGFBANK_FRAME_NUM_PER_CHANNEL % CONFIG_LVP_PCM_FRAME_NUM_PER_CONTEXT != 0)
#error "Logfbank Frame Number of Channel SHOULD be times of Frame Number of Context!"
#endif

//-------------------------------------------------------------------------------------------------

#ifdef CONFIG_LVP_BUFFER_HAS_MIC
// Microphone Input Buffer in SRAM
static short s_mic_buffer[PCM_FRAME_SIZE * CONFIG_LVP_PCM_FRAME_NUM_PER_CHANNEL * CONFIG_MIC_CHANNEL_NUM] ALIGNED_ATTR(16) DRAM0_AUDIO_IN_ATTR;
#endif

#ifdef CONFIG_LVP_BUFFER_HAS_REF
// Ref Input Buffer in SRAM
static short s_ref_buffer[PCM_FRAME_SIZE * CONFIG_LVP_PCM_FRAME_NUM_PER_CHANNEL * CONFIG_REF_CHANNEL_NUM] ALIGNED_ATTR(16);
#endif

#ifdef CONFIG_ENABLE_HARDWARE_FFT
// FFT(complex) Input Buffer in SRAM
static int s_fft_buffer[FFT_DIM_PER_FRAME * 2 * CONFIG_LVP_FFT_FRAME_NUM_PER_CHANNEL * CONFIG_FFT_CHANNEL_NUM] ALIGNED_ATTR(16) DRAM0_AUDIO_IN_ATTR;
#endif

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
# ifdef CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION
// Logfbank Input Buffer in SRAM
static short s_logfbank_buffer[LOGFBANK_DIM_PER_FRAME * CONFIG_LVP_LOGFBANK_FRAME_NUM_PER_CHANNEL * CONFIG_LOGFBANK_CHANNEL_NUM] ALIGNED_ATTR(16);
# else
static short s_logfbank_buffer[LOGFBANK_DIM_PER_FRAME * CONFIG_LVP_LOGFBANK_FRAME_NUM_PER_CHANNEL * CONFIG_LOGFBANK_CHANNEL_NUM] ALIGNED_ATTR(16) DRAM0_AUDIO_IN_ATTR;
# endif
#endif

#ifdef CONFIG_LVP_HAS_OUT_CHANNEL
# ifdef CONFIG_LVP_OUT_CHANNEL_CUSTOMIZE
# define _OUT_FRAME_NUM_PER_CHANNEL_ CONFIG_LVP_OUT_FRAME_NUM_PER_CHANNEL
static short s_out_buffer[CONFIG_OUT_CHANNEL_NUM * CONFIG_LVP_OUT_FRAME_SIZE * _OUT_FRAME_NUM_PER_CHANNEL_];
# else
# define _OUT_FRAME_NUM_PER_CHANNEL_ (CONFIG_LVP_CONTEXT_NUM * CONFIG_LVP_PCM_FRAME_NUM_PER_CONTEXT)
static short s_out_buffer[PCM_FRAME_SIZE * CONFIG_OUT_CHANNEL_NUM * _OUT_FRAME_NUM_PER_CHANNEL_];
# endif
#endif

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
// TODO
# ifdef CONFIG_LVP_ENABLE_CTC_GX_DECODER
static short s_feats_buffer[CONFIG_KWS_MODEL_INPUT_WIN_LENGTH * CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME] ALIGNED_ATTR(16) DRAM0_AUDIO_IN_ATTR;
# else
static short s_feats_buffer[CONFIG_KWS_MODEL_INPUT_WIN_LENGTH * CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME] ALIGNED_ATTR(16);
# endif
#endif

// Context Header
static LVP_CONTEXT_HEADER s_context_header ALIGNED_ATTR(16);
// Context Buffer
static LVP_CONTEXT_BUFFER s_context_buffer[CONFIG_LVP_CONTEXT_NUM] ALIGNED_ATTR(16);

#if CONFIG_AUDIO_OUT_BUFFER_SIZE
static unsigned char s_audio_out_buffer[CONFIG_AUDIO_OUT_BUFFER_SIZE] ALIGNED_ATTR(16) __attribute__((section(".audio_out,\"aw\",@nobits#")));
#endif

#ifdef CONFIG_MCU_SUPPORT_HEAP
static unsigned char tmp_heap_buf[CONFIG_HEAP_SIZE_KB * 1024];
#endif

//-------------------------------------------------------------------------------------------------

int LvpInitBuffer(void)
{
    //printf (LOG_TAG"Enter LvpInitBuffer\n");

    // Clear
    memset(&s_context_header, 0, sizeof(s_context_header));
    memset(&s_context_buffer, 0, sizeof(s_context_buffer));
#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    memset(&s_feats_buffer  , 0, sizeof(s_feats_buffer));
#endif

    // Initialize Context Header
    s_context_header.version = LVP_CONTEXT_VERSION;

#ifdef CONFIG_LVP_BUFFER_HAS_MIC
    s_context_header.mic_num = CONFIG_MIC_CHANNEL_NUM;
    s_context_header.mic_buffer = s_mic_buffer;
    s_context_header.mic_buffer_size = sizeof(s_mic_buffer);
#else
    s_context_header.mic_num = 0;
    s_context_header.mic_buffer = NULL;
    s_context_header.mic_buffer_size = 0;
#endif

#ifdef CONFIG_LVP_BUFFER_HAS_REF
    s_context_header.ref_num = CONFIG_LVP_REF_CHANNEL_NUM;
    s_context_header.ref_buffer = s_ref_buffer;
    s_context_header.ref_buffer_size = sizeof(s_ref_buffer);
#else
    s_context_header.ref_num = 0;
    s_context_header.ref_buffer = NULL;    // NULL
    s_context_header.ref_buffer_size = 0;
#endif

#ifdef CONFIG_ENABLE_HARDWARE_FFT
    s_context_header.fft_num = CONFIG_FFT_CHANNEL_NUM;
    s_context_header.fft_buffer = s_fft_buffer;
    s_context_header.fft_buffer_size = sizeof(s_fft_buffer);
    s_context_header.fft_frame_num_per_channel = CONFIG_LVP_FFT_FRAME_NUM_PER_CHANNEL;
#else
    s_context_header.fft_num = 0;
    s_context_header.fft_buffer = NULL;    // NULL
    s_context_header.fft_buffer_size = 0;
    s_context_header.fft_frame_num_per_channel = 0;
#endif

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    s_context_header.logfbank_num = CONFIG_LOGFBANK_CHANNEL_NUM;
    s_context_header.logfbank_buffer = s_logfbank_buffer;
    s_context_header.logfbank_buffer_size = sizeof(s_logfbank_buffer);
    s_context_header.logfbank_frame_num_per_channel = CONFIG_LVP_LOGFBANK_FRAME_NUM_PER_CHANNEL;
#else
    s_context_header.logfbank_num = 0;
    s_context_header.logfbank_buffer = NULL;
    s_context_header.logfbank_buffer_size = 0;
    s_context_header.logfbank_frame_num_per_channel = 0;
#endif

#ifdef CONFIG_LVP_HAS_OUT_CHANNEL
    s_context_header.out_num                    = CONFIG_OUT_CHANNEL_NUM;
    s_context_header.out_buffer                 = s_out_buffer;
    s_context_header.out_buffer_size            = sizeof(s_out_buffer);
    s_context_header.out_frame_num_per_channel  = _OUT_FRAME_NUM_PER_CHANNEL_;
#else
    s_context_header.out_num                    = 0;
    s_context_header.out_buffer                 = 0;
    s_context_header.out_buffer_size            = 0;
    s_context_header.out_frame_num_per_channel  = 0;
#endif

    s_context_header.frame_length = PCM_FRAME_LENGTH;
    s_context_header.sample_rate  = PCM_SAMPLE_RATE;
    s_context_header.pcm_frame_num_per_context = CONFIG_LVP_PCM_FRAME_NUM_PER_CONTEXT;
    s_context_header.pcm_frame_num_per_channel = CONFIG_LVP_PCM_FRAME_NUM_PER_CHANNEL;
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

    return 0;
}

void *LvpGetMicBufferAddr(void)
{
    return (void *)s_context_header.mic_buffer;
}

int LvpGetMicBufferSize(void)
{
    return s_context_header.mic_buffer_size;
}

void *LvpGetLogfbankBufferAddr(void)
{
    return (void *)s_context_header.logfbank_buffer;
}

int LvpGetLogfbankBufferSize(void)
{
    return s_context_header.logfbank_buffer_size;
}

void *LvpGetFftBuffer(void)
{
    return (void *)s_context_header.fft_buffer;
}

int LvpGetFftBufferSize(void)
{
    return s_context_header.fft_buffer_size;
}

void *LvpGetContextHeader(void)
{
    return (void *)&s_context_header;
}

void *LvpGetFeatsBuffer(void)
{
#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    return (void *)s_feats_buffer;
#else
    return NULL;
#endif
}


void *LvpGetLogfbankBuffer(LVP_CONTEXT *context, unsigned int index)
{
    LVP_CONTEXT_HEADER *ctx_header          = context->ctx_header;
    unsigned int logfbank_num               = ctx_header->logfbank_frame_num_per_channel;
    unsigned int logfbank_dim               = ctx_header->logfbank_dim_per_frame;
    unsigned int pcm_frame_num_per_context  = ctx_header->pcm_frame_num_per_context;
    unsigned int curr_index                 = (index * pcm_frame_num_per_context) % logfbank_num;
    void *logfbank_buffer                   = ctx_header->logfbank_buffer;

    // gx_dcache_invalid_range(logfbank_buffer, ctx_header->logfbank_buffer_size);
    return (LVP_CONTEXT *)((unsigned int)logfbank_buffer + curr_index * logfbank_dim * sizeof(short));
}

int LvpGetOutBufferSize(void)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(0, &context, &ctx_size);

    LVP_CONTEXT_HEADER *ctx_header      = context->ctx_header;
    unsigned int out_num                = ctx_header->out_num;
#ifdef CONFIG_LVP_OUT_CHANNEL_CUSTOMIZE
    unsigned int out_size_per_channel   = out_num * CONFIG_LVP_OUT_FRAME_SIZE * _OUT_FRAME_NUM_PER_CHANNEL_;
#else
    unsigned int out_frame_size         = PCM_FRAME_SIZE * sizeof(short); ///
    unsigned int out_frame_nun_channel  = ctx_header->out_frame_num_per_channel;
    unsigned int out_size_per_channel   = out_num * out_frame_size * out_frame_nun_channel;
#endif
    return out_size_per_channel;
}

int LvpGetOutBufferSizePerContext(void)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(0, &context, &ctx_size);

    LVP_CONTEXT_HEADER *ctx_header      = context->ctx_header;
    unsigned int out_num                = ctx_header->out_num;
#ifdef CONFIG_LVP_OUT_CHANNEL_CUSTOMIZE
    unsigned int out_size_per_context   = out_num * CONFIG_LVP_OUT_FRAME_SIZE;
#else
    unsigned int out_frame_size         = PCM_FRAME_SIZE * sizeof(short); ///
    unsigned int out_frame_nun_context  = ctx_header->pcm_frame_num_per_context;
    unsigned int out_size_per_context   = out_num * out_frame_size * out_frame_nun_context;
#endif
    return out_size_per_context;
}

void *LvpGetOutBuffer(unsigned int index)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(0, &context, &ctx_size);

    LVP_CONTEXT_HEADER *ctx_header      = context->ctx_header;
    unsigned int out_num                = ctx_header->out_num;
    if(!out_num)
        return NULL;
#ifdef CONFIG_LVP_OUT_CHANNEL_CUSTOMIZE
    unsigned int out_size_per_context   = out_num * CONFIG_LVP_OUT_FRAME_SIZE;
    unsigned int out_size_per_channel   = out_num * CONFIG_LVP_OUT_FRAME_SIZE * _OUT_FRAME_NUM_PER_CHANNEL_;
#else
    unsigned int out_frame_size         = PCM_FRAME_SIZE * sizeof(short); ///
    unsigned int out_frame_nun_channel  = ctx_header->out_frame_num_per_channel;
    unsigned int out_frame_nun_context  = ctx_header->pcm_frame_num_per_context;
    unsigned int out_size_per_context   = out_num * out_frame_size * out_frame_nun_context;
    unsigned int out_size_per_channel   = out_num * out_frame_size * out_frame_nun_channel;
#endif

    return (void *)((unsigned int)s_context_header.out_buffer + index % (out_size_per_channel / out_size_per_context) * out_size_per_context);
}

int LvpGetContext(unsigned int index, LVP_CONTEXT **context, unsigned int *size)
{
    LVP_CONTEXT_BUFFER *context_buffer  = &s_context_buffer[index % CONFIG_LVP_CONTEXT_NUM];
    context_buffer->context.ctx_header  = &s_context_header;

#ifdef CONFIG_KWS_SNPU_BUFFER_SIZE
    context_buffer->context.snpu_buffer = context_buffer->snpu_buffer;
#else
    context_buffer->context.snpu_buffer = NULL;
#endif

    *context = &context_buffer->context;
    *size    = sizeof(LVP_CONTEXT_BUFFER);

    return 0;
}

short *LvpGetMicFrame(LVP_CONTEXT *context, unsigned int channel_num, unsigned int frame_index)
{
    LVP_CONTEXT_HEADER *ctx_header  = context->ctx_header;
    unsigned int sample_num_per_frame = ctx_header->frame_length * ctx_header->sample_rate / 1000;
    unsigned int sample_num_per_context = sample_num_per_frame * ctx_header->pcm_frame_num_per_context;
    unsigned int sample_num_per_channel = sample_num_per_frame * ctx_header->pcm_frame_num_per_channel;
    short *mic_frame_buffer = ctx_header->mic_buffer;

    mic_frame_buffer += channel_num * sample_num_per_channel;
    mic_frame_buffer += sample_num_per_context * (context->ctx_index % ctx_header->ctx_num) + frame_index * sample_num_per_frame;

    return mic_frame_buffer;
}

void *LvpGetAudioOutBuffer(void)
{
#if CONFIG_AUDIO_OUT_BUFFER_SIZE
    return (void *)s_audio_out_buffer;
#else
    return NULL;
#endif
}

int LvpGetAudioOutBufferSize(void)
{
#if CONFIG_AUDIO_OUT_BUFFER_SIZE
    return CONFIG_AUDIO_OUT_BUFFER_SIZE;
#else
    return 0;
#endif
}

#ifdef CONFIG_MCU_SUPPORT_HEAP
int LvpInitHeap(void)
{
    rt_system_heap_init((void*)tmp_heap_buf, (void*)((unsigned char*)tmp_heap_buf + CONFIG_HEAP_SIZE_KB * 1024));
    return 0;
}

void *LvpMalloc(unsigned int size)
{
    void *ptr;
    ptr = rt_malloc(size);
    return ptr;
}

void *LvpCalloc(unsigned int nmemb, unsigned int size)
{
    void *ptr;
    ptr = rt_calloc(nmemb, size);
    return ptr;
}

void *LvpRealloc(void *ptr, unsigned int size)
{
    ptr = rt_realloc(ptr, size);
    return ptr;
}

int LvpFree(void *ptr)
{
    rt_free(ptr);
    return 0;
}
#endif

int LvpGetPcmFrameNumPerContext(void)
{
     return s_context_header.pcm_frame_num_per_context;
}

int LvpGetPcmFrameSize(void)
{
    return s_context_header.sample_rate* s_context_header.frame_length/1000;
}

int LvpGetPcmFrameLengthMs(void)
{
    return s_context_header.frame_length;
}

int LvpGetPcmFrameNumPerChannel(void)
{
    return s_context_header.pcm_frame_num_per_channel;
}

int LVpGetPcmSampleRate(void)
{
    return s_context_header.sample_rate;
}

int LvpGetLogfbankFrameNumPerChannel(void)
{
    return s_context_header.logfbank_frame_num_per_channel;
}

int LvpGetFftFrameNumPerChannel(void)
{
    return s_context_header.fft_frame_num_per_channel;
}

int LvpGetContextGap(void)
{
    return CONFIG_LVP_CONTEXT_GAP;
}

int LvpGetContextNum(void)
{
    return s_context_header.ctx_num;
}

int LvpGetMicChannelNum(void)
{
    return s_context_header.mic_num;
}
