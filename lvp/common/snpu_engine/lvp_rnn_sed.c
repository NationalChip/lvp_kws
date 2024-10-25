/* Grus
 * Copyright (C) 2001-2023 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_rnn_sed.c:(Sound Event Detection)
 *
 */
#include <autoconf.h>
#include <types.h>
#include <string.h>
#include <board_config.h>

#include <driver/gx_audio_in.h>
#include <driver/gx_snpu.h>
#include <driver/gx_flash.h>
#include <driver/gx_timer.h>
#include <driver/gx_cache.h>
#include <driver/gx_pmu_ctrl.h>

#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_attr.h>
#include <lvp_rnn_sed.h>
#include <driver/dsp/csky_math.h>
#include <board_misc_config.h>
#include <driver/gx_uart.h>

#define LOG_TAG "[LVP_RNN_SED]"
static GX_SNPU_CALLBACK s_snpu_callback = NULL;

#ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
static unsigned long long s_start_ms;
static unsigned long long s_end_ms;
#endif

//=================================================================================================
static int _SnpuCallback(int module_id, GX_SNPU_STATE state, void *priv)
{
#ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
    s_end_ms = gx_get_time_us();
//    printf ("nu:%d\n", s_end_ms - s_start_ms);
#endif
    if (s_snpu_callback && priv) {
        s_snpu_callback(module_id, state, priv);
    }


    return 0;
}

static unsigned short float_32_to_16(float in_data, int exponent_width)
{
    int e_bit_width = exponent_width; //16bits中有e_bit_width指数位
    int d_bit_width = 15 - e_bit_width; //16bits中有d_bit_width小数位
    unsigned int s, e, d; // 符号位， 指数位， 小数位
    int e_out, d_out;
    unsigned short out_data;
    unsigned char round_bit;
    unsigned int u32_in_data;
    memcpy(&u32_in_data, &in_data, sizeof(unsigned int));


    s = (u32_in_data & 0x80000000) >> 31;
    e = (u32_in_data & 0x7f800000) >> 23;
    d = (u32_in_data & 0x007fffff);
    e_out = e - (1 << 7) + (1 << (e_bit_width - 1));
    d_out = (d >> (23 - d_bit_width));
    round_bit = ((d >> (23 - d_bit_width - 1)) & 1);
    if (e_out < 0) // overflow
        out_data = (s << 15) | 0;
    else if (e_out >= (1 << e_bit_width) - 1) // overflow
        out_data = (s << 15) | 0x7fff;
    else
        out_data = (s << 15) | (((e_out << d_bit_width) | d_out) + round_bit);

    return out_data;
}

static int last_index = CONFIG_SED_MODEL_INPUT_STRIDE_LENGTH - 1;

int LvpRnnSedRun(LVP_CONTEXT *context, float *amp)
{
# ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
    s_start_ms = gx_get_time_us();
# endif

    if (NULL == context) return -1;
    LVP_CONTEXT *pre_context;
    LVP_CONTEXT *cur_context;
    unsigned int size;
    LvpGetContext(context->ctx_index, &pre_context, &size);
    LvpGetContext(context->ctx_index + CONFIG_SED_MODEL_INPUT_STRIDE_LENGTH, &cur_context, &size);

    unsigned short *snpu_input = (unsigned short *)LvpSedModelGetSnpuFeatsBuffer(pre_context->snpu_buffer);
    unsigned short *snpu_output = (unsigned short *)LvpSedModelGetSnpuStatesBuffer(cur_context->snpu_buffer);
    int input_size = LvpSedModelGetSnpuFeatsSize(context->snpu_buffer);
    gx_dcache_invalid_range((uint32_t *)snpu_input, input_size);

    GX_SNPU_TASK snpu_task;
    LvpSedModelInitSnpuTask(&snpu_task);

    if ((context->ctx_index > last_index) && (context->ctx_index != last_index)) {
        LVP_CONTEXT *last_context;
        unsigned int last_ctx_size;
        LvpGetContext(last_index, &last_context, &last_ctx_size);
        memcpy(snpu_input, last_context->snpu_buffer, input_size);
        last_index = context->ctx_index;
    }

    int input_stride = CONFIG_SED_MODEL_INPUT_STRIDE_LENGTH;
    int features_dim = CONFIG_SED_MODEL_FEATURES_DIM_PER_FRAME;
    int last_frames = CONFIG_SED_MODEL_INPUT_WIN_LENGTH - CONFIG_SED_MODEL_INPUT_STRIDE_LENGTH;

    memmove(&snpu_input[0], &snpu_input[input_stride * features_dim], last_frames * features_dim * sizeof(short));
    for (int i = 0; i < input_stride; i++) {
        for(int j = 0; j < features_dim; j++) { // [context->ctx_index%2]
            snpu_input[(last_frames + i) * features_dim + j] = float_32_to_16(amp[i * 257 + j + 1], 5);
        }
    }
#if 0
    printf("### amp:\n");
    for (int i = 250; i < 256; i++) {
        if ((i)%8 == 0) printf ("\n");
        printf("%f[%x]", amp[i], (unsigned int *)&amp[i]);
    }
    printf("\n");
#endif
    gx_dcache_clean_range((uint32_t *)snpu_input, input_size);
    snpu_task.input  = (void *)MCU_TO_DEV(snpu_input);
    snpu_task.output = (void *)MCU_TO_DEV(snpu_output);

    if (context->ctx_index < (PRE_FILL_CONTEXT_NUM - 1)) {
        return -1;
    }

    static int state_reset_flag = 1;  // 将第一次送进模型的states清0
#ifdef CONFIG_LVP_ENABLE_RNN_SED_STATE_RESET
    if (context->ctx_index % (CONFIG_LVP_ENABLE_RNN_SED_STATE_RESET_INTERVAL * 1000 / PCM_FRAME_LENGTH) == 0) {
        state_reset_flag = 1;
    }
#endif
    if (state_reset_flag) {
        unsigned short *snpu_states = (unsigned short *)LvpSedModelGetSnpuStatesBuffer(context->snpu_buffer);
        memset(snpu_states, 0, LvpSedModelGetSnpustatesSize(context->snpu_buffer));
        gx_dcache_clean_range((uint32_t *)snpu_states, LvpSedModelGetSnpustatesSize(context->snpu_buffer));
        state_reset_flag = 0;
    }

    gx_snpu_run_task(&snpu_task, _SnpuCallback, cur_context);

    return 0;
}

#ifndef UART_BOOT_PORT
# define UART_BOOT_PORT 0
#endif

__attribute__ ((unused)) static int _LvpSedLoadSnpuFromUart(const GX_SNPU_TASK *snpu_task, unsigned int cmd_size, unsigned int weight_size)
{
    int ret = 0;

    printf("cmd size: %d, weight size: %d\n", cmd_size, weight_size);
    printf("cmd addr: 0x%x, weight size: 0x%x\n", snpu_task->cmd, snpu_task->weight);
    gx_uart_write(UART_BOOT_PORT, (const unsigned char *)"snpu ok", 9);
    if (0 != ret) {
        printf(LOG_TAG"UART Load Failed\n");
        return -1;
    }

    return 0;
}

static int _LvpSedLoadSnpuFromFlash(const GX_SNPU_TASK *snpu_task, unsigned int cmd_size, unsigned int weight_size)
{
# ifdef CONFIG_NPU_RUN_IN_FLASH

# else
    // unsigned int start_ms = gx_get_time_ms();
    unsigned int bus   = CONFIG_SF_DEFAULT_BUS;
    unsigned int cs    = CONFIG_SF_DEFAULT_CS;
    unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
    unsigned int mode  = CONFIG_SF_DEFAULT_MODE;
    int ret = 0;

    GX_FLASH_DEV *dev = gx_spi_flash_probe(bus, cs, speed, mode);
    if (!dev) {
        printf(LOG_TAG"Init Flash Failed\n");
        ret = -1;
        goto _return;
    }

    extern int LD_NPU_IMAGE_OFFSET;
#ifdef CONFIG_MULTIBOOT_SECOND_BIN
    unsigned int load_addr = (unsigned int)&LD_NPU_IMAGE_OFFSET + CONFIG_SECOND_BIN_OFFSET;
#else
    unsigned int load_addr = (unsigned int)&LD_NPU_IMAGE_OFFSET;
#endif
    printf ("load_addr: %x\n", load_addr);

    ret = gx_spi_flash_readdata(dev, load_addr, snpu_task->cmd, cmd_size);
    ret |= gx_spi_flash_readdata(dev, load_addr + (cmd_size+3)/4*4, snpu_task->weight, weight_size);
    if (0 != ret) {
        printf(LOG_TAG"Read Flash Failed\n");
        return -1;
    }
#endif

    return 0;
_return:
    return ret;
}


static void _LvpLoadSedNpuModle(void) // load npu mode
{
    GX_SNPU_TASK snpu_task;
    int ret;
    unsigned int cmd_size = LvpSedModelGetCmdSize();
    unsigned int weight_size = LvpSedModelGetWeightSize();

#if 0
    snpu_task.cmd     = (void *)(NPU_SRAM_ADDR/4*4);
    snpu_task.weight  = (void *)((unsigned int)snpu_task.cmd + (cmd_size + 3)/4*4);
    snpu_task.ops     = (void *)((unsigned int)snpu_task.weight + (weight_size + 3)/4*4);
    snpu_task.data    = (void *)((unsigned int)snpu_task.ops + (LvpSedModelGetOpsSize() + 3)/4*4);
    snpu_task.tmp_mem = (void *)((unsigned int)snpu_task.data + (LvpSedModelGetDataSize() + 3)/4*4);
#else
    snpu_task.ops = (void *)(NPU_SRAM_ADDR/4*4);
    snpu_task.data = (void *)((unsigned int)snpu_task.ops + (LvpSedModelGetOpsSize() + 3)/4*4);
    snpu_task.tmp_mem = (void *)((unsigned int)snpu_task.data + (LvpSedModelGetDataSize() + 3)/4*4);
    snpu_task.cmd     = (void *)((unsigned int)snpu_task.tmp_mem +(LvpSedModelGetTmpSize() +3)/4*4);
    snpu_task.weight  = (void *)((unsigned int)snpu_task.cmd + (cmd_size + 3)/4*4);
#endif

# if defined (CONFIG_BOOT_BY_FLASH)
#  ifdef MCU_ENABLE_XIP
    gx_spinor_flash_resume();
    ret = _LvpSedLoadSnpuFromFlash(&snpu_task, (cmd_size+3) / 4*4, (weight_size+3) / 4*4);
    gx_spinor_flash_suspend();
#  else
    ret = _LvpSedLoadSnpuFromFlash(&snpu_task, (cmd_size+3) / 4*4, (weight_size+3) / 4*4);
#  endif
# elif defined (CONFIG_BOOT_BY_UART)
    ret = _LvpSedLoadSnpuFromUart(&snpu_task, (cmd_size+3) / 4*4, (weight_size+3) / 4*4);
# endif

//    printf("\n\nL cmd: 0x%x, 0x%x, 0x%x\n\n", *((unsigned char *)snpu_task.cmd), *((unsigned char *)snpu_task.cmd+1), *((unsigned char *)snpu_task.cmd+2));
//    printf("L cmd: 0x%x, 0x%x, 0x%x\n\n", *((unsigned char *)snpu_task.cmd+2657), *((unsigned char *)snpu_task.cmd+2658), *((unsigned char *)snpu_task.cmd+2659));
//    printf("L weight: 0x%x, 0x%x, 0x%x\n\n", *((unsigned char *)snpu_task.weight), *((unsigned char *)snpu_task.weight+1), *((unsigned char *)snpu_task.weight+2));
//    printf("L weight: 0x%x, 0x%x, 0x%x\n\n", *((unsigned char *)snpu_task.weight+11631), *((unsigned char *)snpu_task.weight+11632), *((unsigned char *)snpu_task.weight+11633));

    if(ret == 0) {
        LvpSedSetSnpuTask(&snpu_task);
    }
}

int LvpRnnSedInit(GX_SNPU_CALLBACK callback, GX_WAKEUP_SOURCE start_mode)
{
    if (GX_WAKEUP_SOURCE_COLD == start_mode || GX_WAKEUP_SOURCE_WDT == start_mode) {
        _LvpLoadSedNpuModle();
    }
    gx_snpu_init();
    s_snpu_callback = callback;
    return 0;
}

int LvpRnnSedDone(void)
{
    gx_snpu_exit();
    s_snpu_callback = NULL;
    return 0;
}

//-------------------------------------------------------------------------------------------------
