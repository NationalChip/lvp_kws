#ifndef __GX_AUDIO_IN_H__
#define __GX_AUDIO_IN_H__

#include "autoconf.h"

#if (defined CONFIG_ARCH_LEO) || (defined CONFIG_ARCH_LEO_MINI)
#include "gx_audio_in/gx_audio_in_v1.h"

#elif (defined CONFIG_ARCH_GRUS)
#include "gx_audio_in/gx_audio_in_v2.h"
#endif

#endif
