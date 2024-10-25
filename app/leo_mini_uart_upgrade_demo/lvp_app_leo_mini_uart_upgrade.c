/* Voice Signal Preprocess
* Copyright (C) 2001-2022 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* lvp_app_leo_mini_uart_upgrade.c
*
*/

#include <lvp_app.h>
#include <lvp_buffer.h>
#include <uart_sendboot.h>
#include "mcu_nor_test.h"
#include <driver/gx_gpio.h>
#include <lvp_audio_in.h>
#include <string.h>

#define LOG_TAG "[LEO_MINI_UART_UPGRADE_APP]"


#define PKG_SZIE 4096   // 每次烧录的大小，必须是4096或者4096的整倍数
#define NUM (mcu_nor_test_bin_len / PKG_SZIE)
#define pkg_num ((mcu_nor_test_bin_len % PKG_SZIE) ? NUM + 1 : NUM)
unsigned char upgrade_buff[PKG_SZIE] = {0};
//=================================================================================================

static int LeoMiniUartUpgradeAppSuspend(void *priv){    return 0;}

static int LeoMiniUartUpgradeAppResume(void *priv){    return 0;}

static int LeoMiniUartUpgradeAppInit(void)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
    gx_gpio_set_direction(7, GX_GPIO_DIRECTION_OUTPUT); // 复位脚 拉低复位
    gx_gpio_set_level(7, GX_GPIO_LEVEL_HIGH);
    return 0;
}

// App Event Process
static int LeoMiniUartUpgradeAppEventResponse(APP_EVENT *app_event)
{

    if (app_event->event_id < 100)
        return 0;

    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(app_event->ctx_index, &context, &ctx_size);
    LvpAudioInSuspend();
    if(app_event->event_id == 100){
        DownloadInit(PKG_SZIE);
        for(int i = 0; i < pkg_num; i++)
        {
            memcpy(upgrade_buff, mcu_nor_test_bin + i * PKG_SZIE, PKG_SZIE);
            DownloadFirmware(i * PKG_SZIE, upgrade_buff);
        }
        DownloadReboot();
    }
    LvpAudioInResume();
    return 0;
}

// APP Main Loop
static int LeoMiniUartUpgradeAppTaskLoop(void)
{
    return 0;
}


LVP_APP leo_mini_uart_upgrade_app = {
    .app_name = "Leo mini uart upgrade app",
    .AppInit = LeoMiniUartUpgradeAppInit,
    .AppEventResponse = LeoMiniUartUpgradeAppEventResponse,
    .AppTaskLoop = LeoMiniUartUpgradeAppTaskLoop,
    .AppSuspend = LeoMiniUartUpgradeAppSuspend,
    .suspend_priv = "Leo_mini_uart_upgradeAppSuspend",
    .AppResume = LeoMiniUartUpgradeAppResume,
    .resume_priv = "LeoMiniUartUpgradeAppResume",
};

LVP_REGISTER_APP(leo_mini_uart_upgrade_app);

