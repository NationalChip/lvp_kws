/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app.h
 *
 */

#ifndef __LVP_APP_H__
#define __LVP_APP_H__

#include "lvp_app_core.h"

typedef struct {
    const char *app_name;
    int (*AppInit)(void);
    int (*AppEventResponse)(APP_EVENT *plc_event);
    int (*AppTaskLoop)(void);
    int (*AppSuspend)(void *priv);
    void *suspend_priv;
    int (*AppResume)(void *priv);
    void *resume_priv;
}LVP_APP;

#define LVP_REGISTER_APP(app)    \
    LVP_APP *app_core_ops = &app

#endif // __LVP_APP_H__
