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
#ifdef CONFIG_LVP_APP_XIP_DEMO_RODATA 
const unsigned char s_test_data_rodata[] XIP_RODATA_ATTR = 
#else
const unsigned char s_test_data_rodata[] = 
#endif
{
    0x00, 0x00, 0x42, 0x01, 0x83, 0x02, 0xc5, 0x03, 0x06, 0x05, 0x48, 0x06,
    0xfa, 0xfa, 0x3b, 0xfc, 0x7d, 0xfd, 0xbe, 0xfe
};

static int _show_test_data()
{
    printf("_show_test_data:    0x%x\n", _show_test_data);

#ifdef CONFIG_LVP_APP_XIP_DEMO_RODATA 
    printf("s_test_data_rodata: 0x%x\n", s_test_data_rodata);
    for (int i = 0; i < 16; i++) {
        printf("0x%x,", s_test_data_rodata[i]);
    }
    printf("\n");
#endif
    printf("\n\n");
}


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
    _show_test_data();

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

