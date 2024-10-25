/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_context.h:
 *
 */

#ifndef __LVP_MODE_ACTIVE_H__
#define __LVP_MODE_ACTIVE_H__

#include <autoconf.h>
#include <stdio.h>

#include <driver/gx_audio_in.h>

typedef struct {
    GX_AUDIO_IN_SOURCE     source;
    GX_AUDIO_IN_CHANNEL    channel;
} GX_AUDIO_IN_IO_MAP;

typedef enum {
	MONO_MODE,
	STEREO_MODE
} TRACK_MODE;

//=================================================================================================

int LvpActiveInit(void);
int LvpActiveTick(void);

//=================================================================================================
#endif /* __LVP_MODE_ACTIVE_H__ */
