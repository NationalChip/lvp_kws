#ifndef __GX_CLOCK_H__
#define __GX_CLOCK_H__

#include "autoconf.h"

#if (defined CONFIG_ARCH_LEO) || (defined CONFIG_ARCH_LEO_MINI)
#include "gx_clock/gx_clock_v1.h"

#elif (defined CONFIG_ARCH_GRUS)
#include "gx_clock/gx_clock_v2.h"
#endif

#endif
