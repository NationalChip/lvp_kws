/* Voice Signal Preprocess
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app_sample.c
 *
 */

#include <lvp_app.h>
#include <lvp_buffer.h>

#include <lvp_attr.h>

#define LOG_TAG "[SAMPLE_APP]"

//=================================================================================================

static int DLdemoAppSuspend(void *priv)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
    printf("%s\n", (unsigned char *)priv);
    return 0;
}

static int DLdemoAppResume(void *priv)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
    printf("%s\n", (unsigned char *)priv);
    return 0;
}


extern int LD_EXTERNAL_FLASH_IMAGE_SIZE;


#if 1


SECTION_EXTERNAL_FLASH_TEXT static int DL_test(unsigned int flag)
{
    printf("ttttt\n");
    int tmp = 233;
    tmp += flag + 3;
    printf("tmp %d\n", tmp);

    extern  int _start_stack_section_;
    extern  int _end_stack_section_;
    printf("_start_stack_section_ 0x%x\n", _start_stack_section_);
    printf("_end_stack_section_ 0x%x\n", _end_stack_section_);

    extern unsigned int _dl_pageframe_start_data_;
    extern unsigned int _dl_pageframe_end_data_;
    printf("_dl_pageframe_start_data_ 0x%x\n", _dl_pageframe_start_data_);
    printf("_dl_pageframe_end_data_ 0x%x\n", _dl_pageframe_end_data_);


    return flag;
}
#endif

#if 1
SECTION_EXTERNAL_FLASH_RODATA const static unsigned char DL_buffer[] ALIGNED_ATTR(16)= { \
        255,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31, \
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,\
        64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,\
        96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127, \
        128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159, \
        160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191, \
        192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223, \
        224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255, \
        233

};
#endif


static int DLdemoAppInit(void)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);

    int ret;

#ifdef CONFIG_DL
    printf("DL test %d\n", ret);

#if 1

    printf("DL_test:0x%x\n", DL_test);
    ret = DL_test(6);
    printf("ret = %d\n", ret);


#endif

#if 1

    for (int i = 0; i < sizeof(DL_buffer); i++) {
        if (i % 16 == 0)
            printf("\n");
        printf("0x%x:", &DL_buffer[i]);
        printf("%d ", DL_buffer[i]);
    }
            printf("\n");
            printf("\n");

#endif

#if 1

    printf("DL_test:0x%x\n", DL_test);
    ret = DL_test(6);
    printf("ret = %d\n", ret);


#endif

#endif


    return 0;
}

// App Event Process
static int DLdemoAppEventResponse(APP_EVENT *app_event)
{

    return 0;
}

// APP Main Loop
static int DLdemoAppTaskLoop(void)
{
    return 0;
}


LVP_APP DL_demo_app = {
    .app_name = "Dynamic Program Load demo app",
    .AppInit = DLdemoAppInit,
    .AppEventResponse = DLdemoAppEventResponse,
    .AppTaskLoop = DLdemoAppTaskLoop,
    .AppSuspend = DLdemoAppSuspend,
    .suspend_priv = "DLdemoAppSuspend",
    .AppResume = DLdemoAppResume,
    .resume_priv = "DLdemoAppResume",
};

LVP_REGISTER_APP(DL_demo_app);
