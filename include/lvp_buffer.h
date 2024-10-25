/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_context.h:
 *
 */

#pragma once

#ifndef CONFIG_LVP_USE_BUFFER_V2
#include <autoconf.h>
#include <stdio.h>
#include <lvp_attr.h>
#include <lvp_context.h>

//=================================================================================================

#ifndef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
# define CONFIG_KWS_MODEL_OUTPUT_LENGTH          0
# define CONFIG_KWS_MODEL_INPUT_WIN_LENGTH       0
# define CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME 0
# define CONFIG_KWS_MODEL_DECODER_WIN_LENGTH     0
#ifdef CONFIG_DENOISE_SNPU_BUFFER_SIZE
# define CONFIG_KWS_SNPU_BUFFER_SIZE CONFIG_DENOISE_SNPU_BUFFER_SIZE
#elif (defined CONFIG_SED_SNPU_BUFFER_SIZE)
# define CONFIG_KWS_SNPU_BUFFER_SIZE CONFIG_SED_SNPU_BUFFER_SIZE
#else
# define CONFIG_KWS_SNPU_BUFFER_SIZE             0
#endif
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

//=================================================================================================

int LvpInitBuffer(void);
void *LvpGetMicBufferAddr(void);
int LvpGetMicBufferSize(void);
int LvpGetMicChannelNum(void);
void *LvpGetLogfbankBufferAddr(void);
int LvpGetLogfbankBufferSize(void);
void *LvpGetFftBuffer(void);
int LvpGetFftBufferSize(void);

void *LvpGetOutBuffer(unsigned int index);
int LvpGetOutBufferSizePerContext(void);
int LvpGetOutBufferSize(void);

void *LvpGetContextHeader(void);
void *LvpGetFeatsBuffer(void);
void *LvpGetLogfbankBuffer(LVP_CONTEXT *context, unsigned int index);
int LvpGetContext(unsigned int index, LVP_CONTEXT **context, unsigned int *size);
short *LvpGetMicFrame(LVP_CONTEXT *context, unsigned int channel_num, unsigned int frame_index);

void *LvpGetAudioOutBuffer(void);
int LvpGetAudioOutBufferSize(void);

#ifdef CONFIG_MCU_SUPPORT_HEAP
int LvpInitHeap(void);
void *LvpMalloc(unsigned int size);
void *LvpRealloc(void *ptr, unsigned int size);
void *LvpCalloc(unsigned int nmemb, unsigned int size);
int LvpFree(void *ptr);
#endif

int LvpGetPcmFrameNumPerContext(void);
int LvpGetPcmFrameSize(void);
int LvpGetPcmFrameLengthMs(void);
int LvpGetPcmFrameNumPerChanel(void);
int LVpGetPcmSampleRate(void);
int LvpGetLogfbankFrameNumPerChannel(void);
int LvpGetFftFrameNumPerChannel(void);
int LvpGetContextGap(void);
int LvpGetContextNum(void);
int LvpGetPcmFrameNumPerChannel(void);
#else
#include "lvp_buffer_v2.h"
#endif
