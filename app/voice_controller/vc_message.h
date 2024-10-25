/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * vc_message.h
 *
 */


#ifndef __VC_MESSAGE_H__
#define __VC_MESSAGE_H__

int VCMessageInit(void);
void VCNewMessageNotify(unsigned short kws);
int VCMessageSessionPoll(void);

#endif /*__VC_MESSAGE_H__  */
