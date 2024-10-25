/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mode.h: lvp mode
 *
 */

#ifndef __LVP_MODE_H__
#define __LVP_MODE_H__

typedef enum {
    LVP_MODE_IDLE,
    LVP_MODE_TWS,
    LVP_MODE_FACTORY,
    LVP_MODE_RECORD,
    LVP_MODE_FEED,
    LVP_MODE_DENOISE,
    LVP_MODE_NN_DENOISE,
    LVP_MODE_FFT_RECOVER,
    LVP_MODE_SED,
    LVP_MODE_INIT_FLAG = 0xffff  // 模式初始化之前的模式状态
} LVP_MODE_TYPE;
typedef int  (*LVP_MODE_INIT)(LVP_MODE_TYPE prev_mode);
typedef void (*LVP_MODE_DONE)(LVP_MODE_TYPE next_mode);
typedef void (*LVP_MODE_TICK)(void);
typedef int (*LVP_BUFFER_INIT)(void);
typedef struct {
    LVP_MODE_TYPE   type;
    LVP_MODE_INIT   init;
    LVP_MODE_DONE   done;
    LVP_MODE_TICK   tick;
    LVP_BUFFER_INIT buffer_init;
} LVP_MODE_INFO;

LVP_MODE_TYPE LvpInitMode(LVP_MODE_TYPE mode);
LVP_MODE_TYPE LvpSwitchMode(LVP_MODE_TYPE mode);
LVP_MODE_TYPE LvpGetCurrentMode(void);

int LvpModeTick(void);
void LvpExit(void);

#endif /* __LVP_MODE_H__ */
