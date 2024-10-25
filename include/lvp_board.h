/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_board.h:
 *
 */

#ifndef __LVP_BOARD_H__
#define __LVP_BOARD_H__

#include <lvp_param.h>
#include <lvp_context.h>

//=================================================================================================

LVP_AUDIO_IN_PARAM_CTRL *AudioInBoardGetParamCtrl(void);
void AudioInBoardInit(void);
void BoardInit(void);

void BoardSetUserPinMux(void);
void BoardSetPowerSavePinMux(void);

//=================================================================================================
#endif /* __LVP_BOARD_H__ */
