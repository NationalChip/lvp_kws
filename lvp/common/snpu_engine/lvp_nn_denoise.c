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
#include <denoise_model.h>
#include <lvp_nn_denoise.h>
#include <driver/dsp/csky_math.h>
#include <board_misc_config.h>

#define LOG_TAG "[LVP_DENOISE]"
#define MIN_INPUT_NUM 0x400
static GX_SNPU_CALLBACK s_snpu_callback = NULL;
const float pad_number = 0.03162277660168379;
static short s_pad_number_16b = 0;
static short s_vec_1 = 0;

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
    unsigned int u32_in_data = *(unsigned int *)&in_data;

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

static int last_index = 0;
int LvpNnDenoiseRun(LVP_CONTEXT *context, float *amp)
{
    if (NULL == context) return -1;
    unsigned short *snpu_input = (unsigned short *)LvpDenoiseModelGetSnpuFeatsBuffer(context->snpu_buffer);
    unsigned short *snpu_output = (unsigned short *)LvpDenoiseModelGetSnpuOutBuffer(context->snpu_buffer);
     int input_size = LvpDenoiseModelGetSnpuFeatsSize(context->snpu_buffer);
    gx_dcache_invalid_range((uint32_t *)snpu_input, input_size);

    GX_SNPU_TASK snpu_task;
    LvpDenoiseModelInitSnpuTask(&snpu_task);

    if (context->ctx_index > 0 && context->ctx_index != last_index) {
        LVP_CONTEXT *pre_context;
        unsigned int pre_ctx_size;
        LvpGetContext(last_index, &pre_context, &pre_ctx_size);
        memcpy(context->snpu_buffer, pre_context->snpu_buffer, input_size);
        last_index = context->ctx_index;
    }

    int input_stride = CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH;
    int features_dim = CONFIG_DENOISE_MODEL_FEATURES_DIM_PER_FRAME;
    int last_frames = CONFIG_DENOISE_MODEL_INPUT_WIN_LENGTH - CONFIG_DENOISE_MODEL_INPUT_STRIDE_LENGTH;
    int padding = CONFIG_DENOISE_MODEL_FEATURES_PADDING_NUM;
#if (defined(CONFIG_DENOISE_MODEL_FEATURES_PADDING_NUM_LEFT) && (CONFIG_DENOISE_MODEL_FEATURES_PADDING_NUM_LEFT != CONFIG_DENOISE_MODEL_FEATURES_PADDING_NUM_RIGHT))
        int l_padding = CONFIG_DENOISE_MODEL_FEATURES_PADDING_NUM_LEFT;
        int r_padding = features_dim - CONFIG_DENOISE_MODEL_FEATURES_PADDING_NUM_RIGHT;
#else
        int l_padding = padding;
        int r_padding = features_dim - padding;
#endif

    memcpy(&snpu_input[0], &snpu_input[input_stride * features_dim], last_frames * features_dim * sizeof(short));
    for (int j = 0; j < input_stride; j++) {
        for(int i = 0; i < l_padding; i++)
            snpu_input[(last_frames + j) * features_dim + i] = s_pad_number_16b;
        for(int i = r_padding; i < features_dim; i++)
            snpu_input[(last_frames + j) * features_dim + i] = s_pad_number_16b;
        for (int i = l_padding; i < r_padding; i++) {
            snpu_input[(last_frames + j) * features_dim + i] = float_32_to_16(amp[257 * j + (i - l_padding)], 5);
            if (snpu_input[(last_frames + j) * features_dim + i] < MIN_INPUT_NUM)
                snpu_input[(last_frames + j) * features_dim + i] = MIN_INPUT_NUM;
        }
    }
#if (defined(CONFIG_DENOISE_MODE_EXTRA_INPUT_FRAME))
    int last_frame = CONFIG_DENOISE_MODEL_INPUT_WIN_LENGTH;
    int offset = 0;
    for(int i = 0; i < CONFIG_DENOISE_MODE_EXTRA_INPUT_FRAME; i++) {
        offset = (last_frame + i) * features_dim;
        for(int j = 0; j < features_dim; j++)
            snpu_input[offset + j] = s_vec_1;
    }
#endif
    gx_dcache_clean_range((uint32_t *)snpu_input, input_size);
    snpu_task.input  = (void *)MCU_TO_DEV(snpu_input);
    snpu_task.output = (void *)MCU_TO_DEV(snpu_output);

# ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
    s_start_ms = gx_get_time_us();
# endif

    if (context->ctx_index <= (PRE_FILL_CONTEXT_NUM)) {
        return -1;
    }

    gx_snpu_run_task(&snpu_task, _SnpuCallback, context);

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


# if 0
      unsigned char *cmd = snpu_task->cmd;
      unsigned int cmd_total = 0;
      for (int i = 0; i < cmd_size; i++) {
         cmd_total += cmd[i];
      }
      printf("cmd :%x, cmd_total:%x\n", cmd, cmd_total);
      printf("cmd head:\n");
      for (int i = 0; i < 32; i++) {
            printf("%x ", cmd[i]);
      }
      printf("\n");
      printf("cmd tail:\n");
      for (int i = 0; i < 32; i++) {
            printf("%x ", cmd[cmd_size - 1 - i]);
      }
      printf("\n");

      unsigned char *weight = snpu_task->weight;
      unsigned int weight_total = 0;
      for (int i = 0; i < weight_size; i++) {
         weight_total += weight[i];
      }
      printf("\nweight :%x, weight_total:%x\n", weight, weight_total);
      printf("weight head:\n");
      for (int i = 0; i < 70; i++) {
            printf("%x ", weight[i]);
            if ((i+1)%32 == 0) printf("\n");
      }
      printf("\n");
      printf("weight tail:\n");
      for (int i = 0; i < 70; i++) {
            printf("%x ", weight[weight_size - 1 - i]);
            if ((i+1)%32 == 0) printf("\n");
      }
      printf("\n");

      unsigned char *data = snpu_task->data;
      printf("data :%x\n", data);

# endif

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
    s_pad_number_16b = float_32_to_16(pad_number, 5);
    s_vec_1 = float_32_to_16(1.f, 5);

    return 0;
}

int LvpNnDenoiseDone(void)
{
    gx_snpu_exit();
    s_snpu_callback = NULL;
    return 0;
}

//-------------------------------------------------------------------------------------------------
