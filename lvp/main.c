/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * main.c:
 *
 */

#include <autoconf.h>
#include <stdio.h>

#include <lvp_attr.h>
#include <lvp_system_init.h>
#include "lvp_mode.h"
#include "app_core/lvp_app_core.h"

DRAM0_STAGE2_SRAM_ATTR int main(int argc, char **argv)
{
    LvpSystemInit();

#if defined(CONFIG_LVP_INIT_WORKMODE_TWS)
    LvpInitMode(LVP_MODE_TWS);
#elif defined(CONFIG_LVP_INIT_WORKMODE_FEED)
    LvpInitMode(LVP_MODE_FEED);
#elif defined(CONFIG_LVP_INIT_WORKMODE_RECORD)
    LvpInitMode(LVP_MODE_RECORD);
#elif defined(CONFIG_LVP_INIT_WORKMODE_DENOISE)
    LvpInitMode(LVP_MODE_DENOISE);
#elif defined(CONFIG_LVP_INIT_WORKMODE_NN_DENOISE)
    LvpInitMode(LVP_MODE_NN_DENOISE);
#elif defined(CONFIG_LVP_INIT_WORKMODE_SED)
    LvpInitMode(LVP_MODE_SED);
#elif defined(CONFIG_LVP_INIT_WORKMODE_FFT_RECOVER)
    LvpInitMode(LVP_MODE_FFT_RECOVER);
#else
    LvpInitMode(LVP_MODE_IDLE);
#endif

    LvpInitializeAppEvent();
    while(LvpModeTick()) {
#ifdef CONFIG_MCU_ENABLE_STACK_MONITORING
        LvpStackSecurityMonitoring();
#endif
        LvpAppEventTick();

        continue;
    }
    LvpSystemDone();
}
