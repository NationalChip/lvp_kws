#include "lvp_buffer_v2.h"
#include <soc_config.h>
#include "autoconf.h"
#include "rt_mem.h"

extern unsigned char _start_audio_in_section_;
extern unsigned char _start_audio_out_section_;
extern unsigned char _end_stack_section_;

const unsigned char *lvp_phase1_buf = (void*)&_start_npu_sram_section; //npu
const unsigned char *lvp_phase2_buf = (void*)&_end_stack_section_; //stage2 sram
const unsigned char *lvp_phase3_buf = (void*)&_start_audio_in_section_; //16k
const unsigned char *s_audio_out_buffer = (void*)&_start_audio_out_section_;

// Context Header
LVP_CONTEXT_HEADER s_context_header ALIGNED_ATTR(16);
// Context Buffer
LVP_CONTEXT_BUFFER s_context_buffer[CONFIG_LVP_CONTEXT_NUM] ALIGNED_ATTR(16);

//-------------------------------------------------------------------------------------------------

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

lvp_buffer_func_t lvp_buffer_instance;

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
    if(lvp_buffer_instance.LvpGetContextGap)
    {
        return lvp_buffer_instance.LvpGetContextGap();
    }
    else
    {
        printf("LvpGetContextGap function not exist\n");
    }
    return 0;
}

int LvpGetContextNum(void)
{
    return s_context_header.ctx_num;
}

int LvpGetMicChannelNum(void)
{
    return s_context_header.mic_num;
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

void *LvpGetContextHeader(void)
{
    return (void *)&s_context_header;
}

void *LvpGetAudioOutBuffer(void)
{
    if(lvp_buffer_instance.LvpGetAudioOutBuffer)
    {
        return (void *)lvp_buffer_instance.LvpGetAudioOutBuffer();
    }
    else
    {
        printf("LvpGetAudioOutBuffer function not exist\n");
    }
    return NULL;
}

int LvpGetAudioOutBufferSize(void)
{
    if(lvp_buffer_instance.LvpGetAudioOutBufferSize)
    {
        return lvp_buffer_instance.LvpGetAudioOutBufferSize();
    }
    else
    {
        printf("LvpGetAudioOutBufferSize function not exist\n");
    }
    return 0;
}

int LvpGetOutBufferSize(void)
{
    if(lvp_buffer_instance.LvpGetOutBufferSize)
    {
        return lvp_buffer_instance.LvpGetOutBufferSize();
    }
    else
    {
        printf("LvpGetOutBufferSize function not exist\n");
    }
    return 0;
}

int LvpGetOutBufferSizePerContext(void)
{
    if(lvp_buffer_instance.LvpGetOutBufferSizePerContext)
    {
        return lvp_buffer_instance.LvpGetOutBufferSizePerContext();
    }
    else
    {
        printf("LvpGetOutBufferSizePerContext function not exist\n");
    }
    return 0;
}

void *LvpGetOutBuffer(unsigned int index)
{
    if(lvp_buffer_instance.LvpGetOutBuffer)
    {
        return lvp_buffer_instance.LvpGetOutBuffer(index);
    }
    else
    {
        printf("LvpGetOutBuffer function not exist\n");
    }
    return 0;
}

#if !defined CONFIG_LVP_USE_BUFFER_V2 &&defined (MCU_SUPPORT_HEAP)
static unsigned char tmp_heap_buf[CONFIG_HEAP_SIZE_KB * 1024];
#endif

int LvpInitHeap(void)
{
#if defined(CONFIG_LVP_USE_BUFFER_V2)
    unsigned char *buf = (unsigned char*)&_start_npu_sram_section;
    unsigned int size  = (unsigned char*)&_end_npu_sram_section - (unsigned char*)&_start_npu_sram_section;
    printf("heap size =%d\n", size);
    rt_system_heap_init((void*)buf, (void*)((unsigned char*)buf + size));
#elif defined (MCU_SUPPORT_HEAP)
    rt_system_heap_init((void*)tmp_heap_buf, (void*)((unsigned char*)buf + CONFIG_HEAP_SIZE_KB*1024));
#endif
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
