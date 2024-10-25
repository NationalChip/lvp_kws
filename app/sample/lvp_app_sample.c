/* Voice Signal Preprocess
* Copyright (C) 2001-2020 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* lvp_app_sample.c
*
*/

#include <lvp_app.h>
#include <lvp_buffer.h>

#ifdef CONFIG_LVP_FACTORY_MIC_TEST
#include "lvp_mic_test.h"
#endif

#define LOG_TAG "[SAMPLE_APP]"

//=================================================================================================

static int SampleAppSuspend(void *priv)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
    printf("%s\n", (unsigned char *)priv);
    return 0;
}

static int SampleAppResume(void *priv)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
    printf("%s\n", (unsigned char *)priv);
    return 0;
}

static int SampleAppInit(void)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);

    return 0;
}

// App Event Process
static int SampleAppEventResponse(APP_EVENT *app_event)
{
#ifdef CONFIG_LVP_FACTORY_MIC_TEST
    // 只是个使用mic测试接口的参考例子，没有考虑会休眠的问题和测试与工作模式切换的问题，需要在实际方案里考虑
    // 使用方式分两步：
    // 1 获取到contex，然后调用LvpMicTestProcess，参数为获取到的contex
    // 2 测试一定的contex后，停止测试，调用LvpMicTestGetResult获取测试结果，结果以字符串显示
    static int count = 0;
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(app_event->ctx_index, &context, &ctx_size);
    if(count < 100)
    {
        LvpMicTestProcess(context);
        count++;
    }

    if(count == 100) // 自己定义个测试的时间
    {
        char *str = LvpMicTestGetResult();
        printf("%s\n", str);
        count = 1000;
    }
#else
    if (app_event->event_id < 100)
        return 0;

    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(app_event->ctx_index, &context, &ctx_size);

    printf(LOG_TAG"event_id %d\n", app_event->event_id);
    printf(LOG_TAG"kws %d\n", context->kws);
    printf(LOG_TAG"vad %d\n", context->fft_vad);
    printf(LOG_TAG"G-vad %d\n", context->G_vad);
#endif
    return 0;
}

// APP Main Loop
static int SampleAppTaskLoop(void)
{
    return 0;
}


LVP_APP sample_app = {
    .app_name = "sample app",
    .AppInit = SampleAppInit,
    .AppEventResponse = SampleAppEventResponse,
    .AppTaskLoop = SampleAppTaskLoop,
    .AppSuspend = SampleAppSuspend,
    .suspend_priv = "SampleAppSuspend",
    .AppResume = SampleAppResume,
    .resume_priv = "SampleAppResume",
};

LVP_REGISTER_APP(sample_app);

