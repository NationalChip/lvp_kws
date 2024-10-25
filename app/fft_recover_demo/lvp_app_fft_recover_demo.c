/* Voice Signal Preprocess
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app_sample.c
 *
 */
#include <stdio.h>
#include <lvp_app.h>
#include <lvp_buffer.h>
#include <driver/gx_gpio.h>
#include <multi_button/src/multi_button.h>
#include <gpio_led/gpio_led.h>
#include <board_misc_config.h>
#include <lvp_audio_out.h>
#include <soc_config.h>

#define LOG_TAG "[fft_recover_DEMO_APP]"

#define SAMPLE_RATE    16000
#define AUDIO_IN_FRAME_LENGTH    10
#define AUDIO_OUT_FRAME_LENGTH    (CONFIG_LVP_PCM_FRAME_NUM_PER_CONTEXT * AUDIO_IN_FRAME_LENGTH * SAMPLE_RATE / 1000 * sizeof(short))
#define AUDIO_OUT_FRAME_QUEUE_NUM   (CONFIG_AUDIO_OUT_BUFFER_SIZE / AUDIO_OUT_FRAME_LENGTH)

static int app_init = 0;

static unsigned char *lvp_audio_out_buffer;
static GX_AUDIO_OUT_FRAME s_aout_frame_queue_buffer[AUDIO_OUT_FRAME_QUEUE_NUM];

//=================================================================================================

static int FftRecoverDemoAppInit(void)
{
    if (!app_init) {

        LvpAudioOutInit();

        lvp_audio_out_buffer = (unsigned char *)LvpGetAudioOutBuffer();
        int size = LvpGetAudioOutBufferSize();
        LvpAudioOutSetPcmBuffer(lvp_audio_out_buffer, NULL, size);
        LvpAudioOutSetFrameQueue(s_aout_frame_queue_buffer, AUDIO_OUT_FRAME_QUEUE_NUM);

        LvpAudioOutSetPcmParam(16000, 16, 1);

        LvpAudioOutSetVolume(5);

        app_init = 1;
    }
    return 0;
}

static int s_offset = 0;
// App Event Process
static int FftRecoverDemoAppEventResponse(APP_EVENT *app_event)
{
    unsigned char* addr;
    unsigned int len;

    addr = (unsigned char*)app_event->ctx_index;
    len  = app_event->event_id;

    memcpy(lvp_audio_out_buffer + s_offset, addr, 320);
    s_offset = (s_offset + len * 2) % 1280;
    LvpAudioOutPlay(s_offset, 320);

    return 0;
}

// APP Main Loop
static int FftRecoverDemoAppTaskLoop(void)
{
    return 0;
}

LVP_APP fft_recover_demo_app = {
    .app_name = "fft recover demo app",
    .AppInit = FftRecoverDemoAppInit,
    .AppEventResponse = FftRecoverDemoAppEventResponse,
    .AppTaskLoop = FftRecoverDemoAppTaskLoop,
};

LVP_REGISTER_APP(fft_recover_demo_app);
