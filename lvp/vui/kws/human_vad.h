/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * kws_strategy.h:
 *
 */

#ifndef __HUMAN_VAD_H__
#define __HUMAN_VAD_H__
#include <lvp_context.h>


void HumanVadDectectInit(void);
int HumanVadDectectDone(void);
int HumanVadDectectGetStatus(void);
int HumanVadDectectRun(LVP_CONTEXT *context, float *data);

#endif
