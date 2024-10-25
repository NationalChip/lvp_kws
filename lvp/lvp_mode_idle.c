/* Voice Signal Preprocess
 * Copyright (C) 2001-2019 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 * lvp_mode_idle.c: LVP IDLE mode
 *
 */

#include <autoconf.h>
#include <stdio.h>
#include <types.h>

#include <board_config.h>

#include "lvp_mode.h"

#define LOG_TAG "[IDLE]"

//=================================================================================================

static int _IdleModeInit(LVP_MODE_TYPE prev_mode)
{
    printf(LOG_TAG"Init IDLE mode\n");

    return 0;
}

static void _IdleModeDone(LVP_MODE_TYPE next_mode)
{
    printf(LOG_TAG"Exit IDLE mode\n");
}

static void _IdleModeTick(void)
{
    ;
}

static int _IdleModeBufferInit(void)
{
    return 0;
}

//-------------------------------------------------------------------------------------------------

const LVP_MODE_INFO lvp_idle_mode_info = {
    .type = LVP_MODE_IDLE,
    .buffer_init = _IdleModeBufferInit,
    .init = _IdleModeInit,
    .done = _IdleModeDone,
    .tick = _IdleModeTick,
};
