#ifndef __GX_I2S_H__
#define __GX_I2S_H__

#include "autoconf.h"

#if (defined CONFIG_ARCH_GRUS)
#include "gx_i2s/gx_i2s_v2.h"

#elif (defined CONFIG_ARCH_AQUILA_AUDIO)
#include "gx_i2s/gx_i2s_v3.h"
#endif

#endif
