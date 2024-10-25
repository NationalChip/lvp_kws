/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app_sample.c
 *
 */
#include <stdio.h>
#include <lvp_app.h>
#include <lvp_buffer.h>
#include <lvp_voice_player.h>
#include "types.h"
#include "resource/resource.h"

#define LOG_TAG "[VOICE_PLAYER_APP]"
static int app_init = 0;

//=================================================================================================


static void PlayerEventCallback(int player_event_id, void *data)
{
    //APP_EVENT player_over = {
    //    .event_id = PLAY_OVER_EVENT_ID,
    //};
    //LvpTriggerAppEvent(&player_over);
    printf("player event %d\n", player_event_id);
    //if (player_event_id == EVENT_PLAYER_LAST_FRAME)
    //    LvpVoicePlayerPlayPcm(__1khz_pcm, 12764, 16000, 1, 0);
}

static int SampleAppInit(void)
{
    if (!app_init) {
        printf(LOG_TAG" ---- %s ----\n", __func__);

        LvpVoicePlayerInit(PlayerEventCallback);
        LVP_CONTEXT *context;
        unsigned int ctx_size;
        LvpGetContext(1, &context, &ctx_size);
        LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
        LvpVoicePlayerSetUserPcmBuffer((unsigned char *)ctx_header->mic_buffer, ctx_header->mic_buffer_size);
        app_init = 1;
    }
    return 0;
}

// App Event Process
#ifdef CONFIG_PLAY_MIC_DATA
static int SampleAppEventResponse(APP_EVENT *app_event)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    if (app_event->ctx_index  < 10 )
        return 0;
    LvpGetContext(app_event->ctx_index, &context, &ctx_size);
    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    unsigned int  mic_buffer_len_per_context = ctx_header->pcm_frame_num_per_context * \
                                               ctx_header->frame_length * \
                                               ctx_header->sample_rate * 2 / 1000;
    unsigned char *cur_mic_buffer = (unsigned char *)ctx_header->mic_buffer + \
                                    mic_buffer_len_per_context * \
                                    (context->ctx_index % \
                                     (ctx_header->mic_buffer_size / ctx_header->mic_num / mic_buffer_len_per_context));
    LvpVoicePlayerPlayPcm(cur_mic_buffer, mic_buffer_len_per_context, 16000, 1, 1);

    return 0;
}
#endif

#if (defined CONFIG_PLAY_WAV_DATA) || (defined CONFIG_PLAY_OPUS_DATA)
static int SampleAppEventResponse(APP_EVENT *app_event)
{
    if (app_event->event_id == 100) {
# ifdef CONFIG_PLAY_WAV_DATA
        LvpVoicePlayerPlay(wozai_wav);
# elif defined CONFIG_PLAY_OPUS_DATA
        LvpVoicePlayerPlay(wozai_opus);
# endif
    }
    return 0;
}
#endif

#ifdef CONFIG_YILIAN_FAN_SOLUTION
typedef struct {
    int kws_value;
    const unsigned char *resource_position;
}voice_resource;

static voice_resource resource[] = {
    {100, wozai_opus},
    {101, kaifengshan_opus},
    {102, guanfengshan_opus},
    {103, kaiyaotou_opus},
    {104, kaishiyaotou_opus},
    {105, guanyaotou_opus},
    {106, guanbiyaotou_opus},
    {107, zengdafengsu_opus},
    {108, dangweitiaoda_opus},
    {109, jianxiaofengsu_opus},
    {110, dangweitiaoxiao_opus},
    {111, fengsuzuida_opus},
    {112, fengsuzuixiao_opus},
    {113, kaidingshi_opus},
    {114, guandingshi_opus},
    {115, shuimianfeng_opus},
    {116, ziranfeng_opus},
    {117, zhengchangfeng_opus},
    {118, zengdayinliang_opus},
    {119, jianxiaoyinliang_opus},
    {120, NULL},
    {121, NULL},
    {122, NULL},
    {123, NULL},
    {124, NULL},
};

#define RESOURCE_COUNT (sizeof(resource)/sizeof(voice_resource))
#define PLAY_OVER_EVENT_ID 999
#define TIME_OUT 200   // 200 * 40 = 8s
static int SampleAppEventResponse(APP_EVENT *app_event)
{
#if 1
    int volume;
    int index;
    static int main_kws = 100;
    static int main_kws_remain = -1;

    if (LvpVoicePlayerGetStatus() == PLAYER_STATUS_PLAY)
        return 0;
    if (main_kws_remain >= 0) {
        main_kws_remain--;
        if (main_kws_remain == 0) {
            LvpVoicePlayerPlay(zaijian_opus);
            printf("play time out player\n");
        }
        if (main_kws_remain > 190)
            return 0;
    }

    if (app_event->event_id < 100) {
        return 0;
    }
    for (index = 0; index < RESOURCE_COUNT; index++) {
        if (resource[index].kws_value == app_event->event_id)
            break;
        if ((index + 1) == RESOURCE_COUNT)
            return 0;
    }
    if (app_event->event_id != main_kws && main_kws_remain > 0) {
        if (app_event->event_id == 118) {
            volume = LvpVoicePlayerGetVolume();
            LvpVoicePlayerSetVolume(volume + 10);
        } else if (app_event->event_id == 119) {
            volume = LvpVoicePlayerGetVolume();
            LvpVoicePlayerSetVolume(volume - 10);
        }

        LvpVoicePlayerPlay(resource[index].resource_position);
        main_kws_remain = TIME_OUT;
    } else
    if (app_event->event_id == main_kws) {
        LvpVoicePlayerPlay(resource[index].resource_position);
        main_kws_remain = TIME_OUT;
    }
#endif
    return 0;
}
#endif

// APP Main Loop
static int SampleAppTaskLoop(void)
{
    return 0;
}


static int SampleSuspend(void *priv)
{
    LvpVoicePlayerSuspend();
    return 0;
}

static int SampleResume(void *priv)
{
    LvpVoicePlayerResume();
    return 0;
}
LVP_APP sample_app = {
    .app_name = "sample app",
    .AppInit = SampleAppInit,
    .AppEventResponse = SampleAppEventResponse,
    .AppTaskLoop = SampleAppTaskLoop,
    .AppSuspend = SampleSuspend,
    .suspend_priv = "SampleSuspend",
    .AppResume = SampleResume,
    .resume_priv = "SampleSuspend",
};

LVP_REGISTER_APP(sample_app);
