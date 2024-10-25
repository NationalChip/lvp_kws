/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * ctc.c: The Process For Kws
 *
 */

#include <autoconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <driver/gx_timer.h>
#include <driver/gx_cache.h>

#include <lvp_system_init.h>
#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_param.h>

#include "decoder.h"
#include "ctc_model.h"

#define LOG_TAG "[LVP_USER_DECODE]"

int LvpDoUserDecoder(LVP_CONTEXT *context)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    gx_dcache_invalid_range((uint32_t *)context->snpu_buffer, context->ctx_header->snpu_buffer_size);
    float *output = (float *)LvpCTCModelGetSnpuOutBuffer(context->snpu_buffer);
    printf("moudel output: %f , %f\n", output[0], output[1]);
    /*
    todo: decode model output
     */
#endif
    return 0;
}
