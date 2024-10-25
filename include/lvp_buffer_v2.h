#pragma once
#include <autoconf.h>
#include <stdio.h>
#include <lvp_attr.h>
#include <lvp_context.h>

#ifndef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
# define CONFIG_KWS_MODEL_OUTPUT_LENGTH          65
# define CONFIG_KWS_MODEL_INPUT_WIN_LENGTH       17
# define CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME 40
# define CONFIG_KWS_MODEL_DECODER_WIN_LENGTH     23
# define CONFIG_KWS_SNPU_BUFFER_SIZE             2772
#endif

#ifdef CONFIG_PCM_SAMPLE_RATE_8K
# define PCM_SAMPLE_RATE     8000
#endif

#ifdef CONFIG_PCM_SAMPLE_RATE_16K
# define PCM_SAMPLE_RATE     16000
#endif

#ifdef CONFIG_PCM_SAMPLE_RATE_48K
# define PCM_SAMPLE_RATE     48000
#endif

#ifndef PCM_SAMPLE_RATE
# error "Unknown Sample Rate"
#endif

#ifdef CONFIG_PCM_FRAME_LENGTH_10MS
# define PCM_FRAME_LENGTH    10
#endif

#ifdef CONFIG_PCM_FRAME_LENGTH_16MS
# define PCM_FRAME_LENGTH    16
#endif

#ifndef PCM_FRAME_LENGTH
# error "Unknown Frame Length"
#endif

// size of a channel and a frame
#define PCM_FRAME_SIZE      (PCM_SAMPLE_RATE * PCM_FRAME_LENGTH / 1000)

// size of a fft per frame
#define FFT_FRAME_LEGTH         (10)
#define FFT_DIM_PER_FRAME       (256)

// size of a logfbank per frame
#define LOGFBANK_FRAME_LEGTH    (10)
#define LOGFBANK_DIM_PER_FRAME  (40)

#define MCU_TO_DEV(addr) ((unsigned int)addr & 0x0fffffff)
#define DEV_TO_MCU(addr) ((unsigned int)addr + 0x20000000)

//=================================================================================================
typedef struct {
    LVP_CONTEXT context;
#ifdef CONFIG_KWS_SNPU_BUFFER_SIZE
    unsigned char snpu_buffer[CONFIG_KWS_SNPU_BUFFER_SIZE/16*16 + 16] ALIGNED_ATTR(16);
#endif
} ALIGNED_ATTR(16) LVP_CONTEXT_BUFFER;

extern const unsigned char *lvp_phase1_buf; //npu
extern const unsigned char *lvp_phase2_buf; //stage2 sram
extern const unsigned char *lvp_phase3_buf; //16k
extern const unsigned char *s_audio_out_buffer;
extern unsigned char _start_npu_sram_section;
extern unsigned char _end_npu_sram_section;
extern LVP_CONTEXT_HEADER s_context_header ALIGNED_ATTR(16);
// Context Buffer
extern LVP_CONTEXT_BUFFER s_context_buffer[CONFIG_LVP_CONTEXT_NUM] ALIGNED_ATTR(16);


#if defined(CONFIG_LVP_ENABLE_CTC_DECODER)||defined(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)||defined(CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER)
extern unsigned char *ctc_decoder_window;
#define LVP_CTC_DECODER_SIZE  sizeof(float)*(CONFIG_KWS_MODEL_DECODER_WIN_LENGTH * CONFIG_KWS_MODEL_OUTPUT_LENGTH)
#else
#define LVP_CTC_DECODER_SIZE 0
#endif

//=================================================================================================
int LvpGetContext(unsigned int index, LVP_CONTEXT **context, unsigned int *size);
// int LvpInitBuffer(void);
void *LvpGetMicBufferAddr(void);
int LvpGetMicBufferSize(void);
int LvpGetMicChannelNum(void);
short *LvpGetMicFrame(LVP_CONTEXT *context, unsigned int channel_num, unsigned int frame_index);

void *LvpGetLogfbankBufferAddr(void);
int LvpGetLogfbankBufferSize(void);
void *LvpGetContextHeader(void);
void *LvpGetFeatsBuffer(void);
void *LvpGetLogfbankBuffer(LVP_CONTEXT *context, unsigned int index);

void *LvpGetAudioOutBuffer(void);
int LvpGetAudioOutBufferSize(void);
void *LvpGetOutBuffer(unsigned int index);
int LvpGetOutBufferSizePerContext(void);
int LvpGetOutBufferSize(void);
int LvpGetPcmFrameNumPerContext(void);
int LvpGetPcmFrameSize(void);
int LvpGetPcmFrameLengthMs(void);
int LvpGetPcmFrameNumPerChanel(void);
int LVpGetPcmSampleRate(void);
int LvpGetLogfbankFrameNumPerChannel(void);
int LvpGetFftFrameNumPerChannel(void);
int LvpGetContextGap(void);
int LvpGetContextNum(void);
int LvpInitHeap(void);
void *LvpMalloc(unsigned int size);
void *LvpRealloc(void *ptr, unsigned int size);
void *LvpCalloc(unsigned int nmemb, unsigned int size);
int LvpFree(void *ptr);

short *LvpGetMicFrame(LVP_CONTEXT *context, unsigned int channel_num, unsigned int frame_index);

int LvpInitTwsBuffer(void);
void *LvpGetCtcDecoderWindowAddr(void);
int LvpGetCtcDecoderWindowLength(void);
int LvpInitDenoiseBuffer(void);
void LvpDenoiseGetInputBuffer(void **addr, int *len);
typedef struct {
    const char *name;
    int (*LvpGetContextGap)(void);
    void* (*LvpGetAudioOutBuffer)(void);
    int (*LvpGetAudioOutBufferSize)(void);
    int (*LvpGetOutBufferSize)(void);
    int (*LvpGetOutBufferSizePerContext)(void);
    void* (*LvpGetOutBuffer)(unsigned int index);
}lvp_buffer_func_t;
extern lvp_buffer_func_t lvp_buffer_instance;
