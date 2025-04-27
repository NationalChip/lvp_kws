/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_vpr.c:
 *
 */

#include <autoconf.h>
#include <types.h>
#include <stdio.h>
#include <string.h>
#include <board_config.h>

#include <driver/gx_snpu.h>
#include <driver/gx_cache.h>

#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_attr.h>
#include <driver/gx_flash.h>
#include <driver/gx_delay.h>
#include <driver/dsp/csky_math.h>
#include <driver/dsp/csky_const_structs.h>

#ifdef CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION
#include <voice_print_model.h>
#endif

struct vp_in_out {
    short Feats[1][1][40];
    short State_c0[1][64];
    short State_h0[1][64];
    short State_c1[1][64];
    short State_h1[1][64];
    short State_c2[1][64];
    short State_h2[1][64];
    short SpEnc_states[3][2][1][64];
    float Model_rnn_out[1][64];
} ALIGNED_ATTR(16);

static struct vp_in_out s_vp_in_out;

static float s_vp_out[64] = {0};

int LvpVpRun(LVP_CONTEXT *context)
{
    printf("LvpVpRun\n");
#ifdef CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION
    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    unsigned int logfbank_num = ctx_header->logfbank_frame_num_per_channel;
    unsigned int logfbank_dim_per_frame = ctx_header->logfbank_dim_per_frame;

    struct vp_in_out *s_snpu_buffer = (struct vp_in_out *)&s_vp_in_out;
    memset ((void *)s_snpu_buffer, 0, sizeof(struct vp_in_out));

    for (int i = (context->ctx_index % logfbank_num) + 3; (i % logfbank_num) != (context->ctx_index % logfbank_num) - 1; i += 2) {
        short *feats  = LvpGetLogfbankBuffer(context, (i % logfbank_num));
        memcpy (s_snpu_buffer->Feats, feats, sizeof(short) * logfbank_dim_per_frame);
        gx_dcache_clean_range((uint32_t *)s_snpu_buffer, sizeof(struct vp_in_out));

        GX_SNPU_TASK snpu_task;
        LvpVPInitSnpuTask(&snpu_task);
        snpu_task.input = (void *)MCU_TO_DEV(s_snpu_buffer->Feats);
        snpu_task.output = (void *)MCU_TO_DEV(s_snpu_buffer->SpEnc_states);

        gx_snpu_run_task_sync(&snpu_task);
        gx_dcache_invalid_range((uint32_t *)s_snpu_buffer, sizeof(struct vp_in_out));

        memcpy((void *)s_snpu_buffer->SpEnc_states, (void *)s_snpu_buffer->State_c0, LvpVPModelGetSnpuStateDim() * sizeof(short));
    }
#endif

    return 0;
}

int LvpVpDecoder(void)
{

    float ret = 0;
    csky_dot_prod_f32(s_vp_out,(float *)&(s_vp_in_out.Model_rnn_out), 64, &ret);
    printf("ret = %f\n", ret);

#if 0
    for (int i = 0; i < 64; i++) {
        if (i % 16 == 0)
            printf("\n");

        printf("%f,", s_vp_in_out.Model_rnn_out[0][i]);
    }
            printf("\n");
#endif

    return 0;
}

int LvpVpLearn(void)
{
    static int learn_counter = 10;

    if (learn_counter == 0) {
        return 1;
    }

    float ret = 0;
    csky_dot_prod_f32(s_vp_out,(float *)&(s_vp_in_out.Model_rnn_out), 64, &ret);
    printf("ret = %f\n", ret);


    for (int i = 0; i < 64; i++) {
        if (i == 0)
            s_vp_out[i] += s_vp_in_out.Model_rnn_out[0][i];
        else
            s_vp_out[i] = s_vp_out[i] * 0.8f + s_vp_in_out.Model_rnn_out[0][i] * 0.2f;
    }


    learn_counter --;

    printf("learn counter %d\n", learn_counter);


    if (learn_counter > 0) {
        return 0;
    } else {
        return 1;
    }

}

static int _LvpVpLoadSnpuFromFlash(void)
{

#ifdef CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION
# ifdef CONFIG_NPU_RUN_IN_FLASH

# else
    unsigned int bus   = CONFIG_SF_DEFAULT_BUS;
    unsigned int cs    = CONFIG_SF_DEFAULT_CS;
    unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
    unsigned int mode  = CONFIG_SF_DEFAULT_MODE;
    GX_FLASH_DEV *dev = gx_spi_flash_probe(bus, cs, speed, mode);
    if (!dev) {
        printf(LOG_TAG"Init Flash Failed\n");
        return -1;
    }

    GX_SNPU_TASK snpu_task;
    snpu_task.cmd     = (void *)(NPU_SRAM_ADDR/4*4);
    snpu_task.weight  = (void *)((unsigned int)snpu_task.cmd + (LvpVPModelGetCmdSize() + 3)/4*4);
    snpu_task.ops     = (void *)((unsigned int)snpu_task.weight + (LvpVPModelGetWeightSize() + 3)/4*4);
    snpu_task.data    = (void *)((unsigned int)snpu_task.ops + (LvpVPModelGetOpsSize() + 3)/4*4);
    snpu_task.tmp_mem = (void *)((unsigned int)snpu_task.ops + (LvpVPModelGetDataSize() + 3)/4*4);
    unsigned int start_ms = gx_get_time_ms();

    extern int LD_VP_NPU_IMAGE_OFFSET;
    unsigned int load_addr = (unsigned int)&LD_VP_NPU_IMAGE_OFFSET;

    int ret = gx_spi_flash_readdata(dev, load_addr, snpu_task.cmd, (LvpVPModelGetCmdSize()+3)/4*4);
    ret |= gx_spi_flash_readdata(dev, load_addr + (LvpVPModelGetCmdSize()+3)/4*4, snpu_task.weight, (LvpVPModelGetWeightSize()+3)/4*4);
    unsigned int end_ms = gx_get_time_ms();
    printf (LOG_TAG"VP Use:%d ms\n", end_ms - start_ms);
    if (0 != ret) {
        printf(LOG_TAG"Read Flash Failed\n");
        return -1;
    }

    LvpVPSetSnpuTask(&snpu_task);
# endif
#endif
    return 0;
}

void LvpLoadVpNpuModle(void) // load npu mode
{
#if (!defined CONFIG_MCU_DEFAULT_TEXT_IN_FLASH) && (!defined CONFIG_NPU_RUN_IN_FLASH)
    gx_spinor_flash_resume();
#endif

    _LvpVpLoadSnpuFromFlash();

#if (!defined CONFIG_MCU_DEFAULT_TEXT_IN_FLASH) && (!defined CONFIG_NPU_RUN_IN_FLASH)
    gx_spinor_flash_suspend();
#endif
}


