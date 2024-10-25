#ifndef __GX_AUDIO_OUT_H__
#define __GX_AUDIO_OUT_H__

#include "autoconf.h"

#if (defined CONFIG_ARCH_LEO) || (defined CONFIG_ARCH_LEO_MINI)
#include "gx_audio_out/gx_audio_out_v1.h"

#elif (defined CONFIG_ARCH_GRUS)
#include "gx_audio_out/gx_audio_out_v2.h"
#endif

#endif /* __GX_AUDIO_OUT_H__ */

