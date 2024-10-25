/* Voice Signal Preprocess
* Copyright (C) 2001-2021 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* lvp_app_adpcm_demo.c
*
*/

#include <lvp_app.h>
#include <lvp_buffer.h>
#ifdef CONFIG_LVP_HAS_VOICE_PLAYER
#include <voice_player/lvp_voice_player.h>
#endif
#include <adpcm.h>
#include <driver/gx_dcache.h>
#ifdef CONFIG_APP_UART_RECORD
#include "../../lvp/common/lvp_uart_record.h"
#endif
#ifdef CONFIG_APP_PLAY_ADPCM
#include "record.h"     // adpcm
#endif
#ifdef CONFIG_APP_PLAY_PCM
#include "test.h"    // pcm
#endif

#define LOG_TAG "[ADPCM_DEMO]"

//=================================================================================================
static int SampleApinuspend(void *priv)
{
    AdpcmClearDecode();
    return 0;
}

static int SampleAppResume(void *priv)
{
    AdpcmClearEncode();
    return 0;
}

static int SampleAppInit(void)
{
    // printf(LOG_TAG" ---- %s ----\n", __func__);
#ifdef CONFIG_APP_AUDIO_PLAY
    LvpVoicePlayerInit(NULL);
    LvpVoicePlayerSetVolume(10);
#ifdef CONFIG_APP_PLAY_PCM
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(1, &context, &ctx_size);
    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    LvpVoicePlayerSetUserPcmBuffer((unsigned char *)ctx_header->mic_buffer, ctx_header->mic_buffer_size);
#endif
    AdpcmClearEncode();
    AdpcmClearDecode();
#endif

#ifdef CONFIG_APP_UART_RECORD
    UartRecordInit(0, 1000000);
#endif

    return 0;
}

#define SIMPLE_NUM 320
unsigned char  out_buff[SIMPLE_NUM] = {0};

// App Event Process
static int SampleAppEventResponse(APP_EVENT *app_event)
{
#ifdef CONFIG_APP_PLAY_LOCAL_DATA
    if (app_event->event_id < 100)
        return 0;
#ifdef CONFIG_APP_PLAY_ADPCM
    LvpVoicePlayerPlayAdpcm(mo16k_adpcm, mo16k_adpcm_len , 16000);
#else
    LvpVoicePlayerPlayPcm(record_wav, record_wav_len, 16000, 1, 0);
#endif
#elif defined CONFIG_APP_PLAY_MIC_DATA || defined CONFIG_APP_UART_RECORD
    LVP_CONTEXT *context;
    unsigned int ctx_size;

    LvpGetContext(app_event->ctx_index, &context, &ctx_size);
    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    unsigned int  mic_buffer_len_per_context = ctx_header->pcm_frame_num_per_context * \
                                               ctx_header->frame_length * \
                                               ctx_header->sample_rate * 2 / 1000;
     unsigned char *cur_mic_buffer = (unsigned char *)ctx_header->mic_buffer + \
                                    mic_buffer_len_per_context * \
                                    (context->ctx_index % \
                                    (ctx_header->mic_buffer_size / ctx_header->mic_num / mic_buffer_len_per_context));

    gx_dcache_invalid_range((unsigned int*)cur_mic_buffer, mic_buffer_len_per_context);
    unsigned long long t;
    t = gx_get_time_us();
    // printf("de %ld\n", gx_get_time_us() - t);
    signed short *in = (signed short *)cur_mic_buffer;
    int len = mic_buffer_len_per_context / sizeof(short);

    signed short *out = out_buff;

    Pcm2Adpcm(in, out, len);
    gx_dcache_clean_range((unsigned int*)out, len/2);

#if defined CONFIG_APP_PLAY_PCM || defined CONFIG_APP_UART_RECORD
    in = out_buff;
    signed short *out2 = (signed short *)cur_mic_buffer;
    Adpcm2Pcm (in, out2, len);
    gx_dcache_clean_range((unsigned int*)cur_mic_buffer, mic_buffer_len_per_context);
#endif

#ifdef CONFIG_APP_PLAY_PCM
    LvpVoicePlayerPlayPcm(cur_mic_buffer, mic_buffer_len_per_context, 16000, 1, 0);
#elif defined CONFIG_APP_PLAY_ADPCM
    LvpVoicePlayerPlayAdpcm((unsigned char *)out, len/2, 16000);
#endif
    // printf("de %ld\n", gx_get_time_us() - t);
#ifdef CONFIG_APP_UART_RECORD
     UartRecordChannelTask(cur_mic_buffer, mic_buffer_len_per_context, RECORD_CHANNEL_MIC0);
#endif
#endif
    return 0;
}

// APP Main Loop
static int SampleAppTaskLoop(void)
{
#ifdef CONFIG_APP_UART_RECORD
    UartRecordTick();
#endif
    return 0;
}

LVP_APP sample_app = {
    .app_name = "adpcm demo",
    .AppInit = SampleAppInit,
    .AppEventResponse = SampleAppEventResponse,
    .AppTaskLoop = SampleAppTaskLoop,
    .AppSuspend = SampleApinuspend,
    .suspend_priv = "",
    .AppResume = SampleAppResume,
    .resume_priv = "",
};

LVP_REGISTER_APP(sample_app);

