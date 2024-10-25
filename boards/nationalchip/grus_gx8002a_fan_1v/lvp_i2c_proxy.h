/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_i2c_proxy.h
 *
 */

#ifndef __LVP_I2C_PROXY_H__
#define __LVP_I2C_PROXY_H__

typedef void (*EVENT_CONFIRM_CB)(void);

int lvp_i2c_proxy_init(void);
int lvp_i2c_proxy_write_voice_event(unsigned char event, EVENT_CONFIRM_CB confirm_cb);

#endif
