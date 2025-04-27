/* Grus
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * grus_loop.c:
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
#include "denoise_model.h"
#include <lvp_nn_denoise.h>
#include <driver/dsp/csky_math.h>
#include <board_misc_config.h>
#include <driver/gx_uart.h>

#ifdef CONFIG_LVP_WALKIE_TALKIE_MODEL
# ifndef CONFIG_LVP_APP_NN_DENOISE_DEMO
# error "ERROR, LVP_APP_NN_DENOISE_DEMO is not selected for the application"
# endif
static int s_start_rev_cnt = 0;
#endif

#define LOG_TAG "[LVP_RNN_DENOISE]"
static GX_SNPU_CALLBACK s_snpu_callback = NULL;

#ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
static unsigned int s_start_ms;
static unsigned int s_end_ms;
#endif

//=================================================================================================
static int _SnpuCallback(int module_id, GX_SNPU_STATE state, void *priv)
{
    if (s_snpu_callback && priv) {
        s_snpu_callback(module_id, state, priv);
    }
#ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
    s_end_ms = gx_get_time_us();
    printf ("nu:%d\n", s_end_ms - s_start_ms);
#endif

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

static int last_index = CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH - 1;
int LvpNnDenoiseRun(LVP_CONTEXT *context, float *amp)
{
    if (NULL == context) return -1;
    LVP_CONTEXT *pre_context;
    LVP_CONTEXT *cur_context;
    unsigned int size;
    LvpGetContext(context->ctx_index, &pre_context, &size);
    LvpGetContext(context->ctx_index + CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH, &cur_context, &size);

    unsigned short *snpu_input = (unsigned short *)LvpDenoiseModelGetSnpuFeatsBuffer(pre_context->snpu_buffer);
    unsigned short *snpu_output = (unsigned short *)LvpDenoiseModelGetSnpuStatesBuffer(cur_context->snpu_buffer);
    int input_size = LvpDenoiseModelGetSnpuFeatsSize(context->snpu_buffer);
    gx_dcache_invalid_range((uint32_t *)snpu_input, input_size);

    GX_SNPU_TASK snpu_task;
    LvpDenoiseModelInitSnpuTask(&snpu_task);

    if ((context->ctx_index > last_index) && (context->ctx_index != last_index)) {
        LVP_CONTEXT *last_context;
        unsigned int last_ctx_size;
        LvpGetContext(last_index, &last_context, &last_ctx_size);
        memcpy(snpu_input, last_context->snpu_buffer, input_size);
        last_index = context->ctx_index;
    }

    int input_stride = CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH;
    int features_dim = CONFIG_DENOISE_MODEL_FEATURES_DIM_PER_FRAME;
    int last_frames = CONFIG_DENOISE_MODEL_INPUT_WIN_LENGTH - CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH;

    memmove(&snpu_input[0], &snpu_input[input_stride * features_dim], last_frames * features_dim * sizeof(short));
    for (int i = 0; i < input_stride; i++) {
        for(int j = 0; j < features_dim; j++) {
            snpu_input[(last_frames + i) * features_dim + j] = float_32_to_16(amp[i * 257 + j + 1], 5);
        }
    }

    gx_dcache_clean_range((uint32_t *)snpu_input, input_size);
    snpu_task.input  = (void *)MCU_TO_DEV(snpu_input);
    snpu_task.output = (void *)MCU_TO_DEV(snpu_output);

# ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
    s_start_ms = gx_get_time_us();
# endif

    if (context->ctx_index < (PRE_FILL_CONTEXT_NUM - 1)) {
        return -1;
    }

#ifdef CONFIG_LVP_WALKIE_TALKIE_MODEL
    extern int GetWavRecvStatus(void);
    extern int SetWavRecvStatus(int status);
    // 设置第一次送进模型的states
    if (GetWavRecvStatus() == 1 && s_start_rev_cnt < CONFIG_NN_DENOISE_START_SET_INIT_STATE_TIME) {
        s_start_rev_cnt++;
        unsigned short *snpu_states = snpu_input + CONFIG_DENOISE_MODEL_INPUT_WIN_LENGTH * CONFIG_DENOISE_MODEL_FEATURES_DIM_PER_FRAME;
        short *fix_state = (short*)LvpDenoiseModelGetFixStateBuffer();
        if (fix_state == NULL) {
            memset(snpu_states, 0, LvpDenoiseModelGetSnpustatesSize(context->snpu_buffer));
        } else {
            for(int j = 0; j < LvpDenoiseModelGetSnpustatesSize(context->snpu_buffer)/2; j++) {
                snpu_states[j] = fix_state[j];
            }
        }
        gx_dcache_clean_range((uint32_t *)snpu_states, LvpDenoiseModelGetSnpustatesSize(context->snpu_buffer));
    } else {
        s_start_rev_cnt = 0;
        SetWavRecvStatus(0);
    }
#else
    static char first = 1;
    if (first) {  // 将第一次送进模型的states清0
        unsigned short *snpu_states = (unsigned short *)LvpDenoiseModelGetSnpuStatesBuffer(context->snpu_buffer);
        memset(snpu_states, 0, LvpDenoiseModelGetSnpustatesSize(context->snpu_buffer));
        gx_dcache_clean_range((uint32_t *)snpu_states, LvpDenoiseModelGetSnpustatesSize(context->snpu_buffer));
        first = 0;
    }
#endif

    gx_snpu_run_task(&snpu_task, _SnpuCallback, cur_context);

    return 0;
}

#ifndef UART_BOOT_PORT
# define UART_BOOT_PORT 0
#endif

__attribute__ ((unused)) static int _LvpDenoiseLoadSnpuFromUart(const GX_SNPU_TASK *snpu_task, unsigned int cmd_size, unsigned int weight_size)
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

static int _LvpDenoiseLoadSnpuFromFlash(const GX_SNPU_TASK *snpu_task, unsigned int cmd_size, unsigned int weight_size)
{
#ifdef CONFIG_LVP_ENABLE_NN_DENOISE
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
#endif

    return 0;
_return:
    return ret;
}


void LvpLoadDenoiseNpuModle(void) // load npu mode
{
    GX_SNPU_TASK snpu_task;
    int ret;
    unsigned int cmd_size = LvpDenoiseModelGetCmdSize();
    unsigned int weight_size = LvpDenoiseModelGetWeightSize();

#if 0
    snpu_task.cmd     = (void *)(NPU_SRAM_ADDR/4*4);
    snpu_task.weight  = (void *)((unsigned int)snpu_task.cmd + (cmd_size + 3)/4*4);
    snpu_task.ops     = (void *)((unsigned int)snpu_task.weight + (weight_size + 3)/4*4);
    snpu_task.data    = (void *)((unsigned int)snpu_task.ops + (LvpDenoiseModelGetOpsSize() + 3)/4*4);
    snpu_task.tmp_mem = (void *)((unsigned int)snpu_task.data + (LvpDenoiseModelGetDataSize() + 3)/4*4);
#else
    snpu_task.ops = (void *)(NPU_SRAM_ADDR/4*4);
    snpu_task.data = (void *)((unsigned int)snpu_task.ops + (LvpDenoiseModelGetOpsSize() + 3)/4*4);
    snpu_task.tmp_mem = (void *)((unsigned int)snpu_task.data + (LvpDenoiseModelGetDataSize() + 3)/4*4);
    snpu_task.cmd     = (void *)((unsigned int)snpu_task.tmp_mem +(LvpDenoiseModelGetTmpSize() +3)/4*4);
    snpu_task.weight  = (void *)((unsigned int)snpu_task.cmd + (cmd_size + 3)/4*4);
#endif

# if defined (CONFIG_BOOT_BY_FLASH)
#  ifdef MCU_ENABLE_XIP
    gx_spinor_flash_resume();
    ret = _LvpDenoiseLoadSnpuFromFlash(&snpu_task, (cmd_size+3) / 4*4, (weight_size+3) / 4*4);
    gx_spinor_flash_suspend();
#  else
    ret = _LvpDenoiseLoadSnpuFromFlash(&snpu_task, (cmd_size+3) / 4*4, (weight_size+3) / 4*4);
#  endif
# elif defined (CONFIG_BOOT_BY_UART)
    ret = _LvpDenoiseLoadSnpuFromUart(&snpu_task, (cmd_size+3) / 4*4, (weight_size+3) / 4*4);
# endif

    if(ret == 0) {
        LvpDenoiseSetSnpuTask(&snpu_task);
    }
}

int LvpNnDenoiseInit(GX_SNPU_CALLBACK callback, GX_WAKEUP_SOURCE start_mode)
{
    if (GX_WAKEUP_SOURCE_COLD == start_mode || GX_WAKEUP_SOURCE_WDT == start_mode) {
        LvpLoadDenoiseNpuModle();
    }
    gx_snpu_init();
    s_snpu_callback = callback;
    return 0;
}

int LvpNnDenoiseDone(void)
{
    gx_snpu_exit();
    s_snpu_callback = NULL;
    return 0;
}

//-------------------------------------------------------------------------------------------------
