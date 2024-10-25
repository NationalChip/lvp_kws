/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_i2c_msg.h:
 *
 */


#ifndef __LVP_I2C_MSG_H__
#define __LVP_I2C_MSG_H__

typedef void (*I2C_EVENT_CONFIRM_CB)(void);

int LvpI2CMsgInit(void);
int LvpI2CMsgWriteVoiceEvent(unsigned char event, I2C_EVENT_CONFIRM_CB confirm_cb);

#endif /* __LVP_I2C_MSG_H__ */
