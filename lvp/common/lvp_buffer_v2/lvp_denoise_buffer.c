#include "lvp_buffer_v2.h"
#include <lvp_context.h>
#include <stdio.h>
#include <soc_config.h>
#include "autoconf.h"
#include "rt_mem.h"

//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_LVP_BUFFER_HAS_MIC
#define LVP_DENOISE_MIC_BUFFER_SIZE  (sizeof(short)*PCM_FRAME_SIZE * CONFIG_LVP_DENOISE_PCM_FRAME_NUM_PER_CHANNEL * CONFIG_DENOISE_MIC_CHANNEL_NUM)
#else
#define LVP_DENOISE_MIC_BUFFER_SIZE 0
#endif

#ifdef CONFIG_LVP_BUFFER_HAS_REF
#define LVP_DENOISE_REF_BUFFER_SIZE PCM_FRAME_SIZE * CONFIG_LVP_DENOISE_PCM_FRAME_NUM_PER_CHANNEL * CONFIG_DENOISE_REF_CHANNEL_NUM
#else
#define LVP_DENOISE_REF_BUFFER_SIZE 0
#endif

#ifdef CONFIG_ENABLE_HARDWARE_FFT
#define LVP_DENOISE_FFT_BUFFER_SIZE FFT_DIM_PER_FRAME * 2 * CONFIG_LVP_DENOISE_FFT_FRAME_NUM_PER_CHANNEL * CONFIG_DENOISE_FFT_CHANNEL_NUM
#else
#define LVP_DENOISE_FFT_BUFFER_SIZE 0
#endif

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
# ifdef CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION
static short s_logfbank_buffer[LOGFBANK_DIM_PER_FRAME * CONFIG_LVP_DENOISE_LOGFBANK_FRAME_NUM_PER_CHANNEL * CONFIG_DENOISE_LOGFBANK_CHANNEL_NUM] ALIGNED_ATTR(16);
# define LVP_DENOISE_LOGFBANK_BUFFER_SIZE 0
# endif
#define LVP_DENOISE_LOGFBANK_BUFFER_SIZE (sizeof(short)*LOGFBANK_DIM_PER_FRAME * CONFIG_LVP_DENOISE_LOGFBANK_FRAME_NUM_PER_CHANNEL * CONFIG_DENOISE_LOGFBANK_CHANNEL_NUM)
#else
# define LVP_DENOISE_LOGFBANK_BUFFER_SIZE 0
#endif

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
#define LVP_DENOISE_FEATS_BUFFER_SIZE CONFIG_KWS_MODEL_INPUT_WIN_LENGTH * CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME
static unsigned char *s_feats_buffer;
#else
#define LVP_DENOISE_FEATS_BUFFER_SIZE 0
#endif

#ifdef CONFIG_LVP_DENOISE_HAS_OUT_CHANNEL
# ifdef CONFIG_LVP_DENOISE_OUT_CHANNEL_CUSTOMIZE
# define _OUT_FRAME_NUM_PER_CHANNEL_ CONFIG_LVP_DENOISE_OUT_FRAME_NUM_PER_CHANNEL
# define  LVP_OUT_BUFFER_SIZE CONFIG_DENOISE_OUT_CHANNEL_NUM * CONFIG_LVP_DENOISE_OUT_FRAME_SIZE * _OUT_FRAME_NUM_PER_CHANNEL_ * sizeof(short)
# else
# define _OUT_FRAME_NUM_PER_CHANNEL_ (CONFIG_LVP_CONTEXT_NUM * CONFIG_LVP_DENOISE_PCM_FRAME_NUM_PER_CONTEXT)
# define  LVP_OUT_BUFFER_SIZE PCM_FRAME_SIZE * CONFIG_DENOISE_OUT_CHANNEL_NUM * _OUT_FRAME_NUM_PER_CHANNEL_ * sizeof(short)
# endif
#else
# define  LVP_OUT_BUFFER_SIZE 0
#endif

static int LvpDenoiseGetContextGap(void)
{
    return CONFIG_LVP_CONTEXT_GAP;
}

static void *LvpDenoiseGetAudioOutBuffer(void)
{
#if CONFIG_AUDIO_OUT_BUFFER_SIZE_KB
    return (void *)s_audio_out_buffer;
#else
    return NULL;
#endif
}

static int LvpDenoiseGetAudioOutBufferSize(void)
{
#if CONFIG_AUDIO_OUT_BUFFER_SIZE_KB
    return CONFIG_AUDIO_OUT_BUFFER_SIZE_KB*1024;
#else
    return 0;
#endif
}

int LvpDenoiseGetOutBufferSize(void)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(0, &context, &ctx_size);

    LVP_CONTEXT_HEADER *ctx_header      = context->ctx_header;
    unsigned int out_num                = ctx_header->out_num;
#ifdef CONFIG_LVP_DENOISE_OUT_CHANNEL_CUSTOMIZE
    unsigned int out_size_per_channel   = out_num * CONFIG_LVP_DENOISE_OUT_FRAME_SIZE * _OUT_FRAME_NUM_PER_CHANNEL_;
#else
    unsigned int out_frame_size         = PCM_FRAME_SIZE* sizeof(short); ///
    unsigned int out_frame_nun_channel  = ctx_header->out_frame_num_per_channel;
    unsigned int out_size_per_channel   = out_num * out_frame_size * out_frame_nun_channel;
#endif
    return out_size_per_channel;
}

int LvpDenoiseGetOutBufferSizePerContext(void)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(0, &context, &ctx_size);

    LVP_CONTEXT_HEADER *ctx_header      = context->ctx_header;
    unsigned int out_num                = ctx_header->out_num;
#ifdef CONFIG_LVP_DENOISE_OUT_CHANNEL_CUSTOMIZE
    unsigned int out_size_per_context   = out_num * CONFIG_LVP_DENOISE_OUT_FRAME_SIZE;
#else
    unsigned int out_frame_size         = PCM_FRAME_SIZE* sizeof(short); ///
    unsigned int out_frame_nun_context  = ctx_header->pcm_frame_num_per_context;
    unsigned int out_size_per_context   = out_num * out_frame_size * out_frame_nun_context;
#endif
    return out_size_per_context;
}

void *LvpDenoiseGetOutBuffer(unsigned int index)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(0, &context, &ctx_size);

    LVP_CONTEXT_HEADER *ctx_header      = context->ctx_header;
    unsigned int out_num                = ctx_header->out_num;
    if(!out_num)
        return NULL;
#ifdef CONFIG_LVP_DENOISE_OUT_CHANNEL_CUSTOMIZE
    unsigned int out_size_per_context   = out_num * CONFIG_LVP_DENOISE_OUT_FRAME_SIZE;
    unsigned int out_size_per_channel   = out_num * CONFIG_LVP_DENOISE_OUT_FRAME_SIZE * _OUT_FRAME_NUM_PER_CHANNEL_;
#else
    unsigned int out_frame_size         = PCM_FRAME_SIZE* sizeof(short); ///
    unsigned int out_frame_nun_channel  = ctx_header->out_frame_num_per_channel;
    unsigned int out_frame_nun_context  = ctx_header->pcm_frame_num_per_context;
    unsigned int out_size_per_context   = out_num * out_frame_size * out_frame_nun_context;
    unsigned int out_size_per_channel   = out_num * out_frame_size * out_frame_nun_channel;
#endif
    return (void *)((unsigned int)s_context_header.out_buffer + index % (out_size_per_channel / out_size_per_context) * out_size_per_context);
}

int LvpInitDenoiseBuffer(void)
{
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
    s_context_header.mic_num = CONFIG_DENOISE_MIC_CHANNEL_NUM;
    s_context_header.mic_buffer = (short *)lvp_phase3_buf;
    s_context_header.mic_buffer_size = LVP_DENOISE_MIC_BUFFER_SIZE;
#else
    s_context_header.mic_num = 0;
    s_context_header.mic_buffer = NULL;
    s_context_header.mic_buffer_size = 0;
#endif


#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    s_context_header.logfbank_num = CONFIG_DENOISE_LOGFBANK_CHANNEL_NUM;
# ifdef CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION
    s_context_header.logfbank_buffer = s_logfbank_buffer;
    s_context_header.logfbank_buffer_size = sizeof(s_logfbank_buffer);
# else
    s_context_header.logfbank_buffer =(short *)(lvp_phase3_buf + LVP_DENOISE_MIC_BUFFER_SIZE);
    s_context_header.logfbank_buffer_size = LVP_DENOISE_LOGFBANK_BUFFER_SIZE;
# endif
    s_context_header.logfbank_frame_num_per_channel = CONFIG_LVP_DENOISE_LOGFBANK_FRAME_NUM_PER_CHANNEL;
#else
    s_context_header.logfbank_num = 0;
    s_context_header.logfbank_buffer = NULL;
    s_context_header.logfbank_buffer_size = 0;
    s_context_header.logfbank_frame_num_per_channel = 0;
#endif
/***************************************************************************/
/*********************************phase2 buf******************************************/
#ifdef CONFIG_LVP_BUFFER_HAS_REF
    s_context_header.ref_num = CONFIG_DENOISE_REF_CHANNEL_NUM;
    s_context_header.ref_buffer = lvp_phase2_buf ;
    s_context_header.ref_buffer_size = LVP_DENOISE_REF_BUFFER_SIZE;
#else
    s_context_header.ref_num = 0;
    s_context_header.ref_buffer = NULL;    // NULL
    s_context_header.ref_buffer_size = 0;
#endif

#ifdef CONFIG_ENABLE_HARDWARE_FFT
    s_context_header.fft_num = CONFIG_DENOISE_FFT_CHANNEL_NUM;
    s_context_header.fft_buffer = lvp_phase2_buf + LVP_DENOISE_REF_BUFFER_SIZE;
    s_context_header.fft_buffer_size = LVP_DENOISE_FFT_BUFFER_SIZE;
    s_context_header.fft_frame_num_per_channel = CONFIG_LVP_DENOISE_FFT_FRAME_NUM_PER_CHANNEL;
#else
    s_context_header.fft_num = 0;
    s_context_header.fft_buffer = NULL;    // NULL
    s_context_header.fft_buffer_size = 0;
    s_context_header.fft_frame_num_per_channel = 0;
#endif

#ifdef CONFIG_ENABLE_HARDWARE_LOGFBANK
    s_feats_buffer = (unsigned char *)lvp_phase2_buf + LVP_DENOISE_REF_BUFFER_SIZE + LVP_DENOISE_FFT_BUFFER_SIZE;
#endif

#ifdef CONFIG_DENOISE_OUT_CHANNEL_NUM
    s_context_header.out_num = CONFIG_DENOISE_OUT_CHANNEL_NUM;
    s_context_header.out_buffer = (short*)((unsigned char *)lvp_phase2_buf + LVP_DENOISE_REF_BUFFER_SIZE + LVP_DENOISE_FFT_BUFFER_SIZE + LVP_DENOISE_FEATS_BUFFER_SIZE);
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
    s_context_header.pcm_frame_num_per_context = CONFIG_LVP_DENOISE_PCM_FRAME_NUM_PER_CONTEXT;
    s_context_header.pcm_frame_num_per_channel = CONFIG_LVP_DENOISE_PCM_FRAME_NUM_PER_CHANNEL;
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

    lvp_buffer_instance.name = "denoise";
    lvp_buffer_instance.LvpGetContextGap = LvpDenoiseGetContextGap;
    lvp_buffer_instance.LvpGetAudioOutBuffer = LvpDenoiseGetAudioOutBuffer;
    lvp_buffer_instance.LvpGetAudioOutBufferSize = LvpDenoiseGetAudioOutBufferSize;
    lvp_buffer_instance.LvpGetOutBufferSize = LvpDenoiseGetOutBufferSize;
    lvp_buffer_instance.LvpGetOutBufferSizePerContext = LvpDenoiseGetOutBufferSizePerContext;
    lvp_buffer_instance.LvpGetOutBuffer = LvpDenoiseGetOutBuffer;
    return 0;
}
